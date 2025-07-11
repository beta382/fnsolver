#include <fnsolver/solver/solution.h>

#include <fnsolver/data/probe.h>
#include <fnsolver/layout/layout.h>
#include <fnsolver/solver/score_function.h>

#include <cstdint>
#include <optional>
#include <vector>

Solution::Solution(
    Layout layout,
    std::vector<const Probe *> unused_probes,
    const ScoreFunction &score_function,
    const std::optional<ScoreFunction> &maybe_tiebreaker_function)
    : layout(std::move(layout)),
      unused_probes(std::move(unused_probes)),
      score(score_function(this->layout)),
      tiebreaker_score([&, this]() {
        if (maybe_tiebreaker_function) {
          return (*maybe_tiebreaker_function)(this->layout);
        } else {
          return 0.0;
        }
      }()),
      age(0) {}

const Layout &Solution::get_layout() const {
  return layout;
}

const std::vector<const Probe *> &Solution::get_unused_probes() const {
  return unused_probes;
}

double Solution::get_score() const {
  return score;
}

double Solution::get_tiebreaker_score() const {
  return tiebreaker_score;
}

uint32_t Solution::get_age() const {
  return age;
}

uint32_t &Solution::get_age() {
  return age;
}

std::partial_ordering Solution::operator<=>(const Solution &other) const {
  const std::partial_ordering score_comp = score <=> other.score;
  return score_comp != 0 ? score_comp : tiebreaker_score <=> other.tiebreaker_score;
}

