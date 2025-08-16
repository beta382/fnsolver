#include "options_loader.h"

#include <complex>
#include <format>
#include <toml++/toml.hpp>
#include <QThread>

Options options_loader::default_options() {
  return {
    false,
    ScoreFunction::create_max_effective_mining(2),
    ScoreFunction::create_max_storage(),
    []() {
      // Have enough basic probes for every site.
      std::array<uint32_t, Probe::num_probes> probe_quantities{};
      probe_quantities.at(Probe::idx_for_shorthand.at("-")) = FnSite::num_sites;

      return probe_quantities;
    }(),
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
    static_cast<uint32_t>(QThread::idealThreadCount())
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

const std::string confirm_opt_str = "auto-confirm";

const std::string score_function_opt_str = "score-function";
const std::string tiebreaker_function_opt_str = "tiebreaker";

const std::string probe_quantities_opt_str = "inventory";

const std::string territory_overrides_opt_str = "territories";
const std::string locked_sites_opt_str = "locked-sites";
const std::string seed_opt_str = "seed";
const std::string force_seed_opt_str = "force-seed";

const std::string precious_resources_opt_str = "precious-resources";
const std::string production_minimum_opt_str = "min-mining";
const std::string revenue_minimum_opt_str = "min-revenue";
const std::string storage_minimum_opt_str = "min-storage";

const std::string iterations_opt_str = "iterations";
const std::string bonus_iterations_opt_str = "bonus-iterations";
const std::string population_size_opt_str = "population";
const std::string num_offspring_opt_str = "offspring";
const std::string mutation_rate_opt_str = "mutation-rate";
const std::string max_age_opt_str = "max-age";
const std::string num_threads_opt_str = "threads";

/**
 * Helper to retrieve values of type @p T from a toml table.
 * @throws std::runtime_error if the node is not of type @p T.
 */
template <typename T, std::enable_if_t<toml::is_value<T> || toml::is_container<T>, bool>  = true>
const T& coerce_toml_node(const toml::node& node) {
  if (!node.is<T>()) {
    throw std::runtime_error("Incorrect type");
  }
  return node.ref<T>();
}

template <typename T,
          std::enable_if_t<std::is_integral_v<T>
                           && !std::is_same_v<T, int64_t>
                           && !std::is_same_v<T, bool>
                           , bool>  = true>
T coerce_toml_node(const toml::node& node) {
  return static_cast<T>(coerce_toml_node<int64_t>(node));
}

void merge_probe_list(probe_quantity_map& a, const probe_quantity_map& b) {
  for (const auto& [k,v] : b) {
    a.insert_or_assign(k, v);
  }
}

void load_score_function(const toml::table& tbl, Options& options) {
  const auto& score_function = coerce_toml_node<toml::array>(tbl.at(score_function_opt_str));
  if (!score_function.empty()) {
    std::vector<double> args;
    for (auto args_it = score_function.cbegin() + 1; args_it != score_function.cend(); ++args_it) {
      args.push_back(coerce_toml_node<double>(*args_it));
      if (args.back() < 0) {
        throw std::runtime_error("Invalid value for score function argument");
      }
    }
    options.set_score_function(
      ScoreFunction::from_name_and_args(coerce_toml_node<std::string>(score_function.front()), args)
    );
  }
}

void load_tiebreaker(const toml::table& tbl, Options& options) {
  const auto tiebreaker = coerce_toml_node<std::string>(tbl.at(tiebreaker_function_opt_str));
  if (tiebreaker.empty()) {
    options.set_maybe_tiebreaker_function({});
  }
  else {
    switch (ScoreFunction::type_for_str.at(tiebreaker)) {
      case ScoreFunction::Type::max_mining:
        options.set_maybe_tiebreaker_function(ScoreFunction::create_max_mining());
        break;
      case ScoreFunction::Type::max_revenue:
        options.set_maybe_tiebreaker_function(ScoreFunction::create_max_revenue());
        break;
      case ScoreFunction::Type::max_storage:
        options.set_maybe_tiebreaker_function(ScoreFunction::create_max_storage());
        break;
      default:
        throw std::runtime_error("Invalid tiebreaker");
    }
  }
}

void load_inventory(const toml::table& tbl, Options& options) {
  probe_quantity_map working_inventory;
  for (const auto& probe : Probe::probes) {
    working_inventory.emplace(probe.shorthand, 0);
  }
  for (const auto& entry : coerce_toml_node<toml::array>(tbl.at(probe_quantities_opt_str))) {
    const auto& item = coerce_toml_node<std::string>(entry);
    // Check for specials.
    const auto probe_list = special_probe_lists.find(item);
    if (probe_list != special_probe_lists.end()) {
      // Merge quantities from special list into inventory.
      merge_probe_list(working_inventory, *probe_list->second);
    }
    else {
      // Set specified probe quantity in inventory.
      const std::string::size_type split_pos = item.find(':');
      if (split_pos == std::string::npos || split_pos == 0 || split_pos == item.size() - 1) {
        throw std::runtime_error("Invalid probe description");
      }
      const auto shorthand = item.substr(0, split_pos);
      if (!Probe::idx_for_shorthand.contains(shorthand)) {
        throw std::runtime_error("Invalid probe shorthand");
      }
      const auto quantity = std::stoi(item.substr(split_pos + 1));
      if (quantity < 0) {
        throw std::runtime_error("Invalid quantity");
      }
      working_inventory.insert_or_assign(shorthand, quantity);
    }
  }
  std::array<uint32_t, Probe::num_probes> inventory{};
  for (const auto& [shorthand,quantity] : working_inventory) {
    inventory[Probe::idx_for_shorthand.at(shorthand)] = quantity;
  }
  options.set_probe_quantities(inventory);
}

void load_territory_overrides(const toml::table& tbl, Options& options) {
  const auto& territories = coerce_toml_node<toml::array>(tbl.at(territory_overrides_opt_str));
  std::map<FnSite::id_t, uint32_t> territory_overrides;
  for (const auto& override_str : territories) {
    const auto& override = coerce_toml_node<std::string>(override_str);
    const std::string::size_type split_pos = override.find(':');
    if (split_pos == std::string::npos || split_pos == 0 || split_pos == override.size() - 1) {
      throw std::runtime_error("Invalid override description");
    }
    const auto site_id = std::stoi(override.substr(0, split_pos));
    const uint32_t num_territories = std::stoi(override.substr(split_pos + 1));
    if (num_territories > FnSite::sites.at(FnSite::idx_for_id.at(site_id)).max_territories) {
      throw std::runtime_error("Override is greater than territory count");
    }
    territory_overrides.insert_or_assign(site_id, num_territories);
  }
  options.set_territory_overrides(territory_overrides);
}

void load_locked_sites(const toml::table& tbl, Options& options) {
  const auto& locked_sites = coerce_toml_node<toml::array>(tbl.at(locked_sites_opt_str));
  std::vector<Placement> placements;
  // Try fronteirnav.net URL first.
  std::string frontiernav_url;
  std::optional<Layout> maybe_layout;
  if (!locked_sites.empty()
    && locked_sites.front().is_string()
    && (maybe_layout = Layout::from_frontier_nav_net_url(coerce_toml_node<std::string>(locked_sites.front())))) {
    placements = maybe_layout->get_placements();
    std::erase_if(placements, [](const Placement& placement) {
      return placement.get_probe().probe_type != Probe::Type::none;
    });
  }
  else {
    placements.reserve(locked_sites.size());
    for (const auto& site_id : locked_sites) {
      placements.emplace_back(
        FnSite::sites.at(FnSite::idx_for_id.at(coerce_toml_node<FnSite::id_t>(site_id))),
        Probe::probes.at(Probe::idx_for_shorthand.at("X"))
      );
    }
  }
  options.set_locked_sites(placements);
}

void load_seed(const toml::table& tbl, Options& options) {
  const auto& seed = coerce_toml_node<toml::array>(tbl.at(seed_opt_str));
  std::vector<Placement> placements;
  // Try fronteirnav.net URL first.
  std::optional<Layout> maybe_layout;
  if (!seed.empty()
    && seed.front().is_string()
    && (maybe_layout = Layout::from_frontier_nav_net_url(coerce_toml_node<std::string>(seed.front())))) {
    placements = maybe_layout->get_placements();
    std::erase_if(placements, [](const Placement& placement) {
      return placement.get_probe().probe_type == Probe::Type::none;
    });
  }
  else {
    placements.reserve(seed.size());
    for (const auto& placement_str : seed) {
      const auto& placement = coerce_toml_node<std::string>(placement_str);
      const std::string::size_type split_pos = placement.find(':');
      if (split_pos == std::string::npos || split_pos == 0 || split_pos == placement.size() - 1) {
        throw std::runtime_error("Invalid placement description");
      }
      const auto site_id = std::stoi(placement.substr(0, split_pos));
      const auto shorthand = placement.substr(split_pos + 1);
      placements.emplace_back(
        FnSite::sites.at(FnSite::idx_for_id.at(site_id)),
        Probe::probes.at(Probe::idx_for_shorthand.at(shorthand))
      );
    }
  }
  options.set_seed(placements);
}

void load_precious_resources(const toml::table& tbl, Options& options) {
  const auto& precious_resources = coerce_toml_node<toml::array>(tbl.at(precious_resources_opt_str));
  std::array<uint32_t, precious_resource::count> working_minimums{};
  for (const auto& entry : precious_resources) {
    const auto& item = coerce_toml_node<std::string>(entry);
    const std::string::size_type split_pos = item.find(':');
    if (split_pos == std::string::npos || split_pos == 0 || split_pos == item.size() - 1) {
      throw std::runtime_error("Invalid precious resource description");
    }
    const auto resource_name = item.substr(0, split_pos);
    const auto resource = precious_resource::type_for_str.at(resource_name);

    const auto constraint = item.substr(split_pos + 1);
    uint32_t minimum;
    if (constraint == "all") {
      minimum = precious_resource::max_resource_quantity(resource);
    }
    else if (constraint == "any") {
      minimum = 1;
    }
    else if (constraint.ends_with('%')) {
      double percent = std::stod(constraint);
      if (0.0 >= percent || percent > 100.0) {
        throw std::runtime_error("Precious resource constraint is out of bounds.");
      }
      minimum = static_cast<uint32_t>(
        std::ceil(
          static_cast<double>(precious_resource::max_resource_quantity(resource)) * percent / 100.0)
      );
    }
    else {
      minimum = std::stoi(constraint);
      if (0 < minimum || minimum > precious_resource::max_resource_quantity(resource)) {
        throw std::runtime_error("Precious resource constraint is out of bounds.");
      }
    }
    working_minimums[static_cast<std::size_t>(resource)] = minimum;
  }
  options.set_precious_resource_minimums(working_minimums);
}

Options options_loader::load_from_file(const std::string& filename) {
  // There are many things that throw in here. Anything thrown is caught by the caller.

  const auto tbl = coerce_toml_node<toml::table>(toml::parse_file(filename));

  auto options = default_options();

  // Auto-confirm
  if (tbl.contains(confirm_opt_str)) {
    options.set_auto_confirm(coerce_toml_node<bool>(tbl.at(confirm_opt_str)));
  }

  // Score function
  if (tbl.contains(score_function_opt_str)) {
    load_score_function(tbl, options);
  }

  // Tiebreaker
  if (tbl.contains(tiebreaker_function_opt_str)) {
    load_tiebreaker(tbl, options);
  }

  // Inventory
  if (tbl.contains(probe_quantities_opt_str)) {
    load_inventory(tbl, options);
  }

  // Territories
  if (tbl.contains(territory_overrides_opt_str)) {
    load_territory_overrides(tbl, options);
  }

  // Locked (i.e. not discovered) sites
  if (tbl.contains(locked_sites_opt_str)) {
    load_locked_sites(tbl, options);
  }

  // Seed (i.e. initial layout)
  if (tbl.contains(seed_opt_str)) {
    load_seed(tbl, options);
  }

  // Force seed
  if (tbl.contains(force_seed_opt_str)) {
    options.set_force_seed(coerce_toml_node<bool>(tbl.at(force_seed_opt_str)));
  }

  // Precious resources
  if (tbl.contains(precious_resources_opt_str)) {
    load_precious_resources(tbl, options);
  }

  // Yield minimums
  if (tbl.contains(production_minimum_opt_str)) {
    options.set_production_minimum(coerce_toml_node<uint32_t>(tbl.at(production_minimum_opt_str)));
  }
  if (tbl.contains(revenue_minimum_opt_str)) {
    options.set_revenue_minimum(coerce_toml_node<uint32_t>(tbl.at(revenue_minimum_opt_str)));
  }
  if (tbl.contains(storage_minimum_opt_str)) {
    options.set_storage_minimum(coerce_toml_node<uint32_t>(tbl.at(storage_minimum_opt_str)));
  }

  // Solver params
  if (tbl.contains(iterations_opt_str)) {
    options.set_iterations(coerce_toml_node<uint32_t>(tbl.at(iterations_opt_str)));
  }
  if (tbl.contains(bonus_iterations_opt_str)) {
    options.set_bonus_iterations(coerce_toml_node<uint32_t>(tbl.at(bonus_iterations_opt_str)));
  }
  if (tbl.contains(population_size_opt_str)) {
    options.set_population_size(coerce_toml_node<uint32_t>(tbl.at(population_size_opt_str)));
  }
  if (tbl.contains(num_offspring_opt_str)) {
    options.set_num_offspring(coerce_toml_node<uint32_t>(tbl.at(num_offspring_opt_str)));
  }
  if (tbl.contains(mutation_rate_opt_str)) {
    options.set_mutation_rate(coerce_toml_node<double>(tbl.at(mutation_rate_opt_str)));
  }
  if (tbl.contains(max_age_opt_str)) {
    options.set_max_age(coerce_toml_node<uint32_t>(tbl.at(max_age_opt_str)));
  }
  if (tbl.contains(num_threads_opt_str)) {
    options.set_num_threads(coerce_toml_node<uint32_t>(tbl.at(num_threads_opt_str)));
  }

  return options;
}

void save_score_function(toml::table& tbl, const Options& options) {
  toml::array score_function{options.get_score_function().get_name()};
  for (const auto& arg : options.get_score_function().get_args()) {
    score_function.emplace_back(arg.second);
  }
  tbl.emplace(score_function_opt_str, score_function);
}

void save_inventory(toml::table& tbl, const Options& options) {
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
  tbl.emplace(probe_quantities_opt_str, inventory);
}

void save_territory_overrides(toml::table& tbl, const Options& options) {
  toml::array territories;
  for (const auto& [site_id, num_territories] : options.get_territory_overrides()) {
    territories.emplace_back(std::format("{}:{}", site_id, num_territories));
  }
  if (!territories.empty()) {
    tbl.emplace(territory_overrides_opt_str, territories);
  }
}

void save_locked_sites(toml::table& tbl, const Options& options) {
  toml::array locked_sites;
  for (const auto& placement : options.get_locked_sites()) {
    locked_sites.emplace_back(placement.get_site().site_id);
  }
  if (!locked_sites.empty()) {
    tbl.emplace(locked_sites_opt_str, locked_sites);
  }
}

void save_seed(toml::table& tbl, const Options& options) {
  toml::array seed;
  for (const auto& placement : options.get_seed()) {
    seed.emplace_back(std::format("{}:{}", placement.get_site().site_id, placement.get_probe().shorthand));
  }
  if (!seed.empty()) {
    tbl.emplace(seed_opt_str, seed);
  }
}

void save_precious_resources(toml::table& tbl, const Options& options) {
  toml::array precious_resources;
  for (std::size_t resource_ix = 0; resource_ix < precious_resource::count; ++resource_ix) {
    const auto resource = static_cast<precious_resource::Type>(resource_ix);
    const auto minimum = options.get_precious_resource_minimums().at(resource_ix);
    if (minimum > 0) {
      precious_resources.emplace_back(std::format("{}:{}", precious_resource::str_for_type.at(resource), minimum));
    }
  }
  if (!precious_resources.empty()) {
    tbl.emplace(precious_resources_opt_str, precious_resources);
  }
}

void options_loader::save_to_file(const std::string& filename, const Options& options) {
  toml::table tbl;

  // Auto-confirm
  tbl.emplace(confirm_opt_str, options.get_auto_confirm());

  // Score function
  save_score_function(tbl, options);

  // Tiebreaker
  tbl.emplace(tiebreaker_function_opt_str, options.get_maybe_tiebreaker_function().has_value()
                                             ? options.get_maybe_tiebreaker_function()->get_name()
                                             : "");

  // Inventory
  save_inventory(tbl, options);

  // Territories
  save_territory_overrides(tbl, options);

  // Locked (i.e. not discovered) sites
  save_locked_sites(tbl, options);

  // Seed (i.e. initial layout)
  save_seed(tbl, options);

  // Force seed
  tbl.emplace(force_seed_opt_str, options.get_force_seed());

  // Precious resources
  save_precious_resources(tbl, options);

  // Yield minimums
  tbl.emplace(production_minimum_opt_str, options.get_production_minimum());
  tbl.emplace(revenue_minimum_opt_str, options.get_revenue_minimum());
  tbl.emplace(storage_minimum_opt_str, options.get_storage_minimum());

  // Solver params
  tbl.emplace(iterations_opt_str, options.get_iterations());
  tbl.emplace(bonus_iterations_opt_str, options.get_bonus_iterations());
  tbl.emplace(population_size_opt_str, options.get_population_size());
  tbl.emplace(num_offspring_opt_str, options.get_num_offspring());
  tbl.emplace(mutation_rate_opt_str, options.get_mutation_rate());
  tbl.emplace(max_age_opt_str, options.get_max_age());
  tbl.emplace(num_threads_opt_str, options.get_num_threads());

  // Write output.
  std::ofstream out(filename);
  if (!out.is_open()) {
    throw std::runtime_error("Failed to open file.");
  }
  out << tbl << "\n";
  out.close();
}
