#ifndef FNSOLVER_SOLVER_SOLVER_H
#define FNSOLVER_SOLVER_SOLVER_H

#include <fnsolver/data/probe.h>
#include <fnsolver/layout/layout.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/solver/options.h>
#include <fnsolver/solver/solution.h>

#include <atomic>
#include <cstdint>
#include <optional>
#include <ostream>
#include <random>
#include <utility>
#include <vector>

class Solver {
  public:
    struct IterationStatus {
      uint32_t iteration;
      const Solution& best_solution;
      std::size_t num_killed;
      uint32_t last_improvement;
    };
    using ProgressCallback = std::function<void(IterationStatus)>;
    using StopCallback = std::function<bool()>;

    Solver(Options options);

    Solver(const Solver &other) = delete;
    Solver(Solver &&other) = delete;
    Solver &operator=(const Solver &other) = delete;
    Solver &operator=(Solver &&other) = delete;

    Solution run(ProgressCallback progress_callback, StopCallback stop_callback) const;

  private:
    Options options;

    ScoreFunction constrained_score_function;
    std::vector<Placement> merged_locked_sites_and_seed;
    std::vector<bool> site_idx_is_seeded;
    std::vector<const Probe *> inventory;

    Solution create_random_solution(std::mt19937 &mt_engine) const;
    std::pair<Solution, bool> create_solution_children_and_find_best(
        Solution solution,
        const Solution &best_solution,
        std::mt19937 &mt_engine) const;
    Solution create_solution_mutation(const Solution &solution, std::mt19937 &mt_engine) const;
};

#endif // FNSOLVER_SOLVER_SOLVER_H

