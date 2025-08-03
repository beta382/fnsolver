#include <fnsolver/layout/layout.h>

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/precious_resource.h>
#include <fnsolver/data/probe.h>
#include <fnsolver/data/resource_yield.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/layout/resolved_placement.h>
#include <fnsolver/util/output.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <numeric>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
std::pair<std::vector<std::vector<const Probe *>>, std::vector<std::vector<uint32_t>>>
resolve_probes_and_outgoing_boost_factors(const std::vector<Placement> &placements) {
  std::vector<std::vector<const Probe *>> resolved_probes;
  std::vector<std::vector<uint32_t>> resolved_outgoing_boost_factors;
  for (const Placement &placement : placements) {
    const Probe &probe = placement.get_probe();

    std::vector<const Probe *> site_probes = {&probe};
    std::vector<uint32_t> site_outgoing_boost_factors;
    switch (probe.probe_type) {
    case Probe::Type::duplicator:
      for (const size_t neighbor_idx : placement.get_site().neighbor_idxs) {
        const Probe &neighbor_probe = placements[neighbor_idx].get_probe();
        site_probes.push_back(&neighbor_probe);
        if (neighbor_probe.probe_type == Probe::Type::booster) {
          site_outgoing_boost_factors.push_back(100 + neighbor_probe.boost_bonus);
        }
      }
      break;
    case Probe::Type::booster:
      site_outgoing_boost_factors.push_back(100 + probe.boost_bonus);
      break;
    default:
      // no-op
      break;
    }

    resolved_probes.emplace_back(std::move(site_probes));
    resolved_outgoing_boost_factors.emplace_back(std::move(site_outgoing_boost_factors));
  }

  return {std::move(resolved_probes), std::move(resolved_outgoing_boost_factors)};
}

struct ChainResolutionContext {
  size_t site_idx;
  size_t prev_site_idx;
  size_t prev_chain_idx;

  ChainResolutionContext(size_t site_idx, size_t prev_site_idx, size_t prev_chain_idx)
      : site_idx(site_idx), prev_site_idx(prev_site_idx), prev_chain_idx(prev_chain_idx) {}

  ChainResolutionContext(const ChainResolutionContext &other) = delete;
  ChainResolutionContext(ChainResolutionContext &&other) = default;
  ChainResolutionContext &operator=(const ChainResolutionContext &other) = delete;
  ChainResolutionContext &operator=(ChainResolutionContext &&other) = delete;
};

std::vector<uint32_t> resolve_chain_bonuses(const std::vector<Placement> &placements) {
  const size_t start_idx = FnSite::idx_for_id.at(111); // most central node, probably doesn't matter though
  const size_t none_idx = FnSite::sites.size();

  std::vector<ChainResolutionContext> context_stack;
  context_stack.emplace_back(start_idx, none_idx, none_idx);

  std::vector<std::vector<size_t>> chains;
  while (!context_stack.empty()) {
    const ChainResolutionContext context = std::move(context_stack.back());
    context_stack.pop_back();

    const size_t site_idx = context.site_idx;
    const size_t prev_site_idx = context.prev_site_idx;

    const Placement &placement = placements[site_idx];

    size_t chain_idx = context.prev_chain_idx;
    if (prev_site_idx == none_idx || &placement.get_probe() != &placements[prev_site_idx].get_probe()) {
      chains.emplace_back();
      chain_idx = chains.size() - 1;
    }
    chains[chain_idx].push_back(site_idx);

    for (const size_t neighbor_idx : placement.get_site().neighbor_idxs) {
      if (neighbor_idx != prev_site_idx) {
        context_stack.emplace_back(neighbor_idx, site_idx, chain_idx);
      }
    }
  }

  std::vector<uint32_t> resolved_chain_bonuses(placements.size(), 0);
  for (const std::vector<size_t> &chain : chains) {
    uint32_t chain_bonus = 0;
    const Probe &chain_probe = placements[chain[0]].get_probe();
    if (chain_probe.probe_type != Probe::Type::none && chain_probe.probe_type != Probe::Type::basic) {
      const size_t chain_len = chain.size();
      if (chain_len >= 8) {
        chain_bonus = 80;
      } else if (chain_len >= 5) {
        chain_bonus = 50;
      } else if (chain_len >= 3) {
        chain_bonus = 30;
      }
    }

    for (const size_t site_idx : chain) {
      resolved_chain_bonuses[site_idx] = chain_bonus;
    }
  }

  return resolved_chain_bonuses;
}

