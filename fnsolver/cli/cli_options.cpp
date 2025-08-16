#include <fnsolver/cli/cli_options.h>

#include <fnsolver/cli/CLI11.hpp>
#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/precious_resource.h>
#include <fnsolver/data/probe.h>
#include <fnsolver/layout/layout.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/solver/options.h>
#include <fnsolver/solver/score_function.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {
const std::string config_file_opt_name = "config-file";
const std::string export_config_file_opt_name = "export-config-file";
const std::string confirm_opt_name = "auto-confirm";

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

const CLI::Range non_zero(1u, std::numeric_limits<uint32_t>::max(), "NONZERO");

std::string to_config_str(const std::vector<std::string> &vec) {
  std::ostringstream out;
  out << "[";
  bool first = true;
  for (const std::string &ele : vec) {
    if (!first) {
      out << ", ";
    }
    first = false;
    out << "\"" << ele << "\"";
  }
  out << "]";
  return std::move(out).str();
}

std::vector<double> parse_score_function_args(
    const std::vector<std::string> &score_function_strs,
    size_t num_expected_args) {
  if (score_function_strs.size() != num_expected_args + 1) {
    throw CLI::ArgumentMismatch(std::format("--{}: With {}, exactly {} args required but received {}",
        score_function_opt_str,
        score_function_strs.at(0),
        num_expected_args,
        score_function_strs.size() - 1));
  }

  std::vector<double> score_function_args;
  for (size_t i = 1; i < score_function_strs.size(); ++i) {
    double score_function_arg;
    try {
      score_function_arg = std::stod(score_function_strs.at(i));
    } catch (const std::exception &e) {
      throw CLI::ConversionError(std::format("--{}: Value \"{}\" cannot be converted to a floating-point number",
          score_function_opt_str,
          score_function_strs.at(i)));
    }

    if (score_function_arg < 0) {
      throw CLI::ValidationError(std::format("--{}: Arg {} not in range [0 - {}]",
          score_function_opt_str,
          score_function_arg,
          std::numeric_limits<double>::max()));
    }

    score_function_args.push_back(score_function_arg);
  }

  return score_function_args;
}

ScoreFunction parse_score_function(const std::vector<std::string> &score_function_strs) {
  const std::string &score_function_type = score_function_strs.at(0);
  if (!ScoreFunction::type_for_str.contains(score_function_type)) {
    throw CLI::ValidationError(std::format("--{}: Unknown score function name \"{}\"",
        score_function_opt_str,
        score_function_type));
  }

  std::vector<double> score_function_args;
  switch (ScoreFunction::type_for_str.at(score_function_type)) {
  case ScoreFunction::Type::max_mining:
    score_function_args = parse_score_function_args(score_function_strs, 0);
    return ScoreFunction::create_max_mining();
  case ScoreFunction::Type::max_effective_mining:
    score_function_args = parse_score_function_args(score_function_strs, 1);
    return ScoreFunction::create_max_effective_mining(score_function_args.at(0));
  case ScoreFunction::Type::max_revenue:
    score_function_args = parse_score_function_args(score_function_strs, 0);
    return ScoreFunction::create_max_revenue();
  case ScoreFunction::Type::max_storage:
    score_function_args = parse_score_function_args(score_function_strs, 0);
    return ScoreFunction::create_max_storage();
  case ScoreFunction::Type::ratio:
    score_function_args = parse_score_function_args(score_function_strs, 3);
    return ScoreFunction::create_ratio(
        score_function_args.at(0),
        score_function_args.at(1),
        score_function_args.at(2));
  case ScoreFunction::Type::weights:
    score_function_args = parse_score_function_args(score_function_strs, 3);
    return ScoreFunction::create_weights(
        score_function_args.at(0),
        score_function_args.at(1),
        score_function_args.at(2));
  default:
    throw CLI::HorribleError(std::format("--{}: Unsupported score function name \"{}\"",
        score_function_opt_str,
        score_function_type));
  }
}

std::optional<ScoreFunction> parse_tiebreaker_function(
    const std::string &tiebreaker_function_str,
    const std::string &score_function_str) {
  if (tiebreaker_function_str.empty()) {
    return {};
  }

  if (!ScoreFunction::type_for_str.contains(tiebreaker_function_str)) {
    throw CLI::ValidationError(std::format("--{}: Unknown tiebreaker name \"{}\"",
        tiebreaker_function_opt_str,
        tiebreaker_function_str));
  }

  const ScoreFunction::Type tiebreaker_function_type = ScoreFunction::type_for_str.at(tiebreaker_function_str);
  if (tiebreaker_function_type == ScoreFunction::type_for_str.at(score_function_str)) {
    throw CLI::ValidationError(std::format("--{}/--{}: Tiebreaker may not be the same as score function (\"{}\")",
        score_function_opt_str,
        tiebreaker_function_opt_str,
        score_function_str));
  }

  switch (tiebreaker_function_type) {
  case ScoreFunction::Type::max_mining:
    return ScoreFunction::create_max_mining();
  case ScoreFunction::Type::max_revenue:
    return ScoreFunction::create_max_revenue();
  case ScoreFunction::Type::max_storage:
    return ScoreFunction::create_max_storage();
  default:
    throw CLI::ValidationError(std::format("--{}: Unsupported tiebreaker function name \"{}\"",
        tiebreaker_function_opt_str,
        tiebreaker_function_str));
  }
}

