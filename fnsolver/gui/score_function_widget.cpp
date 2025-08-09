#include "score_function_widget.h"
#include <QButtonGroup>
#include <QApplication>
#include <qstyle.h>

namespace detail::score_function {
ScoreFunctionSelectWidget::ScoreFunctionSelectWidget(QWidget* parent): QWidget(parent),
  form_(new QFormLayout),
  radio_(new QRadioButton(this)),
  description_(new DescriptionWidget(this)) {
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(radio_);
  layout->addWidget(description_);
  // Will be shown if a description is set.
  description_->setVisible(false);
  layout->addLayout(form_);

  // Calculate where the left edge of the radio button's label will be and indent the form by that amount.
  const auto* style = qApp->style();
  form_->setContentsMargins(
    style->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth) + style->pixelMetric(QStyle::PM_RadioButtonLabelSpacing),
    0, 0, 0
  );

  connect(radio_, &QRadioButton::toggled, this, &ScoreFunctionSelectWidget::toggled);
}

void ScoreFunctionSelectWidget::set_selected(bool selected) {
  radio_->setChecked(selected);
  toggled(selected);
}

bool ScoreFunctionSelectWidget::is_selected() const {
  return radio_->isChecked();
}

void ScoreFunctionSelectWidget::set_name(const QString& name) {
  radio_->setText(name);
}

void ScoreFunctionSelectWidget::set_description(const QString& description) {
  description_->set_text(description);
  description_->setVisible(!description.isEmpty());
}

void ScoreFunctionSelectWidget::toggled(bool checked) {
  // Disable/enable form widgets.
  for (int item_ix = 0; item_ix < form_->count(); ++item_ix) {
    auto* item = form_->itemAt(item_ix);
    assert(item);
    auto* widget = item->widget();
    if (widget == nullptr) {
      // Not a widget.
      continue;
    }
    widget->setEnabled(checked);
  }
}

NoneWidget::NoneWidget(QWidget* parent): ScoreFunctionSelectWidget(parent) {
  set_name(tr("None"));
}

std::optional<ScoreFunction> NoneWidget::get_score_function() const {
  return {};
}