std::vector<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>> resolve_incoming_boost_factors(
    const std::vector<Placement> &placements,
    const std::vector<std::vector<uint32_t>> &resolved_outgoing_boost_factors,
    const std::vector<uint32_t> &resolved_chain_bonuses) {
  std::vector<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>> resolved_incoming_boost_factors;
  for (const Placement &placement : placements) {
    std::vector<std::pair<std::vector<uint32_t>, uint32_t>> site_incoming_boost_factors;
    for (const size_t neighbor_idx : placement.get_site().neighbor_idxs) {
      const std::vector<uint32_t> &neighbor_outgoing_boost_factors = resolved_outgoing_boost_factors[neighbor_idx];
      if (!neighbor_outgoing_boost_factors.empty()) {
        site_incoming_boost_factors.emplace_back(
            neighbor_outgoing_boost_factors, resolved_chain_bonuses[neighbor_idx]);
      }
    }

    resolved_incoming_boost_factors.emplace_back(std::move(site_incoming_boost_factors));
  }

  return resolved_incoming_boost_factors;
}

std::vector<ResolvedPlacement> resolve_placements(const std::vector<Placement> &placements) {
  auto [resolved_probes, resolved_outgoing_boost_factors] = resolve_probes_and_outgoing_boost_factors(placements);
  std::vector<uint32_t> resolved_chain_bonuses = resolve_chain_bonuses(placements);
  std::vector<std::vector<std::pair<std::vector<uint32_t>, uint32_t>>> resolved_incoming_boost_factors
      = resolve_incoming_boost_factors(placements, resolved_outgoing_boost_factors, resolved_chain_bonuses);

  std::vector<ResolvedPlacement> resolved_placements;
  for (size_t site_idx = 0; site_idx < placements.size(); ++site_idx) {
    resolved_placements.emplace_back(
        placements[site_idx].get_site(),
        std::move(resolved_probes[site_idx]),
        std::move(resolved_chain_bonuses[site_idx]),
        std::move(resolved_outgoing_boost_factors[site_idx]),
        std::move(resolved_incoming_boost_factors[site_idx]));
  }

  return resolved_placements;
}

ResourceYield resolve_resource_yield(const std::vector<ResolvedPlacement>& resolved_placements) {
  uint32_t production = 0;
  uint32_t revenue = 0;
  uint32_t storage = 6000;
  std::array<uint32_t, precious_resource::count> precious_resource_quantities;
  precious_resource_quantities.fill(0);
  for (const ResolvedPlacement& resolved_placement : resolved_placements) {
    const ResourceYield& site_resource_yield = resolved_placement.get_resource_yield();
    production += site_resource_yield.get_production();
    revenue += site_resource_yield.get_revenue();
    storage += site_resource_yield.get_storage();
    std::transform(
      precious_resource_quantities.cbegin(),
      precious_resource_quantities.cend(),
      site_resource_yield.get_precious_resource_quantities().cbegin(),
      precious_resource_quantities.begin(),
      [](uint32_t lhs, uint32_t rhs) { return lhs + rhs; });
  }
  return ResourceYield(production, revenue, storage, std::move(precious_resource_quantities));
}
} // namespace

// static
std::optional<Layout> Layout::from_frontier_nav_net_url(const std::string &url) {
  const std::string::size_type map_query_pos = url.find("?map=");
  if (map_query_pos == std::string::npos) {
    return {};
  }

  const std::string placements_str = url.substr(map_query_pos + 5, url.find("&", map_query_pos + 5));

  std::unordered_map<FnSite::id_t, size_t> probe_idx_for_site_id;
  std::string::size_type previous_placement_pos = 0;
  std::string::size_type placement_pos;
  do {
    placement_pos = placements_str.find("~", previous_placement_pos);
    const std::string placement_str
        = placements_str.substr(previous_placement_pos, placement_pos - previous_placement_pos);
    previous_placement_pos = placement_pos + 1;

    const std::string::size_type split_pos = placement_str.find("-");
    if (split_pos == std::string::npos) {
      continue;
    }

    try {
      const FnSite::id_t site_id = std::stoi(placement_str.substr(0, split_pos));
      const size_t probe_idx = std::stoi(placement_str.substr(split_pos + 1));
      probe_idx_for_site_id[site_id] = probe_idx;
    } catch (const std::exception &e) {
      continue;
    }

  } while (placement_pos != std::string::npos);

  std::vector<Placement> placements;
  for (const FnSite &site : FnSite::sites) {
    if (probe_idx_for_site_id.contains(site.site_id)) {
      placements.emplace_back(site, Probe::probes.at(probe_idx_for_site_id.at(site.site_id)));
    } else {
      placements.emplace_back(site, Probe::probes.at(Probe::idx_for_shorthand.at("X")));
    }
  }

  return Layout(std::move(placements));
}

Layout::Layout(std::vector<Placement> placements)
    : placements(std::move(placements)),
      resolved_placements(resolve_placements(this->placements)),
    resource_yield(resolve_resource_yield(this->resolved_placements)) {}

