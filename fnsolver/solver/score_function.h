#ifndef FNSOLVER_SOLVER_SCORE_FUNCTION_H
#define FNSOLVER_SOLVER_SCORE_FUNCTION_H

#include <fnsolver/layout/layout.h>

#include <functional>
#include <string>
#include <unordered_map>

class ScoreFunction {
  public:
    using func_t = std::function<double(const Layout &)>;

    enum class Type {
      max_mining,
      max_effective_mining,
      max_revenue,
      max_storage,
      ratio,
      weights
    };

    static const std::unordered_map<std::string, Type> type_for_str;

    static ScoreFunction create_max_mining();
    static ScoreFunction create_max_effective_mining(double storage_factor);
    static ScoreFunction create_max_revenue();
    static ScoreFunction create_max_storage();
    static ScoreFunction create_ratio(double mining_factor, double revenue_factor, double storage_factor);
    static ScoreFunction create_weights(double mining_weight, double revenue_weight, double storage_weight);

    ScoreFunction(func_t score_function, std::string details_str);

    ScoreFunction(const ScoreFunction &other) = delete;
    ScoreFunction(ScoreFunction &&other) = default;
    ScoreFunction &operator=(const ScoreFunction &other) = delete;
    ScoreFunction &operator=(ScoreFunction &&other) = default;

    const std::string &get_details_str() const; // just used for info output
    double operator()(const Layout &layout) const;
  private:
    func_t score_function;
    std::string details_str;
};

#endif // FNSOLVER_SOLVER_SCORE_FUNCTION_H

