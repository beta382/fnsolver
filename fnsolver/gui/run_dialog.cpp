#include "run_dialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <qstyle.h>

#include "options_loader.h"
#include "run_progress_dialog.h"

RunDialog::RunDialog(Options* solver_options, QWidget* parent): QDialog(parent), solver_options_(solver_options) {
  resize(640, 480);

  auto* layout = new QVBoxLayout(this);
  auto* tabs = new QTabWidget(this);
  layout->addWidget(tabs);

  // Score function
  auto* scorefunction_container = new QWidget(this);
  auto* scorefunction_layout = new QVBoxLayout(scorefunction_container);
  // Stop adding extra margins on the contained widgets...
  scorefunction_layout->setContentsMargins(0, 0, 0, 0);
  auto* scorefunction_desc = new DescriptionWidget(this);
  scorefunction_desc->set_text(tr(R"(
The Score Function is used to calculate the "score" for a generated FrontierNav layout, which is then used to compare
different layouts and determine which one is "better".
)"));
  // ... but need to add them back to make this descriptive text look nice.
  const auto* style = qApp->style();
  scorefunction_desc->setContentsMargins(
    style->pixelMetric(QStyle::PM_LayoutLeftMargin),
    style->pixelMetric(QStyle::PM_LayoutTopMargin),
    style->pixelMetric(QStyle::PM_LayoutRightMargin),
    style->pixelMetric(QStyle::PM_LayoutBottomMargin)
  );
  scorefunction_layout->addWidget(scorefunction_desc);
  widgets_.scorefunction = new ScoreFunctionWidget(this);
  scorefunction_layout->addWidget(widgets_.scorefunction);
  auto* scoreFunctionScroll = new QScrollArea(this);
  scoreFunctionScroll->setWidget(scorefunction_container);
  scoreFunctionScroll->setWidgetResizable(true);
  tabs->addTab(scoreFunctionScroll, tr("Score Function"));
  widgets_.scorefunction->set_required(true);
  widgets_.scorefunction->set_allowed({
    ScoreFunction::Type::max_mining,
    ScoreFunction::Type::max_effective_mining,
    ScoreFunction::Type::max_revenue,
    ScoreFunction::Type::max_storage,
    ScoreFunction::Type::ratio,
    ScoreFunction::Type::weights,
  });
  widgets_.scorefunction->set_selection(solver_options_->get_score_function());
  connect(widgets_.scorefunction, &ScoreFunctionWidget::selection_changed, this, &RunDialog::validate);

  // Tiebreaker
  auto* tiebreaker_container = new QWidget(this);
  auto* tiebreaker_layout = new QVBoxLayout(tiebreaker_container);
  tiebreaker_layout->setContentsMargins(0, 0, 0, 0);
  auto* tiebreaker_desc = new DescriptionWidget(this);
  tiebreaker_desc->set_text(tr(R"(
Generally, this is only interesting with "Max Storage", "Max Effective Mining, or "Ratio" as the Score Function. With
"Max Mining" or "Max Revenue" as the Score Function, the non-zero efficacy on Research and Mining probes respectively
make a tiebreaker largely low-impact. With "Weights" as the Score Function, a tiebreaker is effectively built-in.
)"));
  tiebreaker_desc->setContentsMargins(
    style->pixelMetric(QStyle::PM_LayoutLeftMargin),
    style->pixelMetric(QStyle::PM_LayoutTopMargin),
    style->pixelMetric(QStyle::PM_LayoutRightMargin),
    style->pixelMetric(QStyle::PM_LayoutBottomMargin)
  );
  tiebreaker_layout->addWidget(tiebreaker_desc);
  widgets_.tiebreaker = new ScoreFunctionWidget(tiebreaker_container);
  tiebreaker_layout->addWidget(widgets_.tiebreaker);
  auto* tiebreakerScroll = new QScrollArea(this);
  tiebreakerScroll->setWidget(tiebreaker_container);
  tiebreakerScroll->setWidgetResizable(true);
  tabs->addTab(tiebreakerScroll, tr("Tiebreaker"));
  widgets_.tiebreaker->set_required(false);
  widgets_.tiebreaker->set_allowed({
    ScoreFunction::Type::max_mining,
    ScoreFunction::Type::max_revenue,
    ScoreFunction::Type::max_storage,
  });
  connect(widgets_.tiebreaker, &ScoreFunctionWidget::selection_changed, this, &RunDialog::validate);
  if (solver_options_->get_maybe_tiebreaker_function().has_value()) {
    widgets_.tiebreaker->set_selection(solver_options_->get_maybe_tiebreaker_function());
  }

  // Constraints
  widgets_.constraints = new ConstraintsWidget(solver_options_, this);
  auto* constraints_scroll = new QScrollArea(this);
  constraints_scroll->setWidget(widgets_.constraints);
  constraints_scroll->setWidgetResizable(true);
  tabs->addTab(constraints_scroll, tr("Constraints"));

  // Solver Params
  widgets_.solver_params = new SolverParamsWidget(solver_options_, this);
  auto* solver_params_scroll = new QScrollArea(this);
  solver_params_scroll->setWidget(widgets_.solver_params);
  solver_params_scroll->setWidgetResizable(true);
  tabs->addTab(solver_params_scroll, tr("Solver Parameters"));

  // Errors
  widgets_.errors = new QLabel(this);
  widgets_.errors->setTextFormat(Qt::RichText);
  widgets_.errors->setVisible(false);
  layout->addWidget(widgets_.errors);

  // Buttons
  auto* buttons = new QDialogButtonBox(this);
  layout->addWidget(buttons);
  buttons->addButton(QDialogButtonBox::Cancel);
  widgets_.solve = buttons->addButton(tr("Solve"), QDialogButtonBox::AcceptRole);
  connect(buttons, &QDialogButtonBox::accepted, this, &RunDialog::solve);
  connect(buttons, &QDialogButtonBox::rejected, this, &RunDialog::reject);

  validate();
}

void RunDialog::solve() {
  // Create options instance.
  Options solver_options = *solver_options_;

  // None isn't allowed on scorefunction, so assume it has a value.
  solver_options.set_score_function(widgets_.scorefunction->get_score_function().value());
  solver_options.set_maybe_tiebreaker_function(widgets_.tiebreaker->get_score_function());
  widgets_.constraints->apply_to_options(&solver_options);
  widgets_.solver_params->apply_to_options(&solver_options);

  if (solver_options != *solver_options_) {
    Q_EMIT options_changed(solver_options);
  }

  auto* run_progress = new RunProgressDialog(solver_options, this);
  connect(run_progress, &RunProgressDialog::solved, this, &RunDialog::solved);
  connect(run_progress, &RunProgressDialog::solved, this, &RunDialog::accept);
  run_progress->show();
}

void RunDialog::validate() {
  if (widgets_.errors == nullptr || widgets_.solve == nullptr) {
    return;
  }

  std::vector<std::function<void(QStringList& errors)>> validators{
    [this](QStringList& errors) {
      // Ensure the score function and tiebreaker are different.
      if (widgets_.scorefunction->get_score_function() == widgets_.tiebreaker->get_score_function()) {
        errors.push_back(tr("Score Function and Tiebreaker must be different."));
      }
    }
  };

  QStringList errors;
  for (const auto& validator : validators) {
    validator(errors);
  }
  widgets_.errors->setText(QString("<div style=\"color:#FF0000\">%1</div>").arg(errors.join("<br>")));
  widgets_.errors->setVisible(!errors.isEmpty());
  widgets_.solve->setEnabled(errors.isEmpty());
}