const std::vector<Placement> &Layout::get_placements() const {
  return placements;
}

const Probe* Layout::get_probe(const FnSite& site) const {
  auto placement = std::ranges::find_if(placements, [&site](const Placement& placement) {
    return placement.get_site().site_id == site.site_id;
  });
  return &placement->get_probe();
}

void Layout::set_probe(const FnSite& site, const Probe& probe) {
  auto placement = std::ranges::find_if(placements, [&site](const Placement& placement) {
    return placement.get_site().site_id == site.site_id;
  });
  if (placement != placements.end()) {
    *placement = Placement(site, probe);
  } else {
    placements.emplace_back(site, probe);
  }
  // TODO: How (non-)performant is this? Can things be changed in place rather than recalculating everything?
  resolved_placements = resolve_placements(placements);
  resource_yield = resolve_resource_yield(resolved_placements);
}

const std::vector<ResolvedPlacement> &Layout::get_resolved_placements() const {
  return resolved_placements;
}

const ResolvedPlacement& Layout::get_resolved_placement(const FnSite& site) const {
  auto placement = std::ranges::find_if(resolved_placements, [&site](const ResolvedPlacement& placement) {
    return placement.get_site().site_id == site.site_id;
  });
  return *placement;
}

const ResourceYield &Layout::get_resource_yield() const {
  return resource_yield;
}

std::string Layout::to_frontier_nav_net_url() const {
  std::ostringstream url(
      "https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides/Generated%20Layout?map=",
      std::ios::ate);
  bool first = true;
  for (const Placement &placement : placements) {
    if (first) {
      first = false;
    } else {
      url << '~';
    }

    url << placement.get_site().site_id << '-' << placement.get_probe().probe_id;
  }

  return std::move(url).str();
}

