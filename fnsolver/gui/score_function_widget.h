#ifndef FNSOLVER_GUI_SCORE_FUNCTION_WIDGET_H
#define FNSOLVER_GUI_SCORE_FUNCTION_WIDGET_H

#include <optional>
#include <vector>
#include <unordered_set>
#include <QFormLayout>
#include <QWidget>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include "description_widget.h"
#include "QObjectDeleter.h"
#include "fnsolver/solver/score_function.h"

class ScoreFunctionWidget;

namespace detail::score_function {
/**
 * Base class for score function selector widgets.
 */
class ScoreFunctionSelectWidget : public QWidget {
  Q_OBJECT

public:
  explicit ScoreFunctionSelectWidget(ScoreFunctionWidget* parent = nullptr);
  void set_selected(bool selected);

  virtual void set_args(const ScoreFunction::args_map_t&) {}

  bool is_selected() const;
  [[nodiscard]] virtual std::optional<ScoreFunction> get_score_function() const = 0;
  virtual std::optional<ScoreFunction::Type> get_score_function_type() const = 0;

  /** Use to add this widget's radio button to a QButtonGroup to manage exclusive state. */
  QRadioButton* radio_button() const { return radio_; }

protected:
  QFormLayout* form_;

  void set_name(const QString& name);
  void set_description(const QString& description);
  std::unordered_set<ScoreFunction::Type> get_siblings() const;

private:
  QRadioButton* radio_;
  DescriptionWidget* description_;

private Q_SLOTS:
  void toggled(bool checked);
};

class NoneWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit NoneWidget(ScoreFunctionWidget* parent = nullptr);
  [[nodiscard]] virtual std::optional<ScoreFunction> get_score_function() const;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return {};
  }
};

class MaxMiningWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit MaxMiningWidget(ScoreFunctionWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::max_mining;
  }
};

class MaxEffectiveMiningWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit MaxEffectiveMiningWidget(ScoreFunctionWidget* parent = nullptr);
  void set_args(const ScoreFunction::args_map_t& args) override;
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
  explicit MaxRevenueWidget(ScoreFunctionWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::max_revenue;
  }
};

class MaxStorageWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit MaxStorageWidget(ScoreFunctionWidget* parent = nullptr);
  [[nodiscard]] std::optional<ScoreFunction> get_score_function() const override;

  std::optional<ScoreFunction::Type> get_score_function_type() const override {
    return ScoreFunction::Type::max_storage;
  }
};

class RatioWidget : public ScoreFunctionSelectWidget {
  Q_OBJECT

public:
  explicit RatioWidget(ScoreFunctionWidget* parent = nullptr);
  void set_args(const ScoreFunction::args_map_t& args) override;
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
  explicit WeightsWidget(ScoreFunctionWidget* parent = nullptr);
  void set_args(const ScoreFunction::args_map_t& args) override;
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

  [[nodiscard]] const std::unordered_set<ScoreFunction::Type>& get_allowed() const { return allowed_; }

  void set_selection(const std::optional<ScoreFunction>& selection);

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
    auto& widget = widgets_.emplace_back(new Widget_T(this));
    const auto scorefunction_type = widget->get_score_function_type();
    button_group_->addButton(widget->radio_button());
    layout_->addWidget(widget.get());
    connect(widget->radio_button(), &QRadioButton::toggled, [this, scorefunction_type]() {
      Q_EMIT(selection_changed(scorefunction_type));
    });
  }
};

#endif //FNSOLVER_GUI_SCORE_FUNCTION_WIDGET_H
