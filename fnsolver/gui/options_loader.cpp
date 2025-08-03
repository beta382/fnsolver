#include "options_loader.h"

#include <format>
#include <thread>
#include <toml++/toml.hpp>

Options options_loader::default_options() {
  return {
    false,
    ScoreFunction::create_max_effective_mining(2),
    ScoreFunction::create_max_mining(),
    {},
    {},
    {},
    []() {
      // Fill every site with a Basic probe.
      std::vector<Placement> placements;
      placements.reserve(FnSite::num_sites);
      for (const auto& site : FnSite::sites) {
        placements.emplace_back(site, Probe::probes.at(Probe::idx_for_shorthand.at("-")));
      }
      return placements;
    }(),
    false,
    {},
    0,
    0,
    0,
    1000,
    0,
    100,
    200,
    0.04,
    50,
    std::thread::hardware_concurrency()
  };
}

using probe_quantity_map = std::map<std::string, unsigned int>;

const probe_quantity_map probes_all_og{
  {"M1", 20}, {"M2", 24}, {"M3", 7}, {"M4", 15}, {"M5", 9}, {"M6", 10}, {"M7", 4}, {"M8", 23}, {"M9", 10}, {"M10", 4},
  {"R1", 3}, {"R2", 4}, {"R3", 2}, {"R4", 6}, {"R5", 7}, {"R6", 4},
  {"B1", 3}, {"B2", 3},
  {"D", 4},
  {"S", 11},
};
const probe_quantity_map probes_all_de{
  {"M1", 20}, {"M2", 24}, {"M3", 7}, {"M4", 15}, {"M5", 9}, {"M6", 10}, {"M7", 4}, {"M8", 23}, {"M9", 10}, {"M10", 11},
  {"R1", 3}, {"R2", 4}, {"R3", 2}, {"R4", 6}, {"R5", 7}, {"R6", 12},
  {"B1", 6}, {"B2", 6},
  {"D", 10},
  {"S", 22},
};
const probe_quantity_map probes_no_mining{
  {"M1", 0}, {"M2", 0}, {"M3", 0}, {"M4", 0}, {"M5", 0}, {"M6", 0}, {"M7", 0}, {"M8", 0}, {"M9", 0}, {"M10", 0},
};
const probe_quantity_map probes_no_research{
  {"R1", 0}, {"R2", 0}, {"R3", 0}, {"R4", 0}, {"R5", 0}, {"R6", 0},
};
const probe_quantity_map probes_no_booster{
  {"B1", 0}, {"B2", 0},
};
const probe_quantity_map probes_no_storage{
  {"S", 0},
};
const probe_quantity_map probes_no_duplicator{
  {"D", 0},
};
const std::unordered_map<std::string, const probe_quantity_map*> special_probe_lists{
  {"all_og", &probes_all_og},
  {"all_de", &probes_all_de},
  {"no_mining", &probes_no_mining},
  {"no_research", &probes_no_research},
  {"no_booster", &probes_no_booster},
  {"no_storage", &probes_no_storage},
  {"no_duplicator", &probes_no_duplicator},
};

Options options_loader::load_from_file(const std::string& filename) {
  // TODO: Implement.
  return default_options();
}

void options_loader::save_to_file(const std::string& filename, const Options& options) {
  toml::table tbl;

  // Auto-confirm
  tbl.emplace("auto-confirm", options.get_auto_confirm());

  // Score function
  {
    toml::array score_function{options.get_score_function().get_name()};
    for (const auto& arg : options.get_score_function().get_args()) {
      score_function.emplace_back(arg.second);
    }
    tbl.emplace("score-function", score_function);
  }

  // Tiebreaker
  tbl.emplace("tiebreaker", options.get_maybe_tiebreaker_function().has_value()
                              ? options.get_maybe_tiebreaker_function()->get_name()
                              : "");

  // Inventory
  {
    toml::array inventory;
    probe_quantity_map working_inventory;
    for (std::size_t probe_ix = 0; probe_ix < Probe::num_probes; ++probe_ix) {
      const auto& probe = Probe::probes.at(probe_ix);
      if (probe.probe_type == Probe::Type::basic || probe.probe_type == Probe::Type::none) {
        continue;
      }
      const auto quantity = options.get_probe_quantities().at(probe_ix);
      working_inventory.emplace(probe.shorthand, quantity);
    }
    // Check for specials
    for (const auto& [name, probe_list] : special_probe_lists) {
      bool matches = true;
      for (const auto& [probe_shorthand, quantity] : *probe_list) {
        auto it = working_inventory.find(probe_shorthand);
        if (it == working_inventory.end() || it->second != quantity) {
          matches = false;
          break;
        }
      }
      if (matches) {
        // Substitute list of probes for special name.
        inventory.emplace_back(name);
        std::erase_if(working_inventory, [probe_list](const decltype(working_inventory)::value_type& item) {
          return probe_list->contains(item.first);
        });
      }
    }
    // Put remaining probes individually in inventory list.
    for (const auto& [probe_shorthand, quantity] : working_inventory) {
      inventory.emplace_back(std::format("{}:{}", probe_shorthand, quantity));
    }
    tbl.emplace("inventory", inventory);
  }

  // TODO: Support overriding discovered territories.
  // tbl.emplace("territories", toml::array{});

  // Locked (i.e. not discovered) sites
  // TODO: Support frontiernav.net URLs here.
  {
    toml::array locked_sites;
    for (const auto& placement : options.get_locked_sites()) {
      locked_sites.emplace_back(placement.get_site().site_id);
    }
    if (!locked_sites.empty()) {
      tbl.emplace("locked-sites", locked_sites);
    }
  }

  // Seed (i.e. initial layout)
  {
    toml::array seed;
    for (const auto& placement : options.get_seed()) {
      seed.emplace_back(std::format("{}:{}", placement.get_site().site_id, placement.get_probe().shorthand));
    }
    if (!seed.empty()) {
      tbl.emplace("seed", seed);
    }
  }

  // Force seed
  tbl.emplace("force-seed", options.get_force_seed());

  // Precious resources
  {
    toml::array precious_resources;
    for (std::size_t resource_ix = 0; resource_ix < precious_resource::count; ++resource_ix) {
      const auto resource = static_cast<precious_resource::Type>(resource_ix);
      const auto minimum = options.get_precious_resource_minimums().at(resource_ix);
      if (minimum > 0) {
        precious_resources.emplace_back(std::format("{}:{}", precious_resource::str_for_type.at(resource), minimum));
      }
    }
    if (!precious_resources.empty()) {
      tbl.emplace("precious-resources", precious_resources);
    }
  }

  // Yield minimums
  tbl.emplace("min-mining", options.get_production_minimum());
  tbl.emplace("min-revenue", options.get_revenue_minimum());
  tbl.emplace("min-storage", options.get_storage_minimum());

  // Solver params
  tbl.emplace("iterations", options.get_iterations());
  tbl.emplace("bonus-iterations", options.get_bonus_iterations());
  tbl.emplace("population", options.get_population_size());
  tbl.emplace("offspring", options.get_num_offspring());
  tbl.emplace("mutation-rate", options.get_mutation_rate());
  tbl.emplace("max-age", options.get_max_age());
  tbl.emplace("threads", options.get_num_threads());

  // Write output.
  std::ofstream out(filename);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open file.");
  }
  out << tbl << "\n";
  out.close();
}
