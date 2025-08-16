#include "solver_runner.h"
#include "fnsolver/data/probe.h"

SolverRunner::SolverRunner(const Options& options, QObject* parent): QThread(parent), solver_options_(options) {
  // Tweak the inventory to remove probes stored in the seed, then fill it with basic probes to ensure there are enough
  // probes available to fill all unlocked sites. These changes are to satisfy the solver algorithm and are not saved
  // to disk with the user's entries.
  for (const auto& placement : solver_options_.get_seed()) {
    const auto& probe = placement.get_probe();
    if (probe.probe_type == Probe::Type::none || probe.probe_type == Probe::Type::basic) {
      continue;
    }
    --(solver_options_.mutable_probe_quantities().at(probe.probe_id));
  }
  const auto num_probes = std::accumulate(solver_options_.get_probe_quantities().cbegin(),
                                          solver_options_.get_probe_quantities().cend(), 0u);
  const auto num_locked_sites = solver_options_.get_locked_sites().size();
  const auto seed_size = solver_options_.get_seed().size();
  const auto num_probes_required = FnSite::sites.size() - num_locked_sites - seed_size;
  if (num_probes < num_probes_required) {
    solver_options_.mutable_probe_quantities().at(Probe::idx_for_shorthand.at("-")) += num_probes_required - num_probes;
  }
}

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
