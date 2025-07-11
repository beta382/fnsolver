#include <fnsolver/cli/cli_options.h>
#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/precious_resource.h>
#include <fnsolver/data/probe.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/solver/options.h>
#include <fnsolver/solver/solution.h>
#include <fnsolver/solver/solver.h>
#include <fnsolver/util/output.hpp>

#include <array>
#include <format>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace {
void output_inventory_str(const Options &options) {
  uint32_t total_probes = 0;
  std::array<std::vector<std::string>, 7> columns;
  columns.at(0).emplace_back("");
  for (size_t probe_idx = 0; probe_idx < Probe::probes.size(); ++probe_idx) {
    const Probe &probe = Probe::probes.at(probe_idx);
    const std::string probe_name = probe.name + ":";
    const uint32_t quantity = options.get_probe_quantities().at(probe_idx);

    total_probes += quantity;
    const std::string quantity_str = std::to_string(quantity);
    switch (probe.probe_type) {
    case Probe::Type::none:
      continue;
    case Probe::Type::mining:
      columns.at(1).emplace_back(std::move(probe_name));
      columns.at(2).emplace_back(std::move(quantity_str));
      break;
    case Probe::Type::research:
      columns.at(3).emplace_back(std::move(probe_name));
      columns.at(4).emplace_back(std::move(quantity_str));
      break;
    default:
      columns.at(5).emplace_back(std::move(probe_name));
      columns.at(6).emplace_back(std::move(quantity_str));
      break;
    }
  }

  std::cout << std::format("  Inventory{}: {}/{} probes",
      options.get_seed().empty() ? "" : " (excluding seed)",
      total_probes,
      FnSite::num_sites)
      << std::endl;
  util::output_columns(
      std::cout,
      columns,
      {
        util::Alignment::left,
        util::Alignment::left,
        util::Alignment::right,
        util::Alignment::left,
        util::Alignment::right,
        util::Alignment::left,
        util::Alignment::right
      },
      {4, 1, 4, 1, 4, 1});
}

void output_frontier_nav_overrides_str(const Options &options) {
  const std::map<FnSite::id_t, uint32_t> &territory_overrides = options.get_territory_overrides();
  const std::vector<Placement> &locked_sites = options.get_locked_sites();
  const std::vector<Placement> &seed = options.get_seed();

  std::cout << "  FrontierNav Overrides:";

  if (territory_overrides.empty() && locked_sites.empty() && seed.empty()) {
    std::cout << " none" << std::endl;
    return;
  } else {
    std::cout << std::endl;
  }

  std::vector<std::string> territory_override_column = {"Unexplored Territories Found:"};
  if (territory_overrides.empty()) {
    territory_override_column.emplace_back("  none");
  } else {
    for (const auto &[site_id, territories] : territory_overrides) {
      territory_override_column.emplace_back(std::format("  {}: {}/{}",
          site_id,
          territories,
          FnSite::sites.at(FnSite::idx_for_id.at(site_id)).territories));
    }
  }

  std::vector<std::string> locked_site_column = {"Locked Sites:"};
  if (locked_sites.empty()) {
    locked_site_column.emplace_back("  none");
  } else {
    for (const Placement &placement: locked_sites) {
      locked_site_column.emplace_back(std::format("  {}", placement.get_site().site_id));
    }
  }

  std::vector<std::string> seed_column = {std::format("Seed{}:", options.get_force_seed() ? " (forced)" : "")};
  if (seed.empty()) {
    seed_column.emplace_back("  none");
  } else {
    for (const Placement &placement: seed) {
      seed_column.emplace_back(std::format("  {}: {}", placement.get_site().site_id, placement.get_probe().name));
    }
  }

  util::output_columns(
      std::cout,
      std::array<std::vector<std::string>, 4>{
        std::vector<std::string>{""},
        std::move(territory_override_column),
        std::move(locked_site_column),
        std::move(seed_column)
      },
      {util::Alignment::left, util::Alignment::left, util::Alignment::left, util::Alignment::left},
      {4, 4, 4});
}

