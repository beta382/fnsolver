#include "constraints_widget.h"
#include <QFormLayout>
#include <QGroupBox>
#include "precious_resource_ui.h"

namespace detail {
PreciousResourceConstraintWidget::PreciousResourceConstraintWidget(precious_resource::Type precious_resource,
                                                                   QWidget* parent):
  QWidget(parent), precious_resource_(precious_resource) {
  auto* layout = new QHBoxLayout(this);
  // Constraint type.
  widgets_.constraint_type = new QComboBox(this);
  layout->addWidget(widgets_.constraint_type);
  widgets_.constraint_type->addItem(tr("All"), static_cast<int>(Type::All));
  widgets_.constraint_type->addItem(tr("Any"), static_cast<int>(Type::Any));
  widgets_.constraint_type->addItem(tr("Percent"), static_cast<int>(Type::Percent));
  widgets_.constraint_type->addItem(tr("Min. Avg. Yield"), static_cast<int>(Type::Value));
  connect(widgets_.constraint_type, &QComboBox::currentIndexChanged, this,
          &PreciousResourceConstraintWidget::constraint_type_changed);

  // Value.
  widgets_.constraint_value = new QDoubleSpinBox(this);
  layout->addWidget(widgets_.constraint_value);
  widgets_.constraint_value->setDecimals(2);

  // Maximum value.
  widgets_.constraint_max = new QLabel(
    tr("of %1").arg(precious_resource::max_resource_quantity(precious_resource) / 100.0, 0, 'f', 2));
  layout->addWidget(widgets_.constraint_max);

  update_value_spinbox_range();
}

uint32_t PreciousResourceConstraintWidget::get_value() const {
  if (type_ == Type::Any) {
    return 1;
  }
  return std::ceil(widgets_.constraint_value->value() * 100.0);
}

void PreciousResourceConstraintWidget::set_value(uint32_t value) {
  if (value == 1) {
    set_type(Type::Any);
  }
  else if (value == precious_resource::max_resource_quantity(precious_resource_)) {
    set_type(Type::All);
  }
  else {
    set_type(Type::Value);
    widgets_.constraint_value->setValue(value / 100.0);
  }
}

void PreciousResourceConstraintWidget::set_type(Type type) {
  widgets_.constraint_type->setCurrentIndex(static_cast<int>(type));
  // QComboBox will call constraint_type_changed slot.
}

void PreciousResourceConstraintWidget::update_value_spinbox_range() {
  bool value_visible, max_visible;
  double min, max, value;
  QString suffix;
  if (type_ == Type::Any) {
    value_visible = false;
    max_visible = false;
    min = max = value = 0.01;
  }
  else if (type_ == Type::All) {
    value_visible = false;
    max_visible = false;
    min = max = value = precious_resource::max_resource_quantity(precious_resource_) / 100.0;
  }
  else {
    value_visible = true;
    if (type_ == Type::Percent) {
      max_visible = false;
      min = 0;
      max = 100;
      suffix = QLocale().percent();
    }
    else {
      max_visible = true;
      min = 0;
      max = precious_resource::max_resource_quantity(precious_resource_) / 100.0;
    }
    // Clamp to the new maximum value.
    value = std::min(widgets_.constraint_value->value(), max);
  }
  widgets_.constraint_value->setVisible(value_visible);
  widgets_.constraint_value->setMinimum(min);
  widgets_.constraint_value->setMaximum(max);
  widgets_.constraint_value->setValue(value);
  widgets_.constraint_value->setSuffix(suffix);
  widgets_.constraint_max->setVisible(max_visible);
}

void PreciousResourceConstraintWidget::constraint_type_changed(int index) {
  const auto old_type = type_;
  const auto old_value = widgets_.constraint_value->value();
  type_ = static_cast<Type>(widgets_.constraint_type->itemData(index).toInt());
  update_value_spinbox_range();

  // Convert values to new type's representation.
  if (old_type == Type::All) {
    if (type_ == Type::Percent) {
      widgets_.constraint_value->setValue(100.0);
    }
    else if (type_ == Type::Value) {
      widgets_.constraint_value->setValue(precious_resource::max_resource_quantity(precious_resource_) / 100.0);
    }
  }
  else if (old_type == Type::Any) {
    widgets_.constraint_value->setValue(0.01);
  }
  else if (old_type == Type::Percent && type_ == Type::Value) {
    // Convert percent to value.
    widgets_.constraint_value->setValue(
      (precious_resource::max_resource_quantity(precious_resource_) / 100.0) * (old_value / 100.0)
    );
  }
  else if (old_type == Type::Value && type_ == Type::Percent) {
    // Convert value to percent.
    widgets_.constraint_value->setValue(
      (old_value / (precious_resource::max_resource_quantity(precious_resource_) / 100.0)) * 100.0
    );
  }
}
} // namespace detail

