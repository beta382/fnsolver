#include "solution_widget.h"
#include <QFormLayout>
#include <QLocale>
#include "precious_resource_ui.h"

SolutionWidget::SolutionWidget(QWidget* parent): QWidget(parent), production_label_(new QLabel(this)),
  revenue_label_(new QLabel(this)), storage_label_(new QLabel(this)), resources_label_(new QLabel(this)) {
  auto* layout = new QFormLayout(this);
  layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  layout->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
  layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

  layout->addRow(tr("Production:"), production_label_);
  layout->addRow(tr("Revenue:"), revenue_label_);
  layout->addRow(tr("Storage:"), storage_label_);
  layout->addRow(tr("Resources:"), new QLabel(this));
  layout->addRow(resources_label_);
  resources_label_->setTextFormat(Qt::RichText);
}

void SolutionWidget::set_layout(const Layout& layout) {
  const auto& yield = layout.get_resource_yield();

  production_label_->setText(QLocale().toString(yield.get_production()));

  revenue_label_->setText(QLocale().toString(yield.get_revenue()));

  storage_label_->setText(QLocale().toString(yield.get_storage()));

  QStringList precious_resource_rows;
  for (std::size_t resource_ix = 0; resource_ix < precious_resource::count; ++resource_ix) {
    const auto quantity = yield.get_precious_resource_quantities().at(resource_ix);
    const auto max_quantity = precious_resource::max_resource_quantity(
      static_cast<precious_resource::Type>(resource_ix));
    precious_resource_rows.append(
      QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
      .arg(precious_resource_display_name(static_cast<precious_resource::Type>(resource_ix)))
      .arg(tr("%1/%2")
           .arg(QLocale().toString(static_cast<double>(quantity) / 100.0))
           .arg(QLocale().toString(static_cast<double>(max_quantity) / 100.0)))
      .arg(tr("%1%").arg(static_cast<double>(quantity) / static_cast<double>(max_quantity) * 100.0))
    );
  }
  resources_label_->setText(
    QString(R"(
<table>
  <thead>
    <tr>
      <th>%1</th>
      <th>%2</th>
      <th>%3</th>
    </tr>
  </thead>
  <tbody>%4</tbody>
</table>
)")
    .arg(tr("Name"))
    .arg(tr("Quantity"))
    .arg(tr("Percent"))
    .arg(precious_resource_rows.join(""))
  );
}
