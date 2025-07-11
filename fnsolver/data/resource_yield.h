#ifndef FNSOLVER_DATA_RESOURCE_YIELD_H
#define FNSOLVER_DATA_RESOURCE_YIELD_H

#include <fnsolver/data/precious_resource.h>

#include <array>
#include <cstdint>

class ResourceYield {
  public:
    ResourceYield(
        uint32_t production,
        uint32_t revenue,
        uint32_t storage,
        std::array<uint32_t, precious_resource::count> precious_resource_quantities);

    ResourceYield(const ResourceYield &other) = default;
    ResourceYield(ResourceYield &&other) = default;
    ResourceYield &operator=(const ResourceYield &other) = default;
    ResourceYield &operator=(ResourceYield &&other) = default;

    uint32_t get_production() const;
    uint32_t get_revenue() const;
    uint32_t get_storage() const;
    const std::array<uint32_t, precious_resource::count> &get_precious_resource_quantities() const;
  private:
    uint32_t production;
    uint32_t revenue;
    uint32_t storage;
    std::array<uint32_t, precious_resource::count> precious_resource_quantities;
};

#endif // FNSOLVER_DATA_RESOURCE_YIELD_H

