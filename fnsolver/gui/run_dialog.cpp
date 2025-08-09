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
  widgets_.scorefunction = new ScoreFunctionWidget(this);
  auto* scoreFunctionScroll = new QScrollArea(this);
  scoreFunctionScroll->setWidget(widgets_.scorefunction);
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
  widgets_.scorefunction->set_selection(
    ScoreFunction::type_for_str.at(solver_options_->get_score_function().get_name())
  );

  // Tiebreaker
  auto* tiebreaker_container = new QWidget(this);
  auto* tiebreaker_layout = new QVBoxLayout(tiebreaker_container);
  // Stop adding extra margins on the contained widgets...
  tiebreaker_layout->setContentsMargins(0, 0, 0, 0);
  auto* tiebreaker_desc = new QLabel(tr(R"(
Generally, this is only interesting with "Max Storage", "Max Effective Mining, or "Ratio" as the Score Function. With
"Max Mining" or "Max Revenue" as the Score Function, the non-zero efficacy on Research and Mining probes respectively
make a tiebreaker largely low-impact. With "Weights" as the Score Function, a tiebreaker is effectively built-in.
  )"), tiebreaker_container);
  tiebreaker_desc->setTextFormat(Qt::MarkdownText);
  tiebreaker_desc->setWordWrap(true);
  // ... but need to add them back to make this descriptive text look nice.
  const auto* style = qApp->style();
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
  if (solver_options_->get_maybe_tiebreaker_function().has_value()) {
    widgets_.tiebreaker->set_selection(
      ScoreFunction::type_for_str.at(solver_options_->get_maybe_tiebreaker_function()->get_name())
    );
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

  // Buttons.
  auto* buttons = new QDialogButtonBox(this);
  layout->addWidget(buttons);
  buttons->addButton(QDialogButtonBox::Cancel);
  buttons->addButton(tr("Solve"), QDialogButtonBox::AcceptRole);
  // TODO: Make this do run the simulation.
  connect(buttons, &QDialogButtonBox::accepted, this, &RunDialog::solve);
  connect(buttons, &QDialogButtonBox::rejected, this, &RunDialog::reject);
}

void RunDialog::solve() {
  // Create options instance.
  Options solver_options = *solver_options_;

  // None isn't allowed on scorefunction, so assume it has a value.
  solver_options.set_score_function(widgets_.scorefunction->get_score_function().value());
  solver_options.set_maybe_tiebreaker_function(widgets_.tiebreaker->get_score_function());
  widgets_.constraints->apply_to_options(&solver_options);
  widgets_.solver_params->apply_to_options(&solver_options);
  if (!widgets_.solver_params->get_seed()) {
    solver_options.set_seed({});
    solver_options.set_force_seed(false);
  }

  if (solver_options != *solver_options_) {
    Q_EMIT options_changed(solver_options);
  }

  auto* run_progress = new RunProgressDialog(solver_options, this);
  connect(run_progress, &RunProgressDialog::solved, this, &RunDialog::solved);
  connect(run_progress, &RunProgressDialog::solved, this, &RunDialog::accept);
  run_progress->show();
}
