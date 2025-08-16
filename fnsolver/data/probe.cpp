#include <fnsolver/data/probe.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

// probe_id aligns with frontiernav.net ids for convenience
// static
const std::array<Probe, Probe::num_probes> Probe::probes = {
  Probe(0, 0, 0, 0, 0, Type::none, 0,  "X", "Locked"),  // the "locked" probe
  Probe(1, 50, 50, 0, 0, Type::basic, 0, "-", "Basic"),  // the "default" probe
  Probe(2, 100, 30, 0, 0, Type::mining, 1, "M1", "Mining G1"),
  Probe(3, 120, 30, 0, 0, Type::mining, 2, "M2", "Mining G2"),
  Probe(4, 140, 30, 0, 0, Type::mining, 3, "M3", "Mining G3"),
  Probe(5, 160, 30, 0, 0, Type::mining, 4, "M4", "Mining G4"),
  Probe(6, 180, 30, 0, 0, Type::mining, 5, "M5", "Mining G5"),
  Probe(7, 200, 30, 0, 0, Type::mining, 6, "M6", "Mining G6"),
  Probe(8, 220, 30, 0, 0, Type::mining, 7, "M7", "Mining G7"),
  Probe(9, 240, 30, 0, 0, Type::mining, 8, "M8", "Mining G8"),
  Probe(10, 270, 30, 0, 0, Type::mining, 9, "M9", "Mining G9"),
  Probe(11, 300, 30, 0, 0, Type::mining, 10, "M10", "Mining G10"),
  Probe(12, 30, 200, 0, 0, Type::research, 1, "R1", "Research G1"),
  Probe(13, 30, 250, 0, 0, Type::research, 2, "R2", "Research G2"),
  Probe(14, 30, 300, 0, 0, Type::research, 3, "R3", "Research G3"),
  Probe(15, 30, 350, 0, 0, Type::research, 4, "R4", "Research G4"),
  Probe(16, 30, 400, 0, 0, Type::research, 5, "R5", "Research G5"),
  Probe(17, 30, 450, 0, 0, Type::research, 6, "R6", "Research G6"),
  Probe(18, 10, 10, 50, 0, Type::booster, 1, "B1", "Booster G1"),
  Probe(19, 10, 10, 100, 0, Type::booster, 2, "B2", "Booster G2"),
  Probe(20, 0, 0, 0, 0, Type::duplicator, 0, "D", "Duplicator"),
  Probe(21, 10, 10, 0, 3000, Type::storage, 0, "S", "Storage"),
  Probe(22, 10, 10, 0, 0, Type::battle, 0, "C", "Combat")
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
    uint32_t probe_level,
    std::string shorthand,
    std::string name)
    : probe_id(probe_id),
      production_factor(production_factor),
      revenue_factor(revenue_factor),
      boost_bonus(boost_bonus),
      storage(storage),
      probe_type(probe_type),
      probe_level(probe_level),
      shorthand(std::move(shorthand)),
      name(std::move(name)) {}

