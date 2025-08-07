#include "run_dialog.h"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>

RunDialog::RunDialog(Options* solver_options, QWidget* parent): QDialog(parent), solver_options_(solver_options) {
  resize(640, 480);

  auto* layout = new QVBoxLayout(this);
  auto* tabs = new QTabWidget(this);
  layout->addWidget(tabs);

  // Score function
  widgets_.scorefunction = new ScoreFunctionWidget(this);
  tabs->addTab(widgets_.scorefunction, tr("Score Function"));
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
  // auto tiebreaker = new QGroupBox(tr("Tie Breaker"), scorefunction_tiebreaker);
  // scorefunction_tiebreaker_layout->addWidget(tiebreaker);
  // auto* tiebreaker_scroll = new QScrollArea(tiebreaker);
  // widgets_.tiebreaker = new ScoreFunctionWidget(tiebreaker_scroll);
  // widgets_.tiebreaker->set_allowed({
  //   ScoreFunction::Type::max_mining,
  //   ScoreFunction::Type::max_revenue,
  //   ScoreFunction::Type::max_storage,
  // });

  // Buttons.
  auto* buttons = new QDialogButtonBox(this);
  layout->addWidget(buttons);
  buttons->addButton(QDialogButtonBox::Cancel);
  // TODO: Make this do run the simulation.
  connect(buttons, &QDialogButtonBox::accepted, this, &RunDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &RunDialog::reject);
}
