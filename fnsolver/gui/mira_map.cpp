#include "mira_map.h"
#include <QApplication>
#include <QWheelEvent>
#include <QMenu>
#include <QGraphicsProxyWidget>
#include <QHash>

#include "fnsite_ui.h"
#include "fn_site_widget.h"

MiraMap::MiraMap(Layout* layout, QWidget* parent): QGraphicsView(parent), layout_(layout) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setDragMode(ScrollHandDrag);
  setMouseTracking(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  // Add map tiles.
  const auto tile_size = 256; // 256x256 pixels
  const auto map_size = 16; // 16x16 tile grid
  map_scene_.setSceneRect(0, 0, tile_size * map_size, tile_size * map_size);
  for (auto tile_x = 0; tile_x < map_size; ++tile_x) {
    for (auto tile_y = 0; tile_y < map_size; ++tile_y) {
      QPixmap tile_pixmap;
      // Not all tiles are present, only those with map images.
      if (tile_pixmap.load(
        QString(":/map_tiles/tile_%1_%2.png").arg(tile_x).arg(tile_y))) {
        auto* scenePixmap = map_scene_.addPixmap(tile_pixmap);
        scenePixmap->setOffset(tile_x * tile_size, tile_y * tile_size);
        scenePixmap->setZValue(z_map);
      }
    }
  }
  setScene(&map_scene_);

  // Site widgets.
  for (const auto& site : FnSite::sites) {
    auto siteWidget = new FnSiteWidget(&site);
    connect(siteWidget, &FnSiteWidget::data_probe_changed,
            [this, &site](const Probe* probe) {
              layout_->set_probe(site, *probe);
              calculate_links();
              Q_EMIT(site_probe_map_changed());
            });
    auto& siteButton =
      site_widgets_.emplace_back(map_scene_.addWidget(siteWidget));
    // Site data stores the center point, so we need to half it to get the
    // corners for drawing.
    const auto& site_position = site_positions.at(site.site_id);
    siteButton->setX(site_position.first - (siteWidget->width() / 2.0));
    siteButton->setY(site_position.second - (siteWidget->height() / 2.0));
    siteButton->setZValue(z_sites);
  }
  calculate_site_widgets();
}

void MiraMap::set_layout(Layout* layout) {
  layout_ = layout;
  calculate_site_widgets();
}

void MiraMap::fit_all() {
  fitInView(map_scene_.itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MiraMap::zoom_in() {
  constexpr auto scale_factor = 1 + zoom_factor;
  scale(scale_factor, scale_factor);
}

void MiraMap::zoom_out() {
  constexpr auto scale_factor = 1 - zoom_factor;
  scale(scale_factor, scale_factor);
}

void MiraMap::resizeEvent(QResizeEvent* event) {
  fit_all();
  QGraphicsView::resizeEvent(event);
}

void MiraMap::wheelEvent(QWheelEvent* event) {
  if (qApp->queryKeyboardModifiers() == Qt::ControlModifier) {
    // Ctrl+Wheel to zoom.
    event->accept();
    const auto degrees = event->angleDelta() / 8;
    const auto scaleChange = degrees.y() / 100.0;

    // Scaling is relative to current scale, not absolute.
    const auto newScale = scaleChange + 1.0;
    scale(newScale, newScale);
    return;
  }
  QGraphicsView::wheelEvent(event);
}

void MiraMap::contextMenuEvent(QContextMenuEvent* event) {
  // If the widget is responsible for popping its own menu, it is scaled with
  // the rest of the view which makes it hard to read. Popping it here (in the
  // scene) does not scale the menu.
  const auto item = itemAt(event->pos());
  const auto widget = qgraphicsitem_cast<QGraphicsProxyWidget*>(item);
  if (widget == nullptr) {
    return;
  }

  // Add the widget's actions to this menu.
  QMenu menu;
  for (const auto action : widget->widget()->actions()) {
    menu.addAction(action);
  }
  menu.exec(event->globalPos());

  event->accept();
}

void MiraMap::calculate_site_widgets() {
  for (auto& widget : site_widgets_) {
    auto* fn_site_widget = dynamic_cast<FnSiteWidget*>(
      dynamic_cast<QGraphicsProxyWidget*>(widget.get())->widget());

    const auto site = fn_site_widget->site();
    const auto probe = layout_->get_probe(*site);
    fn_site_widget->set_data_probe(probe);
  }

  calculate_links();
}

struct SiteIdSetHash {
  std::size_t operator()(const std::set<FnSite::id_t>& ids) const noexcept {
    return qHashRangeCommutative(ids.cbegin(), ids.cend());
  }
};

static const auto no_combo_link_color = QColorConstants::Svg::cyan;
static const auto with_combo_link_color = QColorConstants::Svg::deeppink;
static const auto combo_circle_color = QColorConstants::Svg::red;

void MiraMap::calculate_links() {
  QPen pen(Qt::SolidLine);
  pen.setWidth(4);
  link_graphics_.clear();
  combo_graphics_.clear();
  // Need to track which lines have already been drawn as both sides of the link
  // are stored.
  std::unordered_set<std::set<FnSite::id_t>, SiteIdSetHash> drawn_links;
  for (const auto& site : FnSite::sites) {
    const auto site_probe = layout_->get_probe(site);
    const auto& resolved_placement = layout_->get_resolved_placement(site);
    const auto combo_bonus = resolved_placement.get_chain_bonus();
    for (const auto neighbor_idx : site.neighbor_idxs) {
      const auto& neighbor = FnSite::sites.at(neighbor_idx);
      // Don't draw combo lines more than once.
      const std::set link_pair{site.site_id, neighbor.site_id};
      if (drawn_links.contains(link_pair)) {
        // Already drawn line.
        continue;
      }
      const auto neighbor_probe = layout_->get_probe(neighbor);
      if (site_probe->probe_type == Probe::Type::none || neighbor_probe->probe_type == Probe::Type::none) {
        // Do not draw links between sites not visited.
        continue;
      }

      bool combo_link = false;
      if (combo_bonus > 1) {
        // This site participates in a combo, determine if this link is part of
        // it.
        if (site_probe->probe_id == neighbor_probe->probe_id) {
          combo_link = true;
        }
      }
      if (combo_link) {
        pen.setColor(with_combo_link_color);
      }
      else {
        pen.setColor(no_combo_link_color);
      }

      const auto [x1, y1] = site_positions.at(site.site_id);
      const auto [x2, y2] = site_positions.at(neighbor.site_id);
      auto& linkItem = link_graphics_.emplace_back(
        map_scene_.addLine(QLine(x1, y1, x2, y2), pen));
      linkItem->setZValue(z_links);
      drawn_links.insert(link_pair);
    }

    // Draw circles on sites that are part of a combo.
    if (combo_bonus > 1) {
      pen.setColor(combo_circle_color);
      auto [x, y] = site_positions.at(site.site_id);
      const auto w = 96;
      const auto h = w;
      x -= w / 2;
      y -= h / 2;
      auto& comboItem = combo_graphics_.emplace_back(
        map_scene_.addEllipse(QRect(x, y, w, h), pen));
      comboItem->setZValue(z_combo);
    }
  }
}