MaxMiningWidget::MaxMiningWidget(QWidget* parent): ScoreFunctionSelectWidget(parent) {
  set_name(tr("Max Mining"));
  set_description(tr(R"(
- Uses "Mining yield" as the score.
- It is recommended to use `Max Effective Mining`, as that tends to be more interesting in practice.
)"));
}

std::optional<ScoreFunction> MaxMiningWidget::get_score_function() const {
  return ScoreFunction::create_max_mining();
}

MaxEffectiveMiningWidget::MaxEffectiveMiningWidget(QWidget* parent): ScoreFunctionSelectWidget(parent),
  storage_factor_(new QDoubleSpinBox(this)) {
  set_name(tr("Max Effective Mining"));
  set_description(tr(R"(
- Uses the lesser of "Total Storage" or "Mining Yield" &times; `Storage Factor` as the score.
- Represents "How much Minanium can I get in `Storage Factor` FrontierNav cycles, capped by my Storage".
)"));

  form_->addRow(tr("Storage Factor"), storage_factor_);
  storage_factor_->setMinimum(0);
}

std::optional<ScoreFunction> MaxEffectiveMiningWidget::get_score_function() const {
  return ScoreFunction::create_max_effective_mining(storage_factor_->value());
}

MaxRevenueWidget::MaxRevenueWidget(QWidget* parent): ScoreFunctionSelectWidget(parent) {
  set_name(tr("Max Revenue"));
  set_description(tr(R"(
- Uses "Revenue Yield" as the score.
)"));
}

std::optional<ScoreFunction> MaxRevenueWidget::get_score_function() const {
  return ScoreFunction::create_max_revenue();
}

MaxStorageWidget::MaxStorageWidget(QWidget* parent): ScoreFunctionSelectWidget(parent) {
  set_name(tr("Max Storage"));
  set_description(tr(R"(
- Uses "Total Storage" as the score.
)"));
}

std::optional<ScoreFunction> MaxStorageWidget::get_score_function() const {
  return ScoreFunction::create_max_storage();
}

RatioWidget::RatioWidget(QWidget* parent): ScoreFunctionSelectWidget(parent),
  mining_factor_(new QDoubleSpinBox(this)),
  revenue_factor_(new QDoubleSpinBox(this)),
  storage_factor_(new QDoubleSpinBox(this)) {
  set_name(tr("Ratio"));
  set_description(tr(R"(
- Takes three argument: `Mining Factor`, `Revenue Factor`, and `Storage Factor`
- Uses the lesser of "yield divided by its `factor` times the largest `factor`" for each yield type with a non-zero
  `factor` as the score.
- Effectively maximizes your yields while maintaining the specified ratio between them.
  )"));

  form_->addRow(tr("Mining Factor"), mining_factor_);
  mining_factor_->setMinimum(0);
  form_->addRow(tr("Revenue Factor"), revenue_factor_);
  revenue_factor_->setMinimum(0);
  form_->addRow(tr("Storage Factor"), storage_factor_);
  storage_factor_->setMinimum(0);
}

std::optional<ScoreFunction> RatioWidget::get_score_function() const {
  return ScoreFunction::create_ratio(mining_factor_->value(), revenue_factor_->value(), storage_factor_->value());
}

WeightsWidget::WeightsWidget(QWidget* parent): ScoreFunctionSelectWidget(parent),
  mining_weight_(new QDoubleSpinBox(this)),
  revenue_weight_(new QDoubleSpinBox(this)),
  storage_weight_(new QDoubleSpinBox(this)) {
  set_name(tr("Weights"));
  set_description(tr(R"(
- Takes three argument: `Mining Weight`, `Revenue Weight`, and `Storage Weight`
- Uses the sum of "yield times its `weight`" for each yield type as the score.
- **Generally not recommended**, since it effectively just maximizes the yield type with the highest `weight`. It is
  recommended to the other Score Functions instead. Provided solely for parity with XenoProbes.
)"));

  form_->addRow(tr("Mining Weight"), mining_weight_);
  mining_weight_->setMinimum(0);
  form_->addRow(tr("Revenue Weight"), revenue_weight_);
  revenue_weight_->setMinimum(0);
  form_->addRow(tr("Storage Weight"), storage_weight_);
  storage_weight_->setMinimum(0);
}

std::optional<ScoreFunction> WeightsWidget::get_score_function() const {
  return ScoreFunction::create_weights(mining_weight_->value(), revenue_weight_->value(), storage_weight_->value());
}
} // namespace detail::score_function

ScoreFunctionWidget::ScoreFunctionWidget(QWidget* parent): QWidget(parent), layout_(new QVBoxLayout(this)),
  button_group_(new QButtonGroup(this)) {
  set_allowed({});
}

void ScoreFunctionWidget::set_required(bool required) {
  required_ = required;
  set_allowed(allowed_);
}

void ScoreFunctionWidget::set_allowed(const std::unordered_set<ScoreFunction::Type>& allowed) {
  allowed_ = allowed;

  // Reset the button group.
  for (const auto& widget : widgets_) {
    button_group_->removeButton(widget->radio_button());
  }

  // Reset the layout.
  for (int item_ix = layout_->count(); item_ix != 0; --item_ix) {
    layout_->takeAt(item_ix - 1);
  }
  widgets_.clear();

  // These widgets all have no parent as the custom deleter takes care of memory management.
  if (!required_) {
    auto& none = widgets_.emplace_back(new detail::score_function::NoneWidget());
    button_group_->addButton(none->radio_button());
    layout_->addWidget(none.get());
  }

  if (allowed.contains(ScoreFunction::Type::max_mining)) {
    init_scorefunction_select_widget<detail::score_function::MaxMiningWidget>();
  }
  if (allowed.contains(ScoreFunction::Type::max_effective_mining)) {
    init_scorefunction_select_widget<detail::score_function::MaxEffectiveMiningWidget>();
  }
  if (allowed.contains(ScoreFunction::Type::max_revenue)) {
    init_scorefunction_select_widget<detail::score_function::MaxRevenueWidget>();
  }
  if (allowed.contains(ScoreFunction::Type::max_storage)) {
    init_scorefunction_select_widget<detail::score_function::MaxStorageWidget>();
  }
  if (allowed.contains(ScoreFunction::Type::ratio)) {
    init_scorefunction_select_widget<detail::score_function::RatioWidget>();
  }
  if (allowed.contains(ScoreFunction::Type::weights)) {
    init_scorefunction_select_widget<detail::score_function::WeightsWidget>();
  }

  // Adding a stretch at the end makes the items stack like a list.
  layout_->addStretch();
}

std::optional<ScoreFunction> ScoreFunctionWidget::get_score_function() const {
  for (const auto& widget : widgets_) {
    if (widget->is_selected()) {
      return widget->get_score_function();
    }
  }
  return {};
}

void ScoreFunctionWidget::set_selection(std::optional<ScoreFunction::Type> selection) {
  for (const auto& widget : widgets_) {
    widget->set_selected(widget->get_score_function_type() == selection);
  }
}
