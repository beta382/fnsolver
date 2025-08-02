#include "fn_site_widget.h"
#include <QLocale>
#include <QPainter>
#include <QPaintEvent>
#include <QSvgRenderer>
#include "precious_resource_ui.h"
#include "probe_ui.h"

FnSiteWidget::FnSiteWidget(const FnSite* site, QWidget* parent): QWidget(parent), site_(site),
  data_probe_(&Probe::probes.at(Probe::idx_for_shorthand.at("X"))) {
  setFixedSize(kSize, kSize);
  setAttribute(Qt::WA_NoSystemBackground, true);
  update_tooltip_text();

  // Actions to change probe.
  auto sep = new QAction(this);
  sep->setSeparator(true);
  addAction(sep);
  set_probes_actions_.reserve(Probe::probes.size());
  for (const auto& probe : Probe::probes) {
    auto* action = new QAction(this);
    action->setIcon(QIcon(probe_image(&probe)));
    action->setText(probe_display_name(&probe));
    connect(action, &QAction::triggered, [this, &probe]() {
      set_data_probe(&probe);
      Q_EMIT(data_probe_changed(&probe));
    });
    addAction(action);
    set_probes_actions_.push_back(action);
  }
}

void FnSiteWidget::set_data_probe(const Probe* dataProbe) {
  data_probe_ = dataProbe;
  update();
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
      QString(":/probe_levels/%1.svg").arg(data_probe_->probe_level));
    svg_renderer.setAspectRatioMode(Qt::KeepAspectRatio);
    svg_renderer.render(&painter);
  }

  event->accept();
}

void FnSiteWidget::update_tooltip_text() {
  QStringList precious_resource_strings;
  for (auto resource_ix = 0; resource_ix < precious_resource::count; ++resource_ix) {
    const auto quantity = site_->precious_resource_quantities.at(resource_ix);
    if (quantity > 0) {
      precious_resource_strings.append(
        precious_resource_display_name(static_cast<precious_resource::Type>(resource_ix)));
    }
  }
  setToolTip(
    tr(
      // @formatter:off
      "<strong>FN%1</strong><br>"
      "%2<br>"
      "<table>"
      "<tr><th align=\"right\">Production:</th><td align=\"left\">%3</td></tr>"
      "<tr><th align=\"right\">Revenue:</th><td align=\"left\">%4</td></tr>"
      "<tr><th align=\"right\">Combat:</th><td align=\"left\">%5</td></tr>"
      "<tr><th align=\"right\">Ore:</th><td align=\"left\">%6</td></tr>"
      "</table>"
      // @formatter:on
    )
    .arg(site_->site_id)
    .arg(data_probe() == nullptr
           ? tr("No probe")
           : probe_display_name(data_probe()))
    .arg(site_->production_grade())
    .arg(site_->revenue_grade())
    .arg(site_->combat)
    .arg(QLocale().createSeparatedList(precious_resource_strings)));
}
