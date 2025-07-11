#include <fnsolver/data/probe.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

// probe_id aligns with frontiernav.net ids for convenience
// static
const std::array<Probe, Probe::num_probes> Probe::probes = {
  Probe(0, 0, 0, 0, 0, Type::none, "X", "Locked"),  // the "locked" probe
  Probe(1, 50, 50, 0, 0, Type::basic, "-", "Basic"),  // the "default" probe
  Probe(2, 100, 30, 0, 0, Type::mining, "M1", "Mining G1"),
  Probe(3, 120, 30, 0, 0, Type::mining, "M2", "Mining G2"),
  Probe(4, 140, 30, 0, 0, Type::mining, "M3", "Mining G3"),
  Probe(5, 160, 30, 0, 0, Type::mining, "M4", "Mining G4"),
  Probe(6, 180, 30, 0, 0, Type::mining, "M5", "Mining G5"),
  Probe(7, 200, 30, 0, 0, Type::mining, "M6", "Mining G6"),
  Probe(8, 220, 30, 0, 0, Type::mining, "M7", "Mining G7"),
  Probe(9, 240, 30, 0, 0, Type::mining, "M8", "Mining G8"),
  Probe(10, 270, 30, 0, 0, Type::mining, "M9", "Mining G9"),
  Probe(11, 300, 30, 0, 0, Type::mining, "M10", "Mining G10"),
  Probe(12, 30, 200, 0, 0, Type::research, "R1", "Research G1"),
  Probe(13, 30, 250, 0, 0, Type::research, "R2", "Research G2"),
  Probe(14, 30, 300, 0, 0, Type::research, "R3", "Research G3"),
  Probe(15, 30, 350, 0, 0, Type::research, "R4", "Research G4"),
  Probe(16, 30, 400, 0, 0, Type::research, "R5", "Research G5"),
  Probe(17, 30, 450, 0, 0, Type::research, "R6", "Research G6"),
  Probe(18, 10, 10, 50, 0, Type::booster, "B1", "Booster G1"),
  Probe(19, 10, 10, 100, 0, Type::booster, "B2", "Booster G2"),
  Probe(20, 0, 0, 0, 0, Type::duplicator, "D", "Duplicator"),
  Probe(21, 10, 10, 0, 3000, Type::storage, "S", "Storage"),
  Probe(22, 10, 10, 0, 0, Type::battle, "C", "Combat")
};

// static
const std::unordered_map<std::string, size_t> Probe::idx_for_shorthand = []() {
  std::unordered_map<std::string, size_t> idx_for_shorthand = {};
  for (size_t probe_idx = 0; probe_idx < probes.size(); ++probe_idx) {
    idx_for_shorthand.emplace(probes.at(probe_idx).shorthand, probe_idx);
  }
  return idx_for_shorthand;
}();

Probe::Probe(
    uint32_t probe_id,
    uint32_t production_factor,
    uint32_t revenue_factor,
    uint32_t boost_bonus,
    uint32_t storage,
    Type probe_type,
    std::string shorthand,
    std::string name)
    : probe_id(probe_id),
      production_factor(production_factor),
      revenue_factor(revenue_factor),
      boost_bonus(boost_bonus),
      storage(storage),
      probe_type(probe_type),
      shorthand(std::move(shorthand)),
      name(std::move(name)) {}

