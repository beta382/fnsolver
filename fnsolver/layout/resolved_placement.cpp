#include <fnsolver/layout/resolved_placement.h>

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/precious_resource.h>
#include <fnsolver/data/probe.h>
#include <fnsolver/data/resource_yield.h>

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace {
ResourceYield calculate_resource_yield(
    const FnSite &site,
    const std::vector<const Probe *> &probes,
    uint32_t chain_bonus,
    const std::vector<std::pair<std::vector<uint32_t>, uint32_t>> &incoming_boost_factors) {
  uint32_t production = 0;
  uint32_t revenue = 0;
  uint32_t storage = 0;
  for (const Probe *probe : probes) {
    uint32_t probe_production;
    uint32_t probe_revenue;
    uint32_t probe_storage;

    switch (probe->probe_type) {
    case Probe::Type::duplicator:
      continue;
    case Probe::Type::none: // fall-through
    case Probe::Type::basic: // fall-through
    case Probe::Type::booster: // fall-through
    case Probe::Type::battle:
      // chain/boost not relevant
      production += site.production * probe->production_factor / 100;
      revenue += site.revenue * probe->revenue_factor / 100;
      // storage should always be 0 here
      break;
    case Probe::Type::mining:
      probe_production = site.production
          * probe->production_factor / 100
          * (100 + chain_bonus) / 100;
      for (const auto &[boost_factors, boost_chain_bonus] : incoming_boost_factors) {
        for (const uint32_t boost_factor : boost_factors) {
          probe_production = probe_production * boost_factor / 100;
        }

        probe_production = probe_production * (100 + boost_chain_bonus) / 100;
      }

      production += probe_production;
      revenue += site.revenue * probe->revenue_factor / 100;
      // storage should always be 0 here
      break;
    case Probe::Type::research:
      probe_revenue = (site.revenue + 2000 * site.territories)
          * probe->revenue_factor / 100
          * (100 + chain_bonus) / 100;
      for (const auto &[boost_factors, boost_chain_bonus] : incoming_boost_factors) {
        for (const uint32_t boost_factor : boost_factors) {
          probe_revenue = probe_revenue * boost_factor / 100;
        }

        probe_revenue = probe_revenue * (100 + boost_chain_bonus) / 100;
      }

      production += site.production * probe->production_factor / 100;
      revenue += probe_revenue;
      // storage should always be 0 here
      break;
    case Probe::Type::storage:
      probe_storage = probe->storage * (100 + chain_bonus) / 100;
      for (const auto &[boost_factors, boost_chain_bonus] : incoming_boost_factors) {
        for (const uint32_t boost_factor : boost_factors) {
          probe_storage = probe_storage * boost_factor / 100;
        }

        probe_storage = probe_storage * (100 + boost_chain_bonus) / 100;
      }

      production += site.production * probe->production_factor / 100;
      revenue += site.revenue * probe->revenue_factor / 100;
      storage += probe_storage;
      break;
    }
  }

  std::array<uint32_t, precious_resource::count> precious_resource_quantities;
  if (probes[0]->probe_type == Probe::Type::basic || probes[0]->probe_type == Probe::Type::mining) {
    precious_resource_quantities = site.precious_resource_quantities;
  } else {
    precious_resource_quantities.fill(0);
  }

  return ResourceYield(production, revenue / 2, storage, std::move(precious_resource_quantities));
}
} // namespace

ResolvedPlacement::ResolvedPlacement(
    const FnSite &site,
    std::vector<const Probe *> probes,
    uint32_t chain_bonus,
    std::vector<uint32_t> outgoing_boost_factors,
    std::vector<std::pair<std::vector<uint32_t>, uint32_t>> incoming_boost_factors)
    : site(&site),
      probes(std::move(probes)),
      chain_bonus(chain_bonus),
      outgoing_boost_factors(std::move(outgoing_boost_factors)),
      incoming_boost_factors(std::move(incoming_boost_factors)),
      resource_yield(
        calculate_resource_yield(*this->site, this->probes, this->chain_bonus, this->incoming_boost_factors)) {}

const FnSite &ResolvedPlacement::get_site() const {
  return *site;
}

const std::vector<const Probe *> &ResolvedPlacement::get_probes() const {
  return probes;
}

uint32_t ResolvedPlacement::get_chain_bonus() const {
  return chain_bonus;
}

const std::vector<uint32_t> &ResolvedPlacement::get_outgoing_boost_factors() const {
  return outgoing_boost_factors;
}

const std::vector<std::pair<std::vector<uint32_t>, uint32_t>> &ResolvedPlacement::get_incoming_boost_factors() const {
  return incoming_boost_factors;
}

const ResourceYield &ResolvedPlacement::get_resource_yield() const {
  return resource_yield;
}

