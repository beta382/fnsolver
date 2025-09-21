#ifndef FNSOLVER_GUI_MIRA_MAP_H
#define FNSOLVER_GUI_MIRA_MAP_H

#include <unordered_map>
#include <QGraphicsView>
#include <fnsolver/layout/layout.h>

#include "image_provider.h"
#include "QGraphicsItemDeleter.h"

class MiraMap : public QGraphicsView {
  Q_OBJECT

public:
  explicit MiraMap(Layout* layout, const ImageProvider& image_provider, QWidget* parent = nullptr);
  void set_layout(Layout* layout);

Q_SIGNALS:
  void site_probe_map_changed();

public Q_SLOTS:
  void redraw();
  void fit_all();
  void zoom_in();
  void zoom_out();

protected:
  void resizeEvent(QResizeEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private:
  using GraphicsItemPtr = std::unique_ptr<QGraphicsItem, QGraphicsItemDeleter>;
  // These values came from trial-and-error to feel best.
  static constexpr auto zoom_factor = 0.25;
  static constexpr auto z_sites = 0;
  static constexpr auto z_combo = -5;
  static constexpr auto z_links = -10;
  static constexpr auto z_map = -100;
  const ImageProvider& image_provider_;
  QGraphicsScene map_scene_;
  Layout* layout_;
  std::unordered_map<FnSite::id_t, GraphicsItemPtr> site_widgets_;
  std::vector<GraphicsItemPtr> link_graphics_;
  std::vector<GraphicsItemPtr> combo_graphics_;

private Q_SLOTS:
  void calculate_site_widgets();
  void calculate_links();
};

#endif //FNSOLVER_GUI_MIRA_MAP_H
