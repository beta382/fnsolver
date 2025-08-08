#include "constraints_widget.h"
#include <QFormLayout>
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
  widgets_.constraint_type->addItem(tr("Average Yield"), static_cast<int>(Type::Value));
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
  auto* layout = new QFormLayout(this);

  // Precious Resources.
  for (int resource_ix = 0; resource_ix < precious_resource::count; ++resource_ix) {
    const auto resource = static_cast<precious_resource::Type>(resource_ix);
    const auto current_minimum = solver_options->get_precious_resource_minimums().at(resource_ix);
    auto& checkbox = widgets_.precious_resource_constraints.emplace(
      resource, new QCheckBox(precious_resource_display_name(resource), this)).first->second;
    checkbox->setChecked(current_minimum > 0);
    auto& value = widgets_.precious_resource_constraint_vals.emplace(
      resource, new detail::PreciousResourceConstraintWidget(resource, this)).first->second;
    value->setVisible(current_minimum > 0);
    value->set_value(current_minimum);
    connect(checkbox.get(), &QCheckBox::toggled, [this, resource](bool checked) {
      widgets_.precious_resource_constraint_vals.at(resource)->setVisible(checked);
    });
    layout->addRow(checkbox.get(), value.get());
  }
}