std::array<uint32_t, Probe::num_probes> parse_probe_quantities(const std::vector<std::string> &probe_quantity_strs) {
  std::unordered_map<std::string, uint32_t> probe_quantity_for_shorthand;
  bool past_special = false;
  for (const std::string &probe_quantity_str : probe_quantity_strs) {
    if (!past_special) {
      if (probe_quantity_str == "all_og") {
        probe_quantity_for_shorthand = {
          {"M1", 20}, {"M2", 24}, {"M3", 7}, {"M4", 15}, {"M5", 9}, {"M6", 10}, {"M7", 4}, {"M8", 23}, {"M9", 10},
            {"M10", 4},
          {"R1", 3}, {"R2", 4}, {"R3", 2}, {"R4", 6}, {"R5", 7}, {"R6", 4},
          {"B1", 3}, {"B2", 3},
          {"D", 4},
          {"S", 11}
        };
      } else if (probe_quantity_str == "all_de") {
        probe_quantity_for_shorthand = {
          {"M1", 20}, {"M2", 24}, {"M3", 7}, {"M4", 15}, {"M5", 9}, {"M6", 10}, {"M7", 4}, {"M8", 23}, {"M9", 10},
            {"M10", 11},
          {"R1", 3}, {"R2", 4}, {"R3", 2}, {"R4", 6}, {"R5", 7}, {"R6", 12},
          {"B1", 6}, {"B2", 6},
          {"D", 10},
          {"S", 22}
        };
      } else if (probe_quantity_str == "no_mining") {
        probe_quantity_for_shorthand["M1"] = 0;
        probe_quantity_for_shorthand["M2"] = 0;
        probe_quantity_for_shorthand["M3"] = 0;
        probe_quantity_for_shorthand["M4"] = 0;
        probe_quantity_for_shorthand["M5"] = 0;
        probe_quantity_for_shorthand["M6"] = 0;
        probe_quantity_for_shorthand["M7"] = 0;
        probe_quantity_for_shorthand["M8"] = 0;
        probe_quantity_for_shorthand["M9"] = 0;
        probe_quantity_for_shorthand["M10"] = 0;
      } else if (probe_quantity_str == "no_research") {
        probe_quantity_for_shorthand["R1"] = 0;
        probe_quantity_for_shorthand["R2"] = 0;
        probe_quantity_for_shorthand["R3"] = 0;
        probe_quantity_for_shorthand["R4"] = 0;
        probe_quantity_for_shorthand["R5"] = 0;
        probe_quantity_for_shorthand["R6"] = 0;
      } else if (probe_quantity_str == "no_booster") {
        probe_quantity_for_shorthand["B1"] = 0;
        probe_quantity_for_shorthand["B2"] = 0;
      } else if (probe_quantity_str == "no_storage") {
        probe_quantity_for_shorthand["S"] = 0;
      } else if (probe_quantity_str == "no_duplicator") {
        probe_quantity_for_shorthand["D"] = 0;
      } else {
        past_special = true;
      }
    }

    if (!past_special) {
      continue;
    }

    const std::string::size_type split_pos = probe_quantity_str.find(":");
    if (split_pos == std::string::npos || split_pos == 0 || split_pos == probe_quantity_str.length() - 1) {
      throw CLI::ConversionError(std::format(
          "--{}: Value \"{}\" after end of known special values not in format \"probe_shorthand:quantity\"",
          probe_quantities_opt_str,
          probe_quantity_str));
    }

    const std::string shorthand = probe_quantity_str.substr(0, split_pos);
    if (!Probe::idx_for_shorthand.contains(shorthand)) {
      throw CLI::ValidationError(std::format("--{}: Unknown probe shorthand \"{}\" in value \"{}\"",
          probe_quantities_opt_str,
          shorthand,
          probe_quantity_str));
    }

    const Probe::Type probe_type = Probe::probes.at(Probe::idx_for_shorthand.at(shorthand)).probe_type;
    if (probe_type == Probe::Type::none || probe_type == Probe::Type::basic) {
      throw CLI::ValidationError(std::format("--{}: Probe shorthand \"{}\" may not be explicitly added to inventory",
          probe_quantities_opt_str,
          shorthand));
    }

    const std::string quantity_str = probe_quantity_str.substr(split_pos + 1);
    try {
      probe_quantity_for_shorthand[shorthand] = std::stoi(quantity_str);
    } catch(const std::exception &e) {
      throw CLI::ConversionError(std::format("--{}: Quantity \"{}\" in value \"{}\" cannot be converted to an integer",
          probe_quantities_opt_str,
          quantity_str,
          probe_quantity_str));
    }
  }

  std::array<uint32_t, Probe::num_probes> probe_quantities;
  for (size_t probe_idx = 0; probe_idx < Probe::probes.size(); ++probe_idx) {
    const std::string &shorthand = Probe::probes[probe_idx].shorthand;
    if (!probe_quantity_for_shorthand.contains(shorthand)) {
      probe_quantities[probe_idx] = 0;
      continue;
    }

    probe_quantities[probe_idx] = probe_quantity_for_shorthand.at(shorthand);
  }

  return probe_quantities;
}

