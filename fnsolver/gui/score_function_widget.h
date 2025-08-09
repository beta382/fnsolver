#ifndef FNSOLVER_GUI_SCORE_FUNCTION_WIDGET_H
#define FNSOLVER_GUI_SCORE_FUNCTION_WIDGET_H

#include <optional>
#include <vector>
#include <unordered_set>
#include <QFormLayout>
#include <QLabel>
#include <QWidget>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include "QObjectDeleter.h"
#include "fnsolver/solver/score_function.h"

namespace detail::score_function {
/**
 * Base class for score function selector widgets.
 */
class ScoreFunctionSelectWidget : public QWidget {
  Q_OBJECT

public:
  explicit ScoreFunctionSelectWidget(QWidget* parent = nullptr);
  void set_selected(bool selected);
  bool is_selected() const;
  [[nodiscard]] virtual std::optional<ScoreFunction> get_score_function() const = 0;
  virtual std::optional<ScoreFunction::Type> get_score_function_type() const = 0;

  /** Use to add this widget's radio button to a QButtonGroup to manage exclusive state. */
  QRadioButton* radio_button() const { return radio_; }

protected:
  QFormLayout* form_;

  void set_name(const QString& name);
  void set_description(const QString& description);

private:
  QRadioButton* radio_;
  QLabel* description_;

private Q_SLOTS:
  void toggled(bool checked);
};

class NoneWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit NoneWidget(QWidget* parent = nullptr);
  [[nodiscard]] virtual std::optional<ScoreFunction> get_score_function() const;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return {};
  }
};

class MaxMiningWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit MaxMiningWidget(QWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::max_mining;
  }
};

class MaxEffectiveMiningWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit MaxEffectiveMiningWidget(QWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::max_effective_mining;
  }

private:
  QDoubleSpinBox* storage_factor_{};
};

class MaxRevenueWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit MaxRevenueWidget(QWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::max_revenue;
  }
};

class MaxStorageWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit MaxStorageWidget(QWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::max_storage;
  }
};

class RatioWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit RatioWidget(QWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::ratio;
  }

private:
  QDoubleSpinBox* mining_factor_;
  QDoubleSpinBox* revenue_factor_;
  QDoubleSpinBox* storage_factor_;
};

class WeightsWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit WeightsWidget(QWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::weights;
  }

private:
  QDoubleSpinBox* mining_weight_;
  QDoubleSpinBox* revenue_weight_;
  QDoubleSpinBox* storage_weight_;
};
} // namespace detail

template <class T>
concept ScoreFunctionSelectWidget = std::derived_from<T, detail::score_function::ScoreFunctionSelectWidget>;

class ScoreFunctionWidget : public QWidget {
  Q_OBJECT

public:
  explicit ScoreFunctionWidget(QWidget* parent = nullptr);

  void set_required(bool required);

  void set_allowed(const std::unordered_set<ScoreFunction::Type>& allowed);

  void set_selection(std::optional<ScoreFunction::Type> selection);

  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const;

Q_SIGNALS:
  void selection_changed(std::optional<ScoreFunction::Type>);

private:
  QBoxLayout* layout_{};
  QButtonGroup* button_group_{};
  bool required_ = false;
  std::unordered_set<ScoreFunction::Type> allowed_;
  std::vector<std::unique_ptr<detail::score_function::ScoreFunctionSelectWidget, QObjectDeleter>> widgets_;

  template <ScoreFunctionSelectWidget Widget_T>
  void init_scorefunction_select_widget() {
    // These widgets all have no parent as the custom deleter takes care of memory management.
    auto& widget = widgets_.emplace_back(new Widget_T());
    button_group_->addButton(widget->radio_button());
    layout_->addWidget(widget.get());
  }
};

#endif //FNSOLVER_GUI_SCORE_FUNCTION_WIDGET_H
