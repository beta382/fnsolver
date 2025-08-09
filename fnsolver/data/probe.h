#ifndef FNSOLVER_DATA_PROBE_H
#define FNSOLVER_DATA_PROBE_H

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

struct Probe {
  friend bool operator==(const Probe&, const Probe&) = default;

  enum class Type {
    none,
    basic,
    mining,
    research,
    booster,
    storage,
    duplicator,
    battle
  };

  static constexpr size_t num_probes = 23;

  static const std::array<Probe, num_probes> probes;
  static const std::unordered_map<std::string, size_t> idx_for_shorthand;

  const uint32_t probe_id;
  const uint32_t production_factor;
  const uint32_t revenue_factor;
  const uint32_t boost_bonus;
  const uint32_t storage;
  const Type probe_type;
  const uint32_t probe_level;
  const std::string shorthand;
  const std::string name;

  Probe(
      uint32_t probe_id,
      uint32_t production_factor,
      uint32_t revenue_factor,
      uint32_t boost_bonus,
      uint32_t storage,
      Type probe_type,
      uint32_t probe_level,
      std::string shorthand,
      std::string name);

  Probe(const Probe &other) = delete;
  Probe(Probe &&other) = delete;
  Probe &operator=(const Probe &other) = delete;
  Probe &operator=(Probe &&other) = delete;
};

#endif // FNSOLVER_DATA_PROBE_H

