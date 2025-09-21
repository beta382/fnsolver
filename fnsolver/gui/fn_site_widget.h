#ifndef FNSOLVER_GUI_FN_SITE_WIDGET_H
#define FNSOLVER_GUI_FN_SITE_WIDGET_H

#include <QWidget>

#include "image_provider.h"
#include "fnsolver/data/fnsite.h"
#include "fnsolver/data/probe.h"

/**
 * Show the data probe assigned to this site.
 */
class FnSiteWidget : public QWidget {
  Q_OBJECT

public:
  static constexpr auto kSize = 64;

  explicit FnSiteWidget(const FnSite* site, const ImageProvider& image_provider, QWidget* parent = nullptr);


  [[nodiscard]] const FnSite* site() const { return site_; }

  [[nodiscard]] bool visited() const { return data_probe_->probe_type != Probe::Type::none; }

  [[nodiscard]] const Probe* data_probe() const {
    return data_probe_;
  }

  void set_data_probe(const Probe* probe);
  void set_num_territories(uint32_t num_territories);

public Q_SLOTS:
  void redraw();

Q_SIGNALS:
  void data_probe_changed(const Probe* dataProbe);
  void territories_changed();

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  const ImageProvider& image_provider_;
  const FnSite* site_;
  bool tooltip_shown_ = false;
  const Probe* data_probe_;
  std::unordered_map<uint32_t, QAction*> probe_actions_;
  std::vector<QAction*> territory_actions_;

  void update_tooltip_text();
};


#endif //FNSOLVER_GUI_FN_SITE_WIDGET_H
