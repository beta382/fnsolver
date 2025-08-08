#ifndef FNSOLVER_GUI_CONSTRAINTS_WIDGET_H
#define FNSOLVER_GUI_CONSTRAINTS_WIDGET_H

#include <unordered_map>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include "QObjectDeleter.h"
#include "fnsolver/solver/options.h"

namespace detail {
class PreciousResourceConstraintWidget : public QWidget {
  Q_OBJECT

public:
  explicit PreciousResourceConstraintWidget(precious_resource::Type precious_resource, QWidget* parent);

  [[nodiscard]] uint32_t get_value() const;
  void set_value(uint32_t value);

private:
  enum class Type {
    All = 0,
    Any,
    Percent,
    Value,
  };

  struct Widgets {
    QComboBox* constraint_type = nullptr;
    QDoubleSpinBox* constraint_value = nullptr;
    QLabel* constraint_max = nullptr;
  };

  Widgets widgets_;
  precious_resource::Type precious_resource_;
  Type type_ = Type::Any;

  void set_type(Type type);

  void update_value_spinbox_range();
private Q_SLOTS:
  void constraint_type_changed(int index);
};
} // namespace detail

class ConstraintsWidget : public QWidget {
  Q_OBJECT

public:
  explicit ConstraintsWidget(const Options* solver_options, QWidget* parent = nullptr);

private:
  struct Widgets {
    std::unordered_map<precious_resource::Type, std::unique_ptr<QCheckBox, QObjectDeleter>>
    precious_resource_constraints;
    std::unordered_map<precious_resource::Type, std::unique_ptr<
                         detail::PreciousResourceConstraintWidget, QObjectDeleter>>
    precious_resource_constraint_vals;
  };

  Widgets widgets_;
};


#endif //FNSOLVER_GUI_CONSTRAINTS_WIDGET_H