ConstraintsWidget::ConstraintsWidget(const Options* solver_options, QWidget* parent): QWidget(parent) {
  auto* layout = new QVBoxLayout(this);

  // Precious Resources.
  auto* resources = new QGroupBox(tr("Minimum Resources"));
  layout->addWidget(resources);
  auto* resources_layout = new QFormLayout();
  resources->setLayout(resources_layout);
  auto* resources_desc = new QLabel(tr(R"(
Requires that a generated FrontierNav layout yield at least the specified quantity of Precious Resources (on average).

A given FrontierNav site distributes precious resources with per-site rates, and per-site roll counts. You may consult
Xeno Series Wiki (e.g., [Bonjelium](https://www.xenoserieswiki.org/wiki/Bonjelium)) for rate information. Because each
site may have greatly different expected drop counts for a given Precious Resource, it does not make much sense to
constrain by "number of sites that are yielding Precious Resources". As such, the constraints provided here are based on
"average expected drop count".
)"), this);
  resources_desc->setTextFormat(Qt::MarkdownText);
  resources_desc->setWordWrap(true);
  resources_desc->setOpenExternalLinks(true);
  resources_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  resources_layout->addRow(resources_desc);
  for (int resource_ix = 0; resource_ix < precious_resource::count; ++resource_ix) {
    const auto resource = static_cast<precious_resource::Type>(resource_ix);
    const auto current_minimum = solver_options->get_precious_resource_minimums().at(resource_ix);
    auto& checkbox = widgets_.precious_resource_constraints.emplace(
      resource, new QCheckBox(precious_resource_display_name(resource), this)).first->second;
    auto& value = widgets_.precious_resource_constraint_vals.emplace(
      resource, new detail::PreciousResourceConstraintWidget(resource, this)).first->second;
    checkbox->setChecked(current_minimum > 0);
    value->setVisible(current_minimum > 0);
    value->set_value(current_minimum);
    connect(checkbox.get(), &QCheckBox::toggled, [this, resource](bool checked) {
      widgets_.precious_resource_constraint_vals.at(resource)->setVisible(checked);
    });
    resources_layout->addRow(checkbox.get(), value.get());
  }

  // Minimum yields.
  auto* yields = new QGroupBox(tr("Minimum Yields"));
  layout->addWidget(yields);
  auto* yields_layout = new QFormLayout();
  yields->setLayout(yields_layout);
  auto* yields_desc = new QLabel(tr(R"(
  Requires that a generated FrontierNav layout yield at least the specified Miranium, Revenue, and/or Storage.

  **Use of this option is discouraged**. Even seemingly small values can make it unreasonably difficult for the FnSolver
  algorithm to discover valid FrontierNav layouts. Prefer using the `ratio` Score Function instead. Provided solely for
  the curious.
  )"), this);
  yields_layout->addRow(yields_desc);
  yields_desc->setTextFormat(Qt::MarkdownText);
  yields_desc->setWordWrap(true);
  yields_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  init_min_yield_widget(&widgets_.min_mining_, &widgets_.min_mining_val_, tr("Min. Mining"),
                        solver_options->get_production_minimum());
  yields_layout->addRow(widgets_.min_mining_, widgets_.min_mining_val_);
  init_min_yield_widget(&widgets_.min_revenue_, &widgets_.min_revenue_val_, tr("Min. Revenue"),
                        solver_options->get_revenue_minimum());
  yields_layout->addRow(widgets_.min_revenue_, widgets_.min_revenue_val_);
  init_min_yield_widget(&widgets_.min_storage_, &widgets_.min_storage_val_, tr("Min. Storage"),
                        solver_options->get_storage_minimum());
  yields_layout->addRow(widgets_.min_storage_, widgets_.min_storage_val_);
}

void ConstraintsWidget::
init_min_yield_widget(QCheckBox** checkbox, QSpinBox** spinbox, const QString& label, const uint32_t value) {
  *checkbox = new QCheckBox(label, this);
  *spinbox = new QSpinBox(this);
  (*spinbox)->setMinimum(0);
  (*spinbox)->setMaximum(999999);
  (*spinbox)->setValue(value);
  (*checkbox)->setChecked(value > 0);
  (*spinbox)->setVisible(value > 0);
  connect(*checkbox, &QCheckBox::toggled, [spinbox](bool checked) {
    (*spinbox)->setVisible(checked);
  });
}