std::map<FnSite::id_t, uint32_t> parse_territory_overrides(const std::vector<std::string> &territory_override_strs) {
  std::map<FnSite::id_t, uint32_t> territory_overrides;
  for (const std::string &territory_override_str : territory_override_strs) {
    const std::string::size_type split_pos = territory_override_str.find(":");
    if (split_pos == std::string::npos || split_pos == 0 || split_pos == territory_override_str.length() - 1) {
      throw CLI::ConversionError(std::format("--{}: Value \"{}\" not in format \"site_id:quantity\"",
          territory_overrides_opt_str,
          territory_override_str));
    }

    const std::string site_id_str = territory_override_str.substr(0, split_pos);
    FnSite::id_t site_id;
    try {
      site_id = std::stoi(site_id_str);
    } catch(const std::exception &e) {
      throw CLI::ConversionError(std::format(
          "--{}: FrontierNav Site \"{}\" in value \"{}\" cannot be converted to an integer",
          territory_overrides_opt_str,
          site_id_str,
          territory_override_str));
    }

    if (!FnSite::idx_for_id.contains(site_id)) {
       throw CLI::ValidationError(std::format("--{}: Unknown FrontierNav Site {} in value \"{}\"",
          territory_overrides_opt_str,
          site_id,
          territory_override_str));
    }

    if (territory_overrides.contains(site_id)) {
       throw CLI::ValidationError(std::format(
          "--{}: FrontierNav Site {} in value \"{}\" may not appear more than once (previously \"{}:{}\")",
          territory_overrides_opt_str,
          site_id,
          territory_override_str,
          site_id,
          territory_overrides.at(site_id)));
    }

    const std::string quantity_str = territory_override_str.substr(split_pos + 1);
    uint32_t quantity;
    try {
      quantity = std::stoi(quantity_str);
    } catch(const std::exception &e) {
      throw CLI::ConversionError(std::format("--{}: Quantity \"{}\" in value \"{}\" cannot be converted to an integer",
          territory_overrides_opt_str,
          quantity_str,
          territory_override_str));
    }

    if (quantity > FnSite::sites.at(FnSite::idx_for_id.at(site_id)).max_territories) {
      throw CLI::ValidationError(std::format(
          "--{}: FrontierNav Site {} can have up to {} unexplored territories, cannot override with {}",
          territory_overrides_opt_str,
          site_id,
          FnSite::sites.at(FnSite::idx_for_id.at(site_id)).max_territories,
          quantity));
    }

    territory_overrides.insert({site_id, quantity});
  }

  return territory_overrides;
}

std::vector<Placement> parse_locked_sites(const std::vector<std::string> &locked_site_strs) {
  std::vector<Placement> locked_sites;
  if (locked_site_strs.empty()) {
    return locked_sites;
  }

  std::optional<Layout> maybe_layout = Layout::from_frontier_nav_net_url(locked_site_strs.at(0));
  if (maybe_layout) {
    if (locked_site_strs.size() != 1) {
      throw CLI::ArgumentMismatch(std::format("--{}: With frontiernav.net URL, exactly 1 required but received {}",
          locked_sites_opt_str,
          locked_site_strs.size()));
    }

    locked_sites = maybe_layout->get_placements();
    std::erase_if(locked_sites, [](const Placement &placement) {
      return placement.get_probe().probe_type != Probe::Type::none;
    });

    return locked_sites;
  }

  std::set<FnSite::id_t> locked_sites_set;
  for (const std::string &locked_site_str : locked_site_strs) {
    FnSite::id_t site_id;
    try {
      site_id = std::stoi(locked_site_str);
    } catch (const std::exception &e) {
      throw CLI::ConversionError(std::format("--{}: Value \"{}\" cannot be converted to an integer",
          locked_sites_opt_str,
          locked_site_str));
    }

    if (!FnSite::idx_for_id.contains(site_id)) {
      throw CLI::ValidationError(std::format("--{}: Unknown FrontierNav Site {}", locked_sites_opt_str, site_id));
    }

    if (locked_sites_set.contains(site_id)) {
       throw CLI::ValidationError(std::format(
          "--{}: FrontierNav Site {} may not appear more than once", locked_sites_opt_str, site_id));
    }

    locked_sites_set.insert(site_id);
  }

  for (const FnSite::id_t site_id : locked_sites_set) {
    locked_sites.emplace_back(
        FnSite::sites.at(FnSite::idx_for_id.at(site_id)),
        Probe::probes.at(Probe::idx_for_shorthand.at("X")));
  }

  return locked_sites;
}

