#include <fnsolver/solver/options.h>

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/precious_resource.h>
#include <fnsolver/layout/layout.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/solver/score_function.h>

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

Options::Options(
    bool auto_confirm,
    ScoreFunction score_function,
    std::optional<ScoreFunction> maybe_tiebreaker_function,
    std::array<uint32_t, Probe::num_probes> probe_quantities,
    std::map<FnSite::id_t, uint32_t> territory_overrides,
    std::vector<Placement> locked_sites,
    std::vector<Placement> seed,
    bool force_seed,
    std::array<uint32_t, precious_resource::count> precious_resource_minimums,
    uint32_t production_minimum,
    uint32_t revenue_minimum,
    uint32_t storage_minimum,
    uint32_t iterations,
    uint32_t bonus_iterations,
    uint32_t population_size,
    uint32_t num_offspring,
    double mutation_rate,
    uint32_t max_age,
    uint32_t num_threads)
    : auto_confirm(auto_confirm),
      score_function(std::move(score_function)),
      maybe_tiebreaker_function(std::move(maybe_tiebreaker_function)),
      probe_quantities(std::move(probe_quantities)),
      territory_overrides(std::move(territory_overrides)),
      locked_sites(std::move(locked_sites)),
      seed(std::move(seed)),
      force_seed(force_seed),
      precious_resource_minimums(std::move(precious_resource_minimums)),
      production_minimum(production_minimum),
      revenue_minimum(revenue_minimum),
      storage_minimum(storage_minimum),
      iterations(iterations),
      bonus_iterations(bonus_iterations),
      population_size(population_size),
      num_offspring(num_offspring),
      mutation_rate(mutation_rate),
      max_age(max_age),
      num_threads(num_threads) {}

bool Options::get_auto_confirm() const {
  return auto_confirm;
}

const ScoreFunction &Options::get_score_function() const {
  return score_function;
}

const std::optional<ScoreFunction> &Options::get_maybe_tiebreaker_function() const {
  return maybe_tiebreaker_function;
}

const std::array<uint32_t, Probe::num_probes> &Options::get_probe_quantities() const {
  return probe_quantities;
}

const std::map<FnSite::id_t, uint32_t> &Options::get_territory_overrides() const {
  return territory_overrides;
}

const std::vector<Placement> &Options::get_locked_sites() const {
  return locked_sites;
}

const std::vector<Placement> &Options::get_seed() const {
  return seed;
}

bool Options::get_force_seed() const {
  return force_seed;
}

const std::array<uint32_t, precious_resource::count> &Options::get_precious_resource_minimums() const {
  return precious_resource_minimums;
}

uint32_t Options::get_production_minimum() const {
  return production_minimum;
}

uint32_t Options::get_revenue_minimum() const {
  return revenue_minimum;
}

uint32_t Options::get_storage_minimum() const {
  return storage_minimum;
}

uint32_t Options::get_iterations() const {
  return iterations;
}

uint32_t Options::get_bonus_iterations() const {
  return bonus_iterations;
}

uint32_t Options::get_population_size() const {
  return population_size;
}

uint32_t Options::get_num_offspring() const {
  return num_offspring;
}

double Options::get_mutation_rate() const {
  return mutation_rate;
}

uint32_t Options::get_max_age() const {
  return max_age;
}

uint32_t Options::get_num_threads() const {
  return num_threads;
}

