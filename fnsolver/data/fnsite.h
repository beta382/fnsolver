#ifndef FNSOLVER_DATA_FNSITE_H
#define FNSOLVER_DATA_FNSITE_H

#include <fnsolver/data/precious_resource.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct FnSite {
  using id_t = uint32_t;

  static constexpr size_t num_sites = 104;

  static const std::unordered_map<id_t, size_t> idx_for_id;
  static const std::array<FnSite, num_sites> &sites;

  static void override_territories(id_t site_id, uint32_t territories);

  const id_t site_id;
  const uint32_t production;
  const uint32_t revenue;
  uint32_t territories; // non-const to allow override
  const std::vector<size_t> neighbor_idxs;
  const std::array<uint32_t, precious_resource::count> precious_resource_quantities;

  FnSite(
      id_t site_id,
      uint32_t production,
      uint32_t revenue,
      uint32_t territories,
      std::vector<id_t> neighbor_ids,
      std::unordered_map<precious_resource::Type, uint32_t> quantity_for_precious_resource_type);

  FnSite(const FnSite &other) = delete;
  FnSite(FnSite &&other) = delete;
  FnSite &operator=(const FnSite &other) = delete;
  FnSite &operator=(FnSite &&other) = delete;
};

#endif // FNSOLVER_DATA_FNSITE_H