std::vector<Placement> parse_seed(const std::vector<std::string> &seed_strs) {
  std::vector<Placement> seed;
  if (seed_strs.empty()) {
    return seed;
  }

  std::optional<Layout> maybe_layout = Layout::from_frontier_nav_net_url(seed_strs.at(0));
  if (maybe_layout) {
    if (seed_strs.size() != 1) {
      throw CLI::ArgumentMismatch(std::format("--{}: With frontiernav.net URL, exactly 1 required but received {}",
          seed_opt_str,
          seed_strs.size()));
    }

    seed = maybe_layout->get_placements();
    std::erase_if(seed, [](const Placement &placement) {
      return placement.get_probe().probe_type == Probe::Type::none;
    });

    return seed;
  }

  std::map<FnSite::id_t, const Probe &> seed_map;
  for (const std::string &seed_str : seed_strs) {
    const std::string::size_type split_pos = seed_str.find(":");
    if (split_pos == std::string::npos || split_pos == 0 || split_pos == seed_str.length() - 1) {
      throw CLI::ConversionError(std::format("--{}: Value \"{}\" not in format \"site_id:probe_shorthand\"",
          seed_opt_str,
          seed_str));
    }

    const std::string site_id_str = seed_str.substr(0, split_pos);
    FnSite::id_t site_id;
    try {
      site_id = std::stoi(site_id_str);
    } catch(const std::exception &e) {
      throw CLI::ConversionError(std::format(
          "--{}: FrontierNav Site \"{}\" in value \"{}\" cannot be converted to an integer",
          seed_opt_str,
          site_id_str,
          seed_str));
    }

    if (!FnSite::idx_for_id.contains(site_id)) {
      throw CLI::ValidationError(std::format("--{}: Unknown FrontierNav Site {} in value \"{}\"",
          seed_opt_str,
          site_id,
          seed_str));
    }

    if (seed_map.contains(site_id)) {
       throw CLI::ValidationError(std::format(
          "--{}: FrontierNav Site {} in value \"{}\" may not appear more than once (previously \"{}:{}\")",
          seed_opt_str,
          site_id,
          seed_str,
          site_id,
          seed_map.at(site_id).shorthand));
    }

    const std::string shorthand = seed_str.substr(split_pos + 1);
    if (!Probe::idx_for_shorthand.contains(shorthand)) {
      throw CLI::ValidationError(std::format("--{}: Unknown probe shorthand \"{}\" in value \"{}\"",
          seed_opt_str,
          shorthand,
          seed_str));
    }

    const Probe &probe = Probe::probes.at(Probe::idx_for_shorthand.at(shorthand));
    if (probe.probe_type == Probe::Type::none) {
      throw CLI::ValidationError(std::format(
          "--{}: Probe shorthand \"{}\" may not be explicitly added to FronterNav layout seed",
          seed_opt_str,
          shorthand));
    }

    seed_map.insert({site_id, probe});
  }

  for (const auto &[site_id, probe] : seed_map) {
    seed.emplace_back(FnSite::sites.at(FnSite::idx_for_id.at(site_id)), probe);
  }

  return seed;
}

