#include <fnsolver/solver/solver.h>

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/probe.h>
#include <fnsolver/layout/layout.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/solver/options.h>
#include <fnsolver/solver/solution.h>

#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstdint>
#include <format>
#include <future>
#include <mutex>
#include <numeric>
#include <optional>
#include <ostream>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {
ScoreFunction create_constrained_score_function(const Options &options) {
  const std::vector<size_t> nonzero_precious_resource_minimum_idxs = [&]() {
    std::vector<size_t> nonzero_precious_resource_minimum_idxs;
    for (size_t i = 0; i < options.get_precious_resource_minimums().size(); ++i) {
      if (options.get_precious_resource_minimums().at(i) != 0) {
        nonzero_precious_resource_minimum_idxs.push_back(i);
      }
    }
    return nonzero_precious_resource_minimum_idxs;
  }();

  return ScoreFunction(
      [&options, nonzero_precious_resource_minimum_idxs](const Layout &layout) {
        const std::array<uint32_t, precious_resource::count> &precious_resource_quantities
            = layout.get_resource_yield().get_precious_resource_quantities();
        const std::array<uint32_t, precious_resource::count> &precious_resource_minimums
            = options.get_precious_resource_minimums();
        for (const size_t idx : nonzero_precious_resource_minimum_idxs) {
          if (precious_resource_quantities.at(idx) < precious_resource_minimums.at(idx)) {
            return 0.0;
          }
        }

        if (layout.get_resource_yield().get_production() < options.get_production_minimum()
            || layout.get_resource_yield().get_revenue() < options.get_revenue_minimum()
            || layout.get_resource_yield().get_storage() < options.get_storage_minimum()) {
          return 0.0;
        }

        return options.get_score_function()(layout);
      },
      options.get_score_function().get_name(), options.get_score_function().get_args());
}

std::vector<Placement> merge_locked_sites_and_seed(const Options &options) {
  std::vector<Placement> merged_seed;
  const std::vector<Placement> &locked_sites = options.get_locked_sites();
  const std::vector<Placement> &seed = options.get_seed();
  std::merge(
      locked_sites.cbegin(),
      locked_sites.cend(),
      seed.cbegin(),
      seed.cend(),
      std::back_inserter(merged_seed),
      &Placement::sort_cmp);
  return merged_seed;
}
} // namespace

Solver::Solver(Options options)
    : options(std::move(options)),
      constrained_score_function(create_constrained_score_function(this->options)),
      merged_locked_sites_and_seed(merge_locked_sites_and_seed(this->options)),
      site_idx_is_seeded([this]() {
        std::vector<bool> site_idx_is_seeded(FnSite::sites.size(), false);
        for (const Placement &placement : this->merged_locked_sites_and_seed) {
          site_idx_is_seeded[FnSite::idx_for_id.at(placement.get_site().site_id)] = true;
        }
        return site_idx_is_seeded;
      }()),
      inventory([this]() {
        std::vector<const Probe *> inventory;
        for (size_t probe_id = 0; probe_id < this->options.get_probe_quantities().size(); ++probe_id) {
          std::fill_n(
              std::back_inserter(inventory),
              this->options.get_probe_quantities().at(probe_id),
              &Probe::probes.at(probe_id));
        }
        return inventory;
      }()) {}

Solution Solver::run(const ProgressCallback& progress_callback, const StopCallback& stop_callback) const {
  std::mt19937 mt_engine(std::random_device{}());
  std::vector<Solution> population;
  for (uint32_t i = 0; i < options.get_population_size(); ++i) {
    population.emplace_back(create_random_solution(mt_engine));
  }

  Solution best_solution = population.at(0); // doesn't really matter, so don't calculate actual max
  uint32_t last_improvement_iteration = 0;
  uint32_t iteration = 0;
  do { // run one iteration even if the user terminated before it started, so that there's some meaningful result
    ++iteration;

    std::vector<Solution> new_generation;
    size_t num_killed = 0;

    std::vector<std::thread> threads;
    std::vector<std::future<std::pair<std::vector<Solution>, size_t>>> futures;
    for (uint32_t thread_idx = 0; thread_idx < options.get_num_threads(); ++thread_idx) {
      std::promise<std::pair<std::vector<Solution>, size_t>> promise;
      futures.emplace_back(promise.get_future());

      threads.emplace_back([&, thread_idx](std::promise<std::pair<std::vector<Solution>, size_t>> ret) {
        std::mt19937 thread_mt_engine(std::random_device{}());

        std::vector<Solution> thread_new_generation;
        size_t thread_num_killed = 0;

        const size_t start_idx = (thread_idx * options.get_population_size()) / options.get_num_threads();
        const size_t end_idx = ((thread_idx + 1) * options.get_population_size()) / options.get_num_threads();
        for (size_t solution_idx = start_idx; solution_idx < end_idx; ++solution_idx) {
          Solution &solution = population[solution_idx];

          auto [best_child, killed_flag]
              = create_solution_children_and_find_best(std::move(solution), best_solution, thread_mt_engine);

          thread_new_generation.emplace_back(std::move(best_child));
          if (killed_flag) {
            ++thread_num_killed;
          }
        }

        ret.set_value({std::move(thread_new_generation), thread_num_killed});
      }, std::move(promise));
    }

    for (size_t thread_idx = 0; thread_idx < threads.size(); ++thread_idx) {
      threads[thread_idx].join();
      auto [thread_new_generation, thread_num_killed] = futures[thread_idx].get();

      for (Solution &solution : thread_new_generation) {
        new_generation.emplace_back(std::move(solution));
      }
      num_killed += thread_num_killed;
    }
    threads.clear();
    futures.clear();

    population = std::move(new_generation);

    const std::vector<Solution>::const_iterator population_best_it
        = std::max_element(population.cbegin(), population.cend());
    if (*population_best_it > best_solution) {
      best_solution = *population_best_it;
      last_improvement_iteration = iteration;
    }

    progress_callback({
      .iteration = iteration,
      .best_score = best_solution.get_score(),
      .num_killed = num_killed,
      .last_improvement = last_improvement_iteration,
      .best_layout = best_solution.get_layout(),
    });
  }
  while (!stop_callback()
    && (iteration < options.get_iterations()
      || (iteration - last_improvement_iteration) < options.get_bonus_iterations()));

  return best_solution;
}

