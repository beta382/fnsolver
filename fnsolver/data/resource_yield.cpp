#include <fnsolver/data/resource_yield.h>

#include <fnsolver/data/precious_resource.h>

#include <array>
#include <cstdint>

ResourceYield::ResourceYield(
    uint32_t production,
    uint32_t revenue,
    uint32_t storage,
    std::array<uint32_t, precious_resource::count> precious_resource_quantities)
    : production(production),
      revenue(revenue),
      storage(storage),
      precious_resource_quantities(std::move(precious_resource_quantities)) {}

uint32_t ResourceYield::get_production() const {
  return production;
}

uint32_t ResourceYield::get_revenue() const {
  return revenue;
}

uint32_t ResourceYield::get_storage() const {
  return storage;
}

const std::array<uint32_t, precious_resource::count> &ResourceYield::get_precious_resource_quantities() const {
  return precious_resource_quantities;
}