std::array<uint32_t, precious_resource::count> parse_precious_resource_minimums(
    const std::vector<std::string> &precious_resource_strs) {
  std::unordered_map<precious_resource::Type, uint32_t> minimum_for_precious_resource_type;
  std::unordered_map<precious_resource::Type, std::string> constraint_str_for_precious_resource_type; // error logging
  for (const std::string &precious_resource_str : precious_resource_strs) {
    const std::string::size_type split_pos = precious_resource_str.find(":");
    if (split_pos == std::string::npos || split_pos == 0 || split_pos == precious_resource_str.length() - 1) {
      throw CLI::ConversionError(std::format("--{}: Value \"{}\" not in format \"precious_resource_name:constraint\"",
          precious_resources_opt_str,
          precious_resource_str));
    }

    const std::string precious_resource_type_str = precious_resource_str.substr(0, split_pos);
    if (!precious_resource::type_for_str.contains(precious_resource_type_str)) {
      throw CLI::ValidationError(std::format("--{}: Unknown precious resource \"{}\" in value \"{}\"",
          precious_resources_opt_str,
          precious_resource_type_str,
          precious_resource_str));
    }

    const precious_resource::Type precious_resource_type
        = precious_resource::type_for_str.at(precious_resource_type_str);
    if (constraint_str_for_precious_resource_type.contains(precious_resource_type)) {
       throw CLI::ValidationError(std::format(
          "--{}: Precious resource {} in value \"{}\" may not appear more than once (previously \"{}:{}\")",
          precious_resources_opt_str,
          precious_resource_type_str,
          precious_resource_str,
          precious_resource_type_str,
          constraint_str_for_precious_resource_type.at(precious_resource_type)));
    }

    const uint32_t precious_resource_max = [=]() {
      uint32_t precious_resource_max = 0;
      for (const FnSite &site : FnSite::sites) {
        precious_resource_max += site.precious_resource_quantities.at(static_cast<size_t>(precious_resource_type));
      }
      return precious_resource_max;
    }();

    const std::string constraint_str = precious_resource_str.substr(split_pos + 1);
    uint32_t minimum;
    if (constraint_str == "all") {
      minimum = precious_resource_max;
    } else if (constraint_str == "any") {
      minimum = 1;
    } else if (constraint_str.ends_with("%")) {
      double percent;
      try {
        percent = std::stod(constraint_str); // ignores '%' by default
      } catch(const std::exception &e) {
        throw CLI::ConversionError(std::format(
            "--{}: Constraint \"{}\" in value \"{}\" cannot be converted to a floating-point number",
            precious_resources_opt_str,
            constraint_str,
            precious_resource_str));
      }

      if (0.0 >= percent || percent > 100.0) {
        throw CLI::ConversionError(std::format(
            "--{}: Constraint {} is outside of the bounds (0.0%, 100.0%]",
            precious_resources_opt_str,
            constraint_str));
      }

      minimum = static_cast<uint32_t>(std::ceil(static_cast<double>(precious_resource_max) * percent / 100.0));
    } else {
      try {
        minimum = std::stoi(constraint_str);
      } catch(const std::exception &e) {
        throw CLI::ConversionError(std::format(
            "--{}: Constraint \"{}\" in value \"{}\" cannot be converted to an integer",
            precious_resources_opt_str,
            constraint_str,
            precious_resource_str));
      }

      if (minimum > precious_resource_max) {
        throw CLI::ConversionError(std::format(
            "--{}: Constraint {} is greater than the maximum ({}) for resource {}",
            precious_resources_opt_str,
            minimum,
            precious_resource_max,
            precious_resource_type_str));
      }
    }

    minimum_for_precious_resource_type[precious_resource_type] = minimum;
    constraint_str_for_precious_resource_type[precious_resource_type] = constraint_str;
  }

  std::array<uint32_t, precious_resource::count> precious_resource_minimums;
  for (size_t i = 0; i < precious_resource::count; ++i) {
    const precious_resource::Type precious_resource_type = static_cast<precious_resource::Type>(i);
    if (!minimum_for_precious_resource_type.contains(precious_resource_type)) {
      precious_resource_minimums[i] = 0;
      continue;
    }

    precious_resource_minimums[i] = minimum_for_precious_resource_type.at(precious_resource_type);
  }

  return precious_resource_minimums;
}

void check_locked_sites_and_seed_overlap(
    const std::vector<Placement> &locked_sites,
    const std::vector<Placement> &seed) {
  auto locked_sites_it = locked_sites.cbegin();
  auto seed_it = seed.cbegin();
  while (locked_sites_it != locked_sites.cend() && seed_it != seed.cend()) {
    FnSite::id_t locked_site_id = locked_sites_it->get_site().site_id;
    FnSite::id_t seed_site_id = seed_it->get_site().site_id;
    if (locked_site_id < seed_site_id) {
      ++locked_sites_it;
    } else if (locked_site_id > seed_site_id) {
      ++seed_it;
    } else {
      throw CLI::ValidationError(std::format("--{}/--{}: FrontierNav Site {} may not appear in both",
          locked_sites_opt_str,
          seed_opt_str,
          locked_sites_it->get_site().site_id));
    }
  }
}

void adjust_probe_quantities_for_seed_and_fill(
    std::array<uint32_t, Probe::num_probes> &probe_quantities,
    const std::vector<Placement> &seed,
    size_t num_locked_sites) {
  const std::array<uint32_t, Probe::num_probes> orig_probe_quantities = probe_quantities;
  for (const Placement &placement : seed) {
    const Probe &probe = placement.get_probe();
    if (probe.probe_type == Probe::Type::none || probe.probe_type == Probe::Type::basic) {
      continue;
    }

    if (probe_quantities.at(probe.probe_id) <= 0) {
      throw CLI::ValidationError(std::format(
          "--{}/--{}: Seed consumed more {} probes than were available in the inventory (max {})",
          probe_quantities_opt_str,
          seed_opt_str,
          probe.name,
          orig_probe_quantities.at(probe.probe_id)));
    }

    --probe_quantities[probe.probe_id];
  }

  uint32_t num_probes = std::accumulate(probe_quantities.cbegin(), probe_quantities.cend(), 0);

  while (num_probes < FnSite::sites.size() - num_locked_sites - seed.size()) {
    ++probe_quantities[Probe::idx_for_shorthand.at("-")];
    ++num_probes;
  }
}
} // namespace

