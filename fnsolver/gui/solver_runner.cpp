#include "solver_runner.h"

SolverRunner::SolverRunner(const Options& options, QObject* parent): QThread(parent), solver_options_(options) {}

void SolverRunner::run() {
  const Solver solver(solver_options_);
  auto progress_callback = [this](const Solver::IterationStatus& iteration_status) {
    Q_EMIT progress(iteration_status);
  };
  auto stop_callback = [this]() {
    return isInterruptionRequested();
  };
  const Solution solution = solver.run(progress_callback, stop_callback);
  Q_EMIT(solved(solution.get_layout()));
}
