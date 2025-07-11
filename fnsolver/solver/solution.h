#ifndef FNSOLVER_SOLVER_SOLUTION_H
#define FNSOLVER_SOLVER_SOLUTION_H

#include <fnsolver/data/probe.h>
#include <fnsolver/layout/layout.h>
#include <fnsolver/solver/score_function.h>

#include <compare>
#include <cstdint>
#include <optional>
#include <vector>

class Solution {
  public:
    Solution(
        Layout layout,
        std::vector<const Probe *> unused_probes,
        const ScoreFunction &score_function,
        const std::optional<ScoreFunction> &maybe_tiebreaker_function);

    Solution(const Solution &other) = default;
    Solution(Solution &&other) = default;
    Solution &operator=(const Solution &other) = default;
    Solution &operator=(Solution &&other) = default;

    const Layout &get_layout() const;
    const std::vector<const Probe *> &get_unused_probes() const;

    double get_score() const;
    double get_tiebreaker_score() const;
    uint32_t get_age() const;
    uint32_t &get_age();

    std::partial_ordering operator<=>(const Solution &other) const;
  private:
    Layout layout;
    std::vector<const Probe *> unused_probes;

    double score;
    double tiebreaker_score;
    uint32_t age;
};

#endif // FNSOLVER_SOLVER_SOLUTION_H