void output_constraints_str(const Options &options) {
  const std::map<precious_resource::Type, uint32_t> precious_resource_minimums = [&]() {
    std::map<precious_resource::Type, uint32_t> precious_resource_minimums;
    for (size_t precious_resource_idx = 0; precious_resource_idx < precious_resource::count; ++precious_resource_idx) {
      if (options.get_precious_resource_minimums().at(precious_resource_idx) != 0) {
        precious_resource_minimums.emplace(
            static_cast<precious_resource::Type>(precious_resource_idx),
            options.get_precious_resource_minimums().at(precious_resource_idx));
      }
    }
    return precious_resource_minimums;
  }();

  const uint32_t production = options.get_production_minimum();
  const uint32_t revenue = options.get_revenue_minimum();
  const uint32_t storage = options.get_storage_minimum();

  std::cout << "  Constraints:";

  const bool no_yield_constraints = production == 0 && revenue == 0 && storage == 0;

  if (precious_resource_minimums.empty() && no_yield_constraints) {
    std::cout << " none" << std::endl;
    return;
  } else {
    std::cout << std::endl;
  }

  std::vector<std::string> precious_resources_column = {"Precious Resources:"};
  if (precious_resource_minimums.empty()) {
    precious_resources_column.emplace_back("  none");
  } else {
    std::vector<std::string> precious_resource_names;
    std::vector<std::string> quantities;
    std::vector<std::string> percentages;
    for (const auto &[precious_resource_type, quantity] : precious_resource_minimums) {
      const uint32_t max_quantity = [=]() {
        uint32_t max_quantity = 0;
        for (const FnSite &site : FnSite::sites) {
          max_quantity += site.precious_resource_quantities.at(static_cast<size_t>(precious_resource_type));
        }
        return max_quantity;
      }();

      precious_resource_names.emplace_back(precious_resource::name_for_type.at(precious_resource_type) + ":");
      quantities.emplace_back(std::format("{:.2f}/{:.2f}",
          static_cast<double>(quantity) / 100.0,
          static_cast<double>(max_quantity) / 100.0));
      percentages.emplace_back(std::format("({:.2f}%)",
          static_cast<double>(quantity) / static_cast<double>(max_quantity) * 100.0));
    }

    util::output_columns(
        precious_resources_column,
        std::array<std::vector<std::string>, 4>{
          std::vector<std::string>{""},
          std::move(precious_resource_names),
          std::move(quantities),
          std::move(percentages)
        },
        {util::Alignment::left, util::Alignment::left, util::Alignment::left, util::Alignment::right},
        {2, 1, 1});
  }

  std::vector<std::string> yield_column = {"Yield:"};
  if (no_yield_constraints) {
    yield_column.emplace_back("  none");
  } else {
    std::vector<std::string> yield_names;
    std::vector<std::string> minimums;

    if (production != 0) {
      yield_names.emplace_back("Mining:");
      minimums.emplace_back(std::to_string(production));
    }

    if (revenue != 0) {
      yield_names.emplace_back("Revenue:");
      minimums.emplace_back(std::to_string(revenue));
    }

    if (storage != 0) {
      yield_names.emplace_back("Storage:");
      minimums.emplace_back(std::to_string(storage));
    }

    util::output_columns(
        yield_column,
        std::array<std::vector<std::string>, 3>{
          std::vector<std::string>{""},
          std::move(yield_names),
          std::move(minimums)
        },
        {util::Alignment::left, util::Alignment::left, util::Alignment::right},
        {2, 1});
  }

  util::output_columns(
      std::cout,
      std::array<std::vector<std::string>, 3>{
        std::vector<std::string>{""},
        std::move(precious_resources_column),
        std::move(yield_column)
      },
      {util::Alignment::left, util::Alignment::left, util::Alignment::left},
      {4, 4});
}

void output_options_report(const Options &options) {
  std::cout << "FnSolver prepared with the following configuration:" << std::endl;

  std::cout << std::format("  Score Function:      {}", options.get_score_function().get_details_str()) << std::endl;
  std::cout << std::format("  Tiebreaker Function: {}",
      options.get_maybe_tiebreaker_function() ? options.get_maybe_tiebreaker_function()->get_details_str() : "none")
      << std::endl;
  std::cout << std::endl;

  output_inventory_str(options);
  std::cout << std::endl;

  output_frontier_nav_overrides_str(options);
  std::cout << std::endl;

  output_constraints_str(options);
  std::cout << std::endl;

  std::cout << "  Solver Parameters:" << std::endl;
  util::output_columns(
      std::cout,
      std::array<std::vector<std::string>, 5>{
        std::vector<std::string>{""},
        std::vector<std::string>{"Iterations:", "Bonus Iterations:", "Population:", "Offspring:"},
        std::vector<std::string>{
          std::to_string(options.get_iterations()),
          std::to_string(options.get_bonus_iterations()),
          std::to_string(options.get_population_size()),
          std::to_string(options.get_num_offspring())
        },
        std::vector<std::string>{"Mutation Rate:", "Max Age:", "Threads:"},
        std::vector<std::string>{
          std::format("{:.4f}%", options.get_mutation_rate() * 100),
          std::to_string(options.get_max_age()),
          std::to_string(options.get_num_threads())
        }
      },
      {
        util::Alignment::left,
        util::Alignment::left,
        util::Alignment::right,
        util::Alignment::left,
        util::Alignment::left
      },
      {4, 1, 4, 1});
}
} // namespace

int main(int argc, char **argv) {
  std::optional<Options> maybe_options;
  try {
    maybe_options = cli_options::parse(argc, argv);
  } catch (const cli_options::ParseExit &e) {
    return e.return_code;
  }
  Options options = std::move(*maybe_options);

  output_options_report(options);
  std::cout << std::endl;
  std::cout << "Once FnSolver starts, you may press Ctrl-C (or your shell's alternative SIGINT key combo)" << std::endl;
  std::cout << "  to stop after the current iteration." << std::endl;
  std::cout << std::endl;

  if (!options.get_auto_confirm()) {
    while (true) {
      std::cout << "Run FnSolver with this configuration? [Y/n]: ";

      std::string confirm;
      std::getline(std::cin, confirm);
      if (std::cin.fail()) {
        std::cerr << "Failed getting confirmation input" << std::endl;
        return 1;
      }

      if (confirm == "y" || confirm == "Y" || confirm == "") {
        break;
      } else if (confirm == "n" || confirm == "N") {
        return 0;
      } else {
        std::cout << std::format("Invalid response (\"{}\"); ", confirm);
      }
    }
  }

  for (const auto &[site_id, territories] : options.get_territory_overrides()) {
    FnSite::override_territories(site_id, territories);
  }

  const Solver solver(std::move(options));
  const Solution solution = solver.run(std::cout);

  std::cout << std::endl;
  std::cout << "Best Layout:" << std::endl;
  solution.get_layout().output_report(std::cout, 2, true, true, true, true);

  return 0;
}