void Layout::output_report(
    std::ostream &out,
    uint32_t indent,
    bool output_url,
    bool output_resource_yield,
    bool output_precious_resources,
    bool output_site_details) const {
  std::string indent_str(indent, ' ');

  bool first_block = true;
  if (output_url) {
    if (!first_block) {
      out << std::endl;
    }
    first_block = false;

    out << std::format("{}frontiernav.net URL: {}", indent_str, to_frontier_nav_net_url()) << std::endl;
  }

  if (output_resource_yield) {
    if (!first_block) {
      out << std::endl;
    }
    first_block = false;

    util::output_columns(
        out,
        std::array<std::vector<std::string>, 3>{
          std::vector<std::string>{""},
          std::vector<std::string>{"Mining Total:", "Revenue Total:", "Storage Total:"},
          std::vector<std::string>{
            std::to_string(resource_yield.get_production()),
            std::to_string(resource_yield.get_revenue()),
            std::to_string(resource_yield.get_storage())
          }
        },
        {util::Alignment::left, util::Alignment::left, util::Alignment::right},
        {indent, 1});
  }

  if (output_precious_resources) {
    if (!first_block) {
      out << std::endl;
    }
    first_block = false;

    out << std::format("{}Precious Resources:", indent_str) << std::endl;

    std::vector<std::string> precious_resource_names;
    std::vector<std::string> quantities;
    std::vector<std::string> percentages;
    for (size_t precious_resource_idx = 0; precious_resource_idx < precious_resource::count; ++precious_resource_idx) {
      const uint32_t quantity = resource_yield.get_precious_resource_quantities().at(precious_resource_idx);
      const uint32_t max_quantity = [=]() {
        uint32_t max_quantity = 0;
        for (const FnSite &site : FnSite::sites) {
          max_quantity += site.precious_resource_quantities.at(precious_resource_idx);
        }
        return max_quantity;
      }();

      precious_resource_names.emplace_back(
          precious_resource::name_for_type.at(static_cast<precious_resource::Type>(precious_resource_idx)) + ":");
      quantities.emplace_back(std::format("{:.2f}/{:.2f}",
          static_cast<double>(quantity) / 100.0,
          static_cast<double>(max_quantity) / 100.0));
      percentages.emplace_back(std::format("({:.2f}%)",
          static_cast<double>(quantity) / static_cast<double>(max_quantity) * 100.0));
    }

    util::output_columns(
        out,
        std::array<std::vector<std::string>, 4>{
          std::vector<std::string>{""},
          std::move(precious_resource_names),
          std::move(quantities),
          std::move(percentages)
        },
        {util::Alignment::left, util::Alignment::left, util::Alignment::left, util::Alignment::right},
        {indent + 2, 1, 1});
  }

  if (output_site_details) {
    if (!first_block) {
      out << std::endl;
    }
    first_block = false;

    const size_t site_id_width = 5;
    const size_t probe_name_width = 1 + std::max_element(
        Probe::probes.cbegin(),
        Probe::probes.cend(),
        [](const Probe &lhs, const Probe &rhs) {
          return lhs.name.length() < rhs.name.length();
        })->name.length();
    const size_t mining_width = 8;
    const size_t revenue_width = 8;
    const size_t storage_width = 8;
    const size_t chain_factor_width = 6;
    const size_t outgoing_boost_factor_width = 9;
    const size_t incoming_boost_factor_width = 9;

    const std::function<void()> output_header = [&]() {
      std::string header_str = std::format(
          "{}| {:<{}}| {:<{}}| {:<{}}| {:<{}}| {:<{}}| {:<{}}| {:<{}}| {:<{}}|",
          indent_str,
          "Site", site_id_width,
          "Probe", probe_name_width,
          "Mining", mining_width,
          "Revenue", revenue_width,
          "Storage", storage_width,
          "Boost ->", outgoing_boost_factor_width,
          "Boost <-", incoming_boost_factor_width,
          "Chain", chain_factor_width);
      std::string separator_str = std::format(
          "{}|{:-<{}} |{:-<{}} |{:-<{}} |{:-<{}} |{:-<{}} |{:-<{}} |{:-<{}} |{:-<{}} |",
          indent_str,
          " ", site_id_width,
          " ", probe_name_width,
          " ", mining_width,
          " ", revenue_width,
          " ", storage_width,
          " ", outgoing_boost_factor_width,
          " ", incoming_boost_factor_width,
          " ", chain_factor_width);

      out << header_str << std::endl;
      out << separator_str << std::endl;
    };

    const std::array<std::string, 5> regions = {"Primordia", "Noctilum", "Oblivia", "Sylvalum", "Cauldros"};

    FnSite::id_t prev_site_id = 0;
    bool first_region = true;
    for (size_t site_idx = 0; site_idx < FnSite::sites.size(); ++site_idx) {
      const FnSite::id_t site_id = FnSite::sites.at(site_idx).site_id;
      if (site_id / 100 != prev_site_id / 100) { // different region
        if (!first_region) {
          out << std::endl;
        }
        first_region = false;

        out << std::format("{}{}:", indent_str, regions.at(site_id / 100 - 1)) << std::endl;
        output_header();
      }
      prev_site_id = site_id;

      const ResolvedPlacement &resolved_placement = resolved_placements.at(site_idx);

      const std::string outgoing_boost_factor_str = [&]() {
        if (!resolved_placement.get_outgoing_boost_factors().empty()) {
          double outgoing_boost_factor = 1.0;
          for (const uint32_t boost_factor : resolved_placement.get_outgoing_boost_factors()) {
            outgoing_boost_factor *= boost_factor / 100.0;
          }

          outgoing_boost_factor *= 1 + resolved_placement.get_chain_bonus() / 100.0;

          return std::format("{:.{}g}x", outgoing_boost_factor, outgoing_boost_factor_width - 3);
        } else {
          return std::string();
        }
      }();

      const std::string incoming_boost_factor_str = [&]() {
        if (!resolved_placement.get_incoming_boost_factors().empty()) {
          double incoming_boost_factor = 1.0;
          for (const auto &[boost_factors, boost_chain_bonus] : resolved_placement.get_incoming_boost_factors()) {
            for (const uint32_t boost_factor : boost_factors) {
              incoming_boost_factor *= boost_factor / 100.0;
            }

            incoming_boost_factor *= 1 + boost_chain_bonus / 100.0;
          }

          return std::format("{:.{}g}x", incoming_boost_factor, incoming_boost_factor_width - 3);
        } else {
          return std::string();
        }
      }();

      const std::string chain_factor_str = [&]() {
        if (resolved_placement.get_chain_bonus() != 0) {
          return std::format("{:{}g}x", 1 + resolved_placement.get_chain_bonus() / 100.0, chain_factor_width - 3);
        } else {
          return std::string();
        }
      }();

      const ResourceYield &placement_resource_yield = resolved_placement.get_resource_yield();
      const std::string row_str = std::format(
          "{}| {:<{}}| {:<{}}|{:>{}} |{:>{}} |{:>{}} |{:>{}} |{:>{}} |{:>{}} |",
          indent_str,
          site_id, site_id_width,
          resolved_placement.get_probes().at(0)->name, probe_name_width,
          placement_resource_yield.get_production(), mining_width,
          placement_resource_yield.get_revenue(), revenue_width,
          placement_resource_yield.get_storage(), storage_width,
          outgoing_boost_factor_str, outgoing_boost_factor_width,
          incoming_boost_factor_str, incoming_boost_factor_width,
          chain_factor_str, chain_factor_width);

      out << row_str << std::endl;
    }
  }
}

