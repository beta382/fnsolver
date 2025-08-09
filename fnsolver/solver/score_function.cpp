#include <fnsolver/solver/score_function.h>

#include <fnsolver/data/resource_yield.h>
#include <fnsolver/layout/layout.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <format>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>

// static
const std::unordered_map<std::string, ScoreFunction::Type> ScoreFunction::type_for_str = {
  {"max_mining", Type::max_mining},
  {"max_effective_mining", Type::max_effective_mining},
  {"max_revenue", Type::max_revenue},
  {"max_storage", Type::max_storage},
  {"ratio", Type::ratio},
  {"weights", Type::weights}
};

// static
ScoreFunction ScoreFunction::create_max_mining() {
  return ScoreFunction(
      [](const Layout &layout) { return layout.get_resource_yield().get_production(); },
      "max_mining");
}

// static
ScoreFunction ScoreFunction::create_max_effective_mining(double storage_factor) {
  return ScoreFunction(
      [=](const Layout &layout) {
        const ResourceYield &resource_yield = layout.get_resource_yield();
        return std::min(
            storage_factor * resource_yield.get_production(),
            static_cast<double>(resource_yield.get_storage()));
      },
      "max_effective_mining", {{"storage_factor", storage_factor}});
}

// static
ScoreFunction ScoreFunction::create_max_revenue() {
  return ScoreFunction(
      [](const Layout &layout) { return layout.get_resource_yield().get_revenue(); },
      "max_revenue");
}

// static
ScoreFunction ScoreFunction::create_max_storage() {
  return ScoreFunction(
      [](const Layout &layout) { return layout.get_resource_yield().get_storage(); },
      "max_storage");
}

// static
ScoreFunction ScoreFunction::create_ratio(double mining_factor, double revenue_factor, double storage_factor) {
  return ScoreFunction(
      [=](const Layout &layout) {
        if (mining_factor <= 0 && revenue_factor <= 0 && storage_factor <= 0) {
          return 0.0;
        }

        const ResourceYield &resource_yield = layout.get_resource_yield();
        const std::array<double, 3> factors = {mining_factor, revenue_factor, storage_factor};
        const std::array<uint32_t, 3> values = {
          resource_yield.get_production(),
          resource_yield.get_revenue(),
          resource_yield.get_storage()
        };

        double min = std::numeric_limits<double>::max();
        for (size_t i = 0; i < factors.size(); ++i) {
          const double factor = factors[i];
          if (factor <= 0) {
            continue;
          }

          const double value = values[i] / factor;
          if (value < min) {
            min = value;
          }
        }

        return min * (*std::max_element(factors.cbegin(), factors.cend()));
      },
      "ratio", {{"mining", mining_factor},{"revenue", revenue_factor}, {"storage", storage_factor}});
}

// static
ScoreFunction ScoreFunction::create_weights(double mining_weight, double revenue_weight, double storage_weight) {
  return ScoreFunction(
      [=](const Layout &layout) {
        const ResourceYield &resource_yield = layout.get_resource_yield();
        return mining_weight * resource_yield.get_production()
            + revenue_weight * resource_yield.get_revenue()
            + storage_weight * resource_yield.get_storage();
      },
      "weights", {{"mining", mining_weight},{"revenue", revenue_weight}, {"storage", storage_weight}});
}

ScoreFunction::ScoreFunction(func_t score_function, std::string name, args_t args)
    : score_function(std::move(score_function)),
      name(std::move(name)), args(std::move(args)) {}

ScoreFunction::ScoreFunction(const ScoreFunction& other) {
  *this = other;
}

ScoreFunction& ScoreFunction::operator=(const ScoreFunction& other) {
  return *this = from_name_and_args(other.name, other.args);
}

ScoreFunction::args_t::value_type::second_type find_arg(const ScoreFunction::args_t& args, const std::string& arg) {
  return std::ranges::find_if(args, [&arg](const ScoreFunction::args_t::value_type& check) {
    return check.first == arg;
  })->second;
}

ScoreFunction ScoreFunction::from_name_and_args(const std::string& name, const args_t& args) {
  switch (type_for_str.at(name)) {
    case Type::max_mining:
      return create_max_mining();
    case Type::max_effective_mining:
      return create_max_effective_mining(find_arg(args, "storage_factor"));
    case Type::max_revenue:
      return create_max_revenue();
    case Type::max_storage:
      return create_max_storage();
    case Type::ratio:
      return create_ratio(find_arg(args, "mining"), find_arg(args, "revenue"), find_arg(args, "storage"));
    case Type::weights:
      return create_weights(find_arg(args, "mining"), find_arg(args, "revenue"), find_arg(args, "storage"));
  }
  assert(false);
}

ScoreFunction ScoreFunction::from_name_and_args(const std::string& name, const std::vector<double>& args) {
  switch (type_for_str.at(name)) {
    case Type::max_mining:
      return create_max_mining();
    case Type::max_effective_mining:
      return create_max_effective_mining(args.at(0));
    case Type::max_revenue:
      return create_max_revenue();
    case Type::max_storage:
      return create_max_storage();
    case Type::ratio:
      return create_ratio(args.at(0), args.at(1), args.at(2));
    case Type::weights:
      return create_weights(args.at(0), args.at(1), args.at(2));
  }
  assert(false);
}

std::string ScoreFunction::get_details_str() const {
  if (args.empty()) {
    return std::format("{}()", name);
  }

  // Can't format ranges until C++23.
  std::stringstream args_ss;
  for (const auto &arg : args) {
    args_ss << std::format("{} = {}, ", arg.first, arg.second);
  }
  auto args_str = args_ss.str();
  // Remove trailing comma-space.
  args_str.resize(args_str.size() - 2);

  return std::format("{}({})", name, args_str);
}

double ScoreFunction::operator()(const Layout &layout) const {
  return score_function(layout);
}

