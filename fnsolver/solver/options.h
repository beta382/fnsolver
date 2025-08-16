#ifndef FNSOLVER_SOLVER_OPTIONS_H
#define FNSOLVER_SOLVER_OPTIONS_H

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/precious_resource.h>
#include <fnsolver/data/probe.h>
#include <fnsolver/layout/layout.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/solver/score_function.h>

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

class Options {
    friend bool operator==(const Options&, const Options&) = default;

public:
    Options(
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
        uint32_t num_threads);

    Options(const Options &other) = default;
    Options(Options &&other) = default;
    Options &operator=(const Options &other) = default;
    Options &operator=(Options &&other) = default;

    bool get_auto_confirm() const;
    void set_auto_confirm(bool auto_confirm);

    const ScoreFunction &get_score_function() const; // does not include constraints
    void set_score_function(ScoreFunction score_function);

    const std::optional<ScoreFunction> &get_maybe_tiebreaker_function() const;
    void set_maybe_tiebreaker_function(std::optional<ScoreFunction> maybe_tiebreaker_function);

    // ordered, corrected for seed, filled with basic probes as needed
    const std::array<uint32_t, Probe::num_probes> &get_probe_quantities() const;
    std::array<uint32_t, Probe::num_probes> &mutable_probe_quantities();
    void set_probe_quantities(std::array<uint32_t, Probe::num_probes> probe_quantities);

    const std::map<FnSite::id_t, uint32_t> &get_territory_overrides() const;
    void set_territory_overrides(std::map<FnSite::id_t, uint32_t> territory_overrides);

    const std::vector<Placement> &get_locked_sites() const; // ordered
    void set_locked_sites(std::vector<Placement> locked_sites);

    const std::vector<Placement> &get_seed() const; // ordered
    void set_seed(std::vector<Placement> seed);

    bool get_force_seed() const;
    void set_force_seed(bool force_seed);

    const std::array<uint32_t, precious_resource::count> &get_precious_resource_minimums() const; // ordered
    void set_precious_resource_minimums(std::array<uint32_t, precious_resource::count> precious_resource_minimums);

    uint32_t get_production_minimum() const;
    void set_production_minimum(uint32_t production_minimum);

    uint32_t get_revenue_minimum() const;
    void set_revenue_minimum(uint32_t revenue_minimum);

    uint32_t get_storage_minimum() const;
    void set_storage_minimum(uint32_t storage_minimum);

    uint32_t get_iterations() const;
    void set_iterations(uint32_t iterations);

    uint32_t get_bonus_iterations() const;
    void set_bonus_iterations(uint32_t bonus_iterations);

    uint32_t get_population_size() const;
    void set_population_size(uint32_t population_size);

    uint32_t get_num_offspring() const;
    void set_num_offspring(uint32_t num_offspring);

    double get_mutation_rate() const;
    void set_mutation_rate(double mutation_rate);

    uint32_t get_max_age() const;
    void set_max_age(uint32_t max_age);

    uint32_t get_num_threads() const;
    void set_num_threads(uint32_t num_threads);
  private:
    bool auto_confirm;

    ScoreFunction score_function;
    std::optional<ScoreFunction> maybe_tiebreaker_function;

    std::array<uint32_t, Probe::num_probes> probe_quantities;

    std::map<FnSite::id_t, uint32_t> territory_overrides;
    std::vector<Placement> locked_sites;
    std::vector<Placement> seed;
    bool force_seed;

    std::array<uint32_t, precious_resource::count> precious_resource_minimums;
    uint32_t production_minimum;
    uint32_t revenue_minimum;
    uint32_t storage_minimum;

    uint32_t iterations;
    uint32_t bonus_iterations;
    uint32_t population_size;
    uint32_t num_offspring;
    double mutation_rate;
    uint32_t max_age;
    uint32_t num_threads;
};

#endif // FNSOLVER_SOLVER_OPTIONS_H