Solution Solver::create_random_solution(std::mt19937 &mt_engine) const {
  std::vector<const Probe *> inventory_copy = inventory;
  std::shuffle(inventory_copy.begin(), inventory_copy.end(), mt_engine);

  std::vector<Placement> placements;
  size_t probe_idx = 0;
  for (size_t site_idx = 0, seed_idx = 0; site_idx < FnSite::sites.size(); ++site_idx) {
    if (site_idx_is_seeded[site_idx]) {
      placements.emplace_back(merged_locked_sites_and_seed[seed_idx]);
      ++seed_idx;
    } else {
      placements.emplace_back(FnSite::sites[site_idx], *inventory_copy[probe_idx]);
      ++probe_idx;
    }
  }

  std::vector<const Probe *> unused_probes;
  for (; probe_idx < inventory_copy.size(); ++probe_idx) {
    unused_probes.push_back(inventory_copy[probe_idx]);
  }

  return Solution(
      std::move(placements),
      std::move(unused_probes),
      constrained_score_function,
      options.get_maybe_tiebreaker_function());
}

std::pair<Solution, bool> Solver::create_solution_children_and_find_best(
    Solution solution,
    const Solution &best_solution,
    std::mt19937 &mt_engine) const {
  Solution best_child = create_solution_mutation(solution, mt_engine);
  for (size_t i = 0; i < options.get_num_offspring() - 1; ++i) {
    Solution child = create_solution_mutation(solution, mt_engine);
    if (child > best_child) {
      best_child = std::move(child);
    }
  }

  bool improved = best_child > solution;
  if (!improved) {
    best_child = std::move(solution);
  }

  if (best_child.get_score() == 0) {
    best_child.get_age() += 5; // rapidly age solutions that fail constraints
  } else if (!improved && best_child < best_solution) {
    best_child.get_age() += 1; // age solutions that aren't an improvement so long as they aren't the global best
  }

  if (best_child.get_age() >= options.get_max_age()) {
    return {create_random_solution(mt_engine), true};
  } else {
    return {std::move(best_child), false};
  }
}

Solution Solver::create_solution_mutation(const Solution &solution, std::mt19937 &mt_engine) const {
  std::bernoulli_distribution should_mutate(options.get_mutation_rate());

  std::vector<Placement> new_placements = solution.get_layout().get_placements();
  std::vector<const Probe *> new_unused_probes = solution.get_unused_probes();
  bool mutated = false;
  const size_t placements_size = new_placements.size();
  const size_t inventory_size = placements_size + new_unused_probes.size();
  for (size_t i = 0; i < inventory_size; ++i) {
    const bool i_in_placements = i < placements_size;
    const Probe &probe_i = i_in_placements ? new_placements[i].get_probe() : *new_unused_probes[i - placements_size];
    if (i_in_placements
        && site_idx_is_seeded[i]
        && (options.get_force_seed() || probe_i.probe_type == Probe::Type::none)) {
      continue;
    }

    if (should_mutate(mt_engine)) {
      std::uniform_int_distribution<size_t> get_mutation_idx(0, inventory_size - 1);
      const size_t j = get_mutation_idx(mt_engine);
      const bool j_in_placements = j < placements_size;
      const Probe &probe_j = j_in_placements ? new_placements[j].get_probe() : *new_unused_probes[j - placements_size];
      if (j_in_placements
          && site_idx_is_seeded[j]
          && (options.get_force_seed() || probe_j.probe_type == Probe::Type::none)) {
        continue;
      }

      if (&probe_i == &probe_j) {
        continue;
      }

      if (i_in_placements) {
        new_placements[i] = Placement(new_placements[i].get_site(), probe_j);
      } else {
        new_unused_probes[i - placements_size] = &probe_j;
      }

      if (j_in_placements) {
        new_placements[j] = Placement(new_placements[j].get_site(), probe_i);
      } else {
        new_unused_probes[j - placements_size] = &probe_i;
      }

      mutated = true;
    }
  }

  if (mutated) {
    return Solution(
        Layout(std::move(new_placements)),
        std::move(new_unused_probes),
        constrained_score_function,
        options.get_maybe_tiebreaker_function());
  } else {
    return solution;
  }
}

