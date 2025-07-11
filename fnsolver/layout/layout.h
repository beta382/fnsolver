#ifndef FNSOLVER_LAYOUT_LAYOUT_H
#define FNSOLVER_LAYOUT_LAYOUT_H

#include <fnsolver/data/probe.h>
#include <fnsolver/data/resource_yield.h>
#include <fnsolver/layout/placement.h>
#include <fnsolver/layout/resolved_placement.h>

#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

class Layout {
  public:
    static std::optional<Layout> from_frontier_nav_net_url(const std::string &url);

    Layout(std::vector<Placement> placements);

    Layout(const Layout &layout) = default;
    Layout(Layout &&layout) = default;
    Layout &operator=(const Layout &other) = default;
    Layout &operator=(Layout &&other) = default;

    const std::vector<Placement> &get_placements() const;
    const std::vector<ResolvedPlacement> &get_resolved_placements() const;
    const ResourceYield &get_resource_yield() const;

    std::string to_frontier_nav_net_url() const;
    void output_report(
        std::ostream &out,
        uint32_t indent,
        bool output_url,
        bool output_resource_yield,
        bool output_precious_resources,
        bool output_site_details) const;
  private:
    std::vector<Placement> placements;

    std::vector<ResolvedPlacement> resolved_placements;
    ResourceYield resource_yield;
};

#endif // FNSOLVER_LAYOUT_LAYOUT_H

