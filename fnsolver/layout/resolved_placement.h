#ifndef FNSOLVER_LAYOUT_RESOLVED_PLACEMENT_H
#define FNSOLVER_LAYOUT_RESOLVED_PLACEMENT_H

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/probe.h>
#include <fnsolver/data/resource_yield.h>

#include <cstdint>
#include <utility>
#include <vector>

class ResolvedPlacement {
  public:
    ResolvedPlacement(
        const FnSite &site,
        std::vector<const Probe *> probes,
        uint32_t chain_bonus,
        std::vector<uint32_t> outgoing_boost_factors,
        std::vector<std::pair<std::vector<uint32_t>, uint32_t>> incoming_boost_factors);

    ResolvedPlacement(const ResolvedPlacement &other) = default;
    ResolvedPlacement(ResolvedPlacement &&other) = default;
    ResolvedPlacement &operator=(const ResolvedPlacement &other) = default;
    ResolvedPlacement &operator=(ResolvedPlacement &&other) = default;

    const FnSite &get_site() const;
    const std::vector<const Probe *> &get_probes() const;
    uint32_t get_chain_bonus() const;
    const std::vector<uint32_t> &get_outgoing_boost_factors() const;
    const std::vector<std::pair<std::vector<uint32_t>, uint32_t>> &get_incoming_boost_factors() const;

    const ResourceYield &get_resource_yield() const;
  private:
    const FnSite *site;
    std::vector<const Probe *> probes;
    uint32_t chain_bonus;
    std::vector<uint32_t> outgoing_boost_factors;
    std::vector<std::pair<std::vector<uint32_t>, uint32_t>> incoming_boost_factors;

    ResourceYield resource_yield;
};

#endif // FNSOLVER_LAYOUT_RESOLVED_PLACEMENT_H

