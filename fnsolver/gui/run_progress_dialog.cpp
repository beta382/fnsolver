#include "run_progress_dialog.h"

#include <QPushButton>
#include <QTime>

RunProgressDialog::RunProgressDialog(const Options& solver_options, QWidget* parent):
  QDialog(parent), solver_options_(solver_options) {
  setWindowTitle(tr("Calculating..."));
  setModal(true);
  auto* layout = new QFormLayout(this);

  // Setup solver.
  solver_runner_ = new SolverRunner(solver_options, this);
  connect(solver_runner_, &SolverRunner::progress, this, &RunProgressDialog::progress);
  connect(solver_runner_, &SolverRunner::solved, this, &RunProgressDialog::solved);

  // Status
  widgets_.iteration = new QLabel(this);
  layout->addRow(widgets_.iteration);
  widgets_.iteration->setAlignment(Qt::AlignCenter);
  widgets_.time_elapsed = new QLabel(this);
  layout->addRow(widgets_.time_elapsed);
  widgets_.time_elapsed->setAlignment(Qt::AlignCenter);
  widgets_.time_remaining = new QLabel(this);
  layout->addRow(widgets_.time_remaining);
  widgets_.time_remaining->setAlignment(Qt::AlignCenter);
  widgets_.best_score = new QLabel(this);
  layout->addRow(tr("Overall Best Score"), widgets_.best_score);
  widgets_.killed = new QLabel(this);
  layout->addRow(tr("Solutions Killed"), widgets_.killed);
  widgets_.last_improvement = new QLabel(this);
  layout->addRow(tr("Last Improvement"), widgets_.last_improvement);

  // Yields
  widgets_.mining = new QLabel(this);
  layout->addRow(tr("Mining"), widgets_.mining);
  widgets_.revenue = new QLabel(this);
  layout->addRow(tr("Revenue"), widgets_.revenue);
  widgets_.storage = new QLabel(this);
  layout->addRow(tr("Storage"), widgets_.storage);

  // Progress bar
  widgets_.progress_bar = new QProgressBar(this);
  layout->addRow(widgets_.progress_bar);
  widgets_.progress_bar->setMinimum(0);
  widgets_.progress_bar->setMaximum(solver_options_.get_iterations());

  // Cancel
  auto* cancel = new QPushButton(tr("Cancel"), this);
  layout->addRow(cancel);
  connect(cancel, &QPushButton::clicked, solver_runner_, &SolverRunner::requestInterruption);

  // Start
  solver_stopwatch_.start();
  solver_runner_->start();
}

void RunProgressDialog::progress(const Solver::IterationStatus& iteration_status) {
  const QLocale locale;

  // Time estimation
  const auto time_format = QStringLiteral("HH:mm:ss");
  const auto time_elapsed = QTime::fromMSecsSinceStartOfDay(solver_stopwatch_.elapsed());
  const auto total_time_required = QTime::fromMSecsSinceStartOfDay(
    (time_elapsed.msecsSinceStartOfDay() / iteration_status.iteration) * solver_options_.get_iterations());
  const auto time_remaining = QTime::fromMSecsSinceStartOfDay(
    total_time_required.msecsSinceStartOfDay() - time_elapsed.msecsSinceStartOfDay());
  widgets_.iteration->setText(tr("%1 of %2")
                              .arg(iteration_status.iteration)
                              .arg(solver_options_.get_iterations())
  );
  widgets_.time_elapsed->setText(tr("%1 / %2")
                                 .arg(locale.toString(time_elapsed, time_format))
                                 .arg(locale.toString(total_time_required, time_format))
  );
  widgets_.time_remaining->setText(tr("%1 remaining").arg(locale.toString(time_remaining, time_format)));

  // Status
  widgets_.best_score->setText(locale.toString(iteration_status.best_score));
  widgets_.killed->setText(locale.toString(iteration_status.num_killed));
  const auto last_improvement_iteration = iteration_status.iteration - iteration_status.last_improvement;
  widgets_.last_improvement->setText(last_improvement_iteration == 0
                                       ? tr("This iteration")
                                       : tr("%n iteration(s) ago", "", last_improvement_iteration));

  // Yields
  const auto resource_yield = iteration_status.best_layout.get_resource_yield();
  widgets_.mining->setText(locale.toString(resource_yield.get_production()));
  widgets_.revenue->setText(locale.toString(resource_yield.get_revenue()));
  widgets_.storage->setText(locale.toString(resource_yield.get_storage()));

  // Progress bar
  widgets_.progress_bar->setValue(iteration_status.iteration);
}
