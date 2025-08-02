#ifndef FNSOLVER_GUI_FN_SITE_WIDGET_H
#define FNSOLVER_GUI_FN_SITE_WIDGET_H

#include <QWidget>

#include "fnsolver/data/fnsite.h"
#include "fnsolver/data/probe.h"

/**
 * Show the data probe assigned to this site.
 */
class FnSiteWidget : public QWidget {
  Q_OBJECT

public:
  static constexpr auto kSize = 64;

  explicit FnSiteWidget(const FnSite* site, QWidget* parent = nullptr);


  [[nodiscard]] const FnSite* site() const { return site_; }

  [[nodiscard]] bool visited() const { return data_probe_->probe_type != Probe::Type::none; }

  [[nodiscard]] const Probe* data_probe() const {
    return data_probe_;
  }

  void set_data_probe(const Probe* dataProbe);

Q_SIGNALS:
  void data_probe_changed(const Probe* dataProbe);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  const FnSite* site_;
  bool tooltip_shown_ = false;
  std::vector<QAction*> set_probes_actions_;
  const Probe* data_probe_;

  void update_tooltip_text();
};


#endif //FNSOLVER_GUI_FN_SITE_WIDGET_H
