#ifndef FNSOLVER_SOLVER_SCORE_FUNCTION_H
#define FNSOLVER_SOLVER_SCORE_FUNCTION_H

#include <fnsolver/layout/layout.h>

#include <functional>
#include <string>
#include <unordered_map>

class ScoreFunction {
  friend bool operator==(const ScoreFunction& lhs, const ScoreFunction& rhs) {
    return
      lhs.name == rhs.name
      && lhs.args == rhs.args;
  }

public:
  using func_t = std::function<double(const Layout &)>;
    // For serialization.

    using args_map_t = std::unordered_map<std::string, double>;
    using args_t = std::vector<std::pair<args_map_t::key_type, args_map_t::mapped_type>>;

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
    ScoreFunction(func_t score_function, std::string name, args_t args = {});

    static ScoreFunction from_name_and_args(const std::string& name, const args_map_t& args);
    static ScoreFunction from_name_and_args(const std::string& name, const std::vector<double>& args);
    ScoreFunction(const ScoreFunction &other);
    ScoreFunction(ScoreFunction &&other) = default;
    ScoreFunction &operator=(const ScoreFunction &other);
    ScoreFunction &operator=(ScoreFunction &&other) = default;

    const std::string &get_name() const { return name; }
    const args_t &get_args() const { return args; }
    args_map_t get_args_map() const;
    std::string get_details_str() const; // just used for info output
    double operator()(const Layout &layout) const;
  private:
    func_t score_function;
    // For serialization.
    std::string name;
    // For serialization.
    args_t args;
};

#endif // FNSOLVER_SOLVER_SCORE_FUNCTION_H

