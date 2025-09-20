#include "fn_site_widget.h"

#include <QLocale>
#include <QMenu>
#include <QPainter>
#include <QPaintEvent>
#include <QSvgRenderer>
#include <QActionGroup>

#include "precious_resource_ui.h"
#include "probe_ui.h"

FnSiteWidget::FnSiteWidget(const FnSite* site, QWidget* parent): QWidget(parent), site_(site),
  data_probe_(&Probe::probes.at(Probe::idx_for_shorthand.at("X"))) {
  setFixedSize(kSize, kSize);
  setAttribute(Qt::WA_NoSystemBackground, true);
  update_tooltip_text();

  // Discovered territories.
  if (site->max_territories > 0) {
    QActionGroup* territories_group = new QActionGroup(this);
    for (uint32_t num_territories = 0; num_territories <= site->max_territories; ++num_territories) {
      auto* action = territory_actions_.emplace_back(new QAction(tr("%n territories", "", num_territories), this));
      action->setCheckable(true);
      territories_group->addAction(action);
      action->setChecked(site->territories == num_territories);
      connect(action, &QAction::triggered, this, [this, num_territories] {
        set_num_territories(num_territories);
        Q_EMIT(territories_changed());
      });
      addAction(action);
    }
  }

  // Actions to change probe.
  auto sep = new QAction(this);
  sep->setSeparator(true);
  addAction(sep);
  auto* probes_action_group = new QActionGroup(this);
  for (const auto& probe : Probe::probes) {
    auto* action = probe_actions_.emplace(probe.probe_id, new QAction(this)).first->second;
    action->setCheckable(true);
    probes_action_group->addAction(action);
    action->setChecked(data_probe_->probe_id == probe.probe_id);
    action->setIcon(QIcon(probe_image(&probe)));
    action->setText(probe_display_name(&probe));
    connect(action, &QAction::triggered, [this, &probe]() {
      set_data_probe(&probe);
      Q_EMIT(data_probe_changed(&probe));
    });
    addAction(action);
  }
}

void FnSiteWidget::set_data_probe(const Probe* probe) {
  data_probe_ = probe;
  probe_actions_.at(data_probe_->probe_id)->setChecked(true);
  update();
  update_tooltip_text();
}

void FnSiteWidget::set_num_territories(uint32_t num_territories) {
  if (territory_actions_.empty()) {
    return;
  }
  FnSite::override_territories(site_->site_id, num_territories);
  territory_actions_.at(num_territories)->setChecked(true);
  update_tooltip_text();
}

void FnSiteWidget::paintEvent(QPaintEvent* event) {
  const auto image = probe_image(data_probe_);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  // Probe image.
  painter.drawPixmap(rect(), image);

  // Probe level.
  if (data_probe_ != nullptr && data_probe_->probe_level > 0) {
    QSvgRenderer svg_renderer(
      QString(":/probe_levels/og/%1.svg").arg(data_probe_->probe_level));
    svg_renderer.setAspectRatioMode(Qt::KeepAspectRatio);
    svg_renderer.render(&painter);
  }

  event->accept();
}

void FnSiteWidget::update_tooltip_text() {
  QStringList precious_resource_strings;
  for (std::size_t resource_ix = 0; resource_ix < precious_resource::count; ++resource_ix) {
    const auto quantity = site_->precious_resource_quantities.at(resource_ix);
    if (quantity > 0) {
      precious_resource_strings.append(
        precious_resource_display_name(static_cast<precious_resource::Type>(resource_ix)));
    }
  }
  setToolTip(
    tr(R"(
<strong>FN%1</strong><br>
%2<br>
%3 of %4 territories found<br>
<table>
  <tr>
    <th align="right">Production:</th>
    <td align="left">%5</td>
  </tr>
  <tr>
    <th align="right">Revenue:</th>
    <td align="left">%6</td>
  </tr>
  <tr>
    <th align="right">Combat:</th>
    <td align="left">%7</td>
  </tr>
  <tr>
    <th align="right">Ore:</th>
    <td align="left">%8</td>
  </tr>
</table>
    )")
    .arg(site_->site_id)
    .arg(data_probe() == nullptr
           ? tr("No probe")
           : probe_display_name(data_probe()))
    .arg(site_->territories)
    .arg(site_->max_territories)
    .arg(site_->production_grade())
    .arg(site_->revenue_grade())
    .arg(site_->combat)
    .arg(QLocale().createSeparatedList(precious_resource_strings)));
}