Options cli_options::parse(int argc, char **argv) {
  CLI::App app(
    "FnSolver is a tool for the video games \"Xenoblade Chronicles X\" and \"Xenoblade Chronicles X: Definitive "
    "Edition\", which generates tailored solutions to FrontierNav layouts");

  argv = app.ensure_utf8(argv);
  app.option_defaults()->always_capture_default();

  std::string export_config_filename;
  bool auto_confirm = false;

  std::vector<std::string> score_function_strs;
  std::string tiebreaker_function_str;

  std::vector<std::string> probe_quantity_strs = {"all_de"};

  std::vector<std::string> locked_site_strs;
  std::vector<std::string> territory_override_strs;
  std::vector<std::string> seed_strs;
  bool force_seed = false;

  std::vector<std::string> precious_resource_strs;
  uint32_t production_minimum = 0;
  uint32_t revenue_minimum = 0;
  uint32_t storage_minimum = 0;

  uint32_t iterations = 1000;
  uint32_t bonus_iterations = 0;
  uint32_t population_size = 100;
  uint32_t num_offspring = 200;
  double mutation_rate = 0.04;
  uint32_t max_age = 50;
  uint32_t num_threads = std::thread::hardware_concurrency();

  // OPTIONS group
  app.set_config("--" + config_file_opt_name, "",
      "Use options from the specified configuration file\n\n"
      "If an option is in the configuration file and is also passed on the command line, the option passed on the "
      "command line will take priority.");
  app.add_option("--" + export_config_file_opt_name, export_config_filename,
      "Export options (excluding this option and --" + config_file_opt_name + ") to the specified configuration "
      "file\n\n"
      "If this option is not explicitly passed, then no action is taken. If the specified configuration file already "
      "exists, it will be overwritten.")
      ->type_name("NONE OR TEXT")
      ->default_str("./config.toml")
      ->expected(0, 1);
  app.add_flag("-y,--" + confirm_opt_name, auto_confirm,
      "Automatically accepts the confirmation prompt");

  const std::string score_function_group_name = "SCORE FUNCTION";
  app.add_option("-f,--" + score_function_opt_str, score_function_strs,
      "Sets the score function\n\n"
      "Must be one of:\n"
      "- \"max_mining\": maximize Mining yield\n"
      "- \"max_effective_mining <storage_factor>\": maximize Mining yield, constrained by Storage\n"
      "- \"max_revenue\": maximize Revenue yield\n"
      "- \"max_storage\": maximize Storage\n"
      "- \"ratio <mining_factor> <revenue_factor> <storage_factor>\": maximize yields with the given ratios between "
      "them\n"
      "- \"weights <mining_weight> <revenue_weight> <storage_weight>\": maximize yieids with the given weights")
      ->group(score_function_group_name)
      ->required()
      ->option_text("TEXT <FLOAT:NONNEGATIVE...> REQUIRED");
  app.add_option("--" + tiebreaker_function_opt_str, tiebreaker_function_str,
      "Sets the tiebreaker Score Function\n\n"
      "Must be one of:\n"
      "- \"max_mining\"\n"
      "- \"max_revenue\"\n"
      "- \"max_storage\"")
      ->group(score_function_group_name);

  const std::string inventory_group_name = "INVENTORY";
  app.add_option("-i,--" + probe_quantities_opt_str, probe_quantity_strs,
      "Sets the probe inventory\n\n"
      "Arguments must be either:\n"
      "- A group name:\n"
      "-- \"all_og\": Sets the inventory to the maximum number of probes available in Xenoblade Chronicles X (without "
      "Combat probes)\n"
      "-- \"all_de\": Sets the inventory to the maximum number of probes available in Xenoblade Chronicles X: "
      "Definitive Edition (without Combat probes)\n"
      "-- \"no_mining\": Sets the quantity of all Mining probes to zero\n"
      "-- \"no_research\": Sets the quantity of all Research probes to zero\n"
      "-- \"no_booster\": Sets the quantity of all Booster probes to zero\n"
      "-- \"no_storage\": Sets the quantity of Storage probes to zero\n"
      "-- \"no_duplicator\": Sets the quantity of Duplicator probes to zero\n"
      "- A string in the format \"probe_shorthand:quantity\", where \"probe_shorthand\" is one of:\n"
      "-- \"M1\" through \"M10\": Mining probes\n"
      "-- \"R1\" through \"R6\": Research probes\n"
      "-- \"B1\" or \"B2\": Booster probes\n"
      "-- \"S\": Storage probes\n"
      "-- \"D\": Duplicator probes\n"
      "-- \"C\": Combat probes")
      ->group(inventory_group_name)
      ->option_text(std::format("TEXT... {}", to_config_str(probe_quantity_strs)));

  const std::string layout_group_name = "FRONTIERNAV OVERRIDES";
  app.add_option("--" + territory_overrides_opt_str, territory_override_strs,
      "Sets FrontierNav site \"Unexplored Territories Found Nearby\" overrides\n\n"
      "Arguments must be in the format \"site_id:quantity\"")
      ->group(layout_group_name)
      ->option_text(std::format("TEXT... {}", to_config_str(territory_override_strs)));
  app.add_option("--" + locked_sites_opt_str, locked_site_strs,
      "Sets FrontierNav sites that are are locked/undiscovered\n\n"
      "Arguments must be either:\n"
      "- A frontiernav.net URL, where any sites with \"No Probe\" are considered locked/undiscovered, and all others "
      "are ignored\n"
      "- A list of FrontierNav site IDs")
      ->group(layout_group_name)
      ->option_text(std::format("TEXT OR UINT... {}", to_config_str(locked_site_strs)));
  app.add_option("--" + seed_opt_str, seed_strs,
      "Sets the layout seed\n\n"
      "Arguments must be either:\n"
      "- A frontiernav.net URL, where any sites with a probe other than \"No Probe\" are considered part of the layout "
      "seed\n"
      "- A list of FrontierNav site ID and probe type pairs in the format \"site_id:probe_shorthand\", where "
      "\"probe_shorthand\" is one of:\n"
      "-- \"M1\" through \"M10\": Mining probes\n"
      "-- \"R1\" through \"R6\": Research probes\n"
      "-- \"B1\" or \"B2\": Booster probes\n"
      "-- \"S\": Storage probes\n"
      "-- \"D\": Duplicator probes\n"
      "-- \"C\": Combat probes\n"
      "-- \"-\": Basic probes")
      ->group(layout_group_name)
      ->option_text(std::format("TEXT... {}", to_config_str(seed_strs)));
  app.add_flag("--" + force_seed_opt_str, force_seed,
      "Forces the layout seed")
      ->group(layout_group_name);

  const std::string constraints_group_name = "CONSTRAINTS";
  app.add_option("--" + precious_resources_opt_str, precious_resource_strs,
      "Requires that a generated FrontierNav layout yield at least the specified quantity of Precious Resources (on "
      "average)\n\n"
      "Arguments must be in the format \"precious_resource_name:constraint\", where:\n"
      "- \"precious_resource_name\" is one of \"arc_sand_ore\", \"aurorite\", \"white_cometite\", \"enduron_lead\", "
      "\"everfreeze_ore\", \"foucaultium\", \"lionbone_bort\", \"infernium\", \"boiled_egg_ore\", \"marine_rutile\", "
      "\"dawnstone\", \"cimmerian_cinnabar\", \"ouroboros_crystal\", \"parhelion_platinum\", or \"bonjelium\"\n"
      "- \"constraint\" is one of:\n"
      "-- \"all\"\n"
      "-- \"any\"\n"
      "-- \"x%\" where \"x\" is a floating-point value in (0.0, 100.0] representing the required percentage of the "
      "maximum average yield\n"
      "-- \"x\", an integer value representing the required minimum absolute average yield times 100")
      ->group(constraints_group_name)
      ->option_text(std::format("TEXT... {}", to_config_str(precious_resource_strs)));
  app.add_option("--" + production_minimum_opt_str, production_minimum,
      "Requires that a generated FrontierNav layout yield at least the specified Mining\n\n"
      "Use of this option is discouraged")
      ->group(constraints_group_name);
  app.add_option("--" + revenue_minimum_opt_str, revenue_minimum,
      "Requires that a generated FrontierNav layout yield at least the specified Revenue\n\n"
      "Use of this option is discouraged")
      ->group(constraints_group_name);
  app.add_option("--" + storage_minimum_opt_str, storage_minimum,
      "Requires that a generated FrontierNav layout yield at least the specified Storage\n\n"
      "Use of this option is discouraged")
      ->group(constraints_group_name);

  const std::string solver_controls_group_name = "SOLVER ALGORITHM PARAMETERS";
  app.add_option("-n,--" + iterations_opt_str, iterations,
      "Sets the number of iterations FnSolver will run for")
      ->group(solver_controls_group_name)
      ->check(non_zero);
  app.add_option("--" + bonus_iterations_opt_str, bonus_iterations,
      "Sets the maximum number of additional iterations FnSolver will run for if an improvement to the best "
      "FrontierNav layout is found")
      ->group(solver_controls_group_name);
  app.add_option("-p,--" + population_size_opt_str, population_size,
      "Sets the size of the FronterNav layout population")
      ->group(solver_controls_group_name)
      ->check(non_zero);
  app.add_option("-o,--" + num_offspring_opt_str, num_offspring,
      "Sets the number of offspring to generate for each FronterNav layout in each iteration")
      ->group(solver_controls_group_name)
      ->check(non_zero);
  app.add_option("-m,--" + mutation_rate_opt_str, mutation_rate,
      "Sets the degree by which a FronterNav layout will mutate when generating offspring")
      ->group(solver_controls_group_name)
      ->check(CLI::Range(0.0, 1.0));
  app.add_option("-a,--" + max_age_opt_str, max_age,
      "Sets the maximum number of iterations a FrontierNav layout lineage can go without improvement before it is "
      "killed")
      ->group(solver_controls_group_name)
      ->check(non_zero);
  app.add_option("-t,--" + num_threads_opt_str, num_threads,
      "Sets the number of threads to execute FnSolver in parallel with\n\n"
      "FnSolver tries to determine the number of logical processors on your computer to use as the default. If it "
      "cannot do this, the default will be 0, and you must manually set this option. It is recommended you set this to "
      "exactly the number of logical processors on your computer.")
      ->group(solver_controls_group_name)
      ->check(non_zero);

  std::optional<ScoreFunction> score_function; // not actually optional, just don't want to make a default constructor
  std::optional<ScoreFunction> maybe_tiebreaker_function;

  std::array<uint32_t, Probe::num_probes> probe_quantities;

  std::map<FnSite::id_t, uint32_t> territory_overrides;
  std::vector<Placement> locked_sites;
  std::vector<Placement> seed;

  std::array<uint32_t, precious_resource::count> precious_resource_minimums;

  try {
    app.parse(argc, argv);

    score_function = parse_score_function(score_function_strs);
    maybe_tiebreaker_function = parse_tiebreaker_function(tiebreaker_function_str, score_function_strs.at(0));

    probe_quantities = parse_probe_quantities(probe_quantity_strs);

    territory_overrides = parse_territory_overrides(territory_override_strs);
    locked_sites = parse_locked_sites(locked_site_strs);
    seed = parse_seed(seed_strs);
    check_locked_sites_and_seed_overlap(locked_sites, seed);

    if (seed.empty() && force_seed) {
      throw CLI::ValidationError(std::format("--{0}/--{1}: --{1} is invalid without --{0}",
          seed_opt_str,
          force_seed_opt_str));
    }

    precious_resource_minimums = parse_precious_resource_minimums(precious_resource_strs);

    // only possible if std::thread::hardware_concurrency() == 0
    if (num_threads == 0) {
      throw CLI::ValidationError(std::format(
          "--{}: Could not determine the number of logical processors on your computer, you must manually set this",
          num_threads_opt_str));
    }

    adjust_probe_quantities_for_seed_and_fill(probe_quantities, seed, locked_sites.size());
  } catch (const CLI::ParseError &e) {
    throw ParseExit(app.exit(e));
  }

  if (!export_config_filename.empty()) {
    std::ofstream export_config_file(export_config_filename);

    export_config_file << "# OPTIONS" << std::endl;
    export_config_file << confirm_opt_name << " = " << (auto_confirm ? "true" : "false") << std::endl;
    export_config_file << std::endl;

    export_config_file << "# " << score_function_group_name << std::endl;
    // must be non-empty
    export_config_file << score_function_opt_str << " = " << to_config_str(score_function_strs) << std::endl;
    export_config_file << tiebreaker_function_opt_str << " = \"" << tiebreaker_function_str << "\"" << std::endl;
    export_config_file << std::endl;

    export_config_file << "# " << inventory_group_name << std::endl;
    export_config_file << (probe_quantity_strs.empty() ? "# " : "")
        << probe_quantities_opt_str << " = " << to_config_str(probe_quantity_strs) << std::endl;
    export_config_file << std::endl;

    export_config_file << "# " << layout_group_name << std::endl;
    export_config_file << (territory_override_strs.empty() ? "# " : "")
        << territory_overrides_opt_str << " = " << to_config_str(territory_override_strs) << std::endl;
    export_config_file << (locked_site_strs.empty() ? "# " : "")
        << locked_sites_opt_str << " = " << to_config_str(locked_site_strs) << std::endl;
    export_config_file << (seed_strs.empty() ? "# " : "")
        << seed_opt_str << " = " << to_config_str(seed_strs) << std::endl;
    export_config_file << force_seed_opt_str << " = " << (force_seed ? "true" : "false") << std::endl;
    export_config_file << std::endl;

    export_config_file << "# " << constraints_group_name << std::endl;
    export_config_file << (precious_resource_strs.empty() ? "# " : "")
        << precious_resources_opt_str << " = " << to_config_str(precious_resource_strs) << std::endl;
    export_config_file << production_minimum_opt_str << " = " << production_minimum << std::endl;
    export_config_file << revenue_minimum_opt_str << " = " << revenue_minimum << std::endl;
    export_config_file << storage_minimum_opt_str << " = " << storage_minimum << std::endl;
    export_config_file << std::endl;

    export_config_file << "# " << solver_controls_group_name << std::endl;
    export_config_file << iterations_opt_str << " = " << iterations << std::endl;
    export_config_file << bonus_iterations_opt_str << " = " << bonus_iterations << std::endl;
    export_config_file << population_size_opt_str << " = " << population_size << std::endl;
    export_config_file << num_offspring_opt_str << " = " << num_offspring << std::endl;
    export_config_file << mutation_rate_opt_str << " = " << mutation_rate << std::endl;
    export_config_file << max_age_opt_str << " = " << max_age << std::endl;
    export_config_file << num_threads_opt_str << " = " << num_threads << std::endl;
  }

  return Options(
      auto_confirm,
      std::move(*score_function),
      std::move(maybe_tiebreaker_function),
      std::move(probe_quantities),
      std::move(territory_overrides),
      std::move(locked_sites),
      std::move(seed),
      force_seed,
      std::move(precious_resource_minimums),
      production_minimum,
      revenue_minimum,
      storage_minimum,
      iterations,
      bonus_iterations,
      population_size,
      num_offspring,
      mutation_rate,
      max_age,
      num_threads);
}

cli_options::ParseExit::ParseExit(int return_code)
    : runtime_error("Exiting due to request from CLI parser"),
      return_code(return_code) {}

