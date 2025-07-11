#ifndef FNSOLVER_UTIL_OUTPUT_HPP
#define FNSOLVER_UTIL_OUTPUT_HPP

#include <algorithm>
#include <array>
#include <format>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace util {

enum class Alignment {
  left,
  right
};

template <size_t N>
void output_columns(
    std::ostream &out,
    const std::array<std::vector<std::string>, N> &columns,
    const std::array<Alignment, N> &alignments,
    const std::array<size_t, N - 1> &paddings) {
  const std::array<size_t, N> widths = [&]() {
    std::array<size_t, N> widths;
    for (size_t column_idx = 0; column_idx < columns.size(); ++column_idx) {
      const std::vector<std::string> &column = columns.at(column_idx);
      widths.at(column_idx) = column.empty() ? 0 : std::max_element(
          column.cbegin(),
          column.cend(),
          [](const std::string &lhs, const std::string &rhs) { return lhs.length() < rhs.length(); })
          ->length();
    }
    return widths;
  }();

  const size_t longest_column = std::max_element(
      columns.cbegin(),
      columns.cend(),
      [](const std::vector<std::string> &lhs, const std::vector<std::string> &rhs) { return lhs.size() < rhs.size(); })
      ->size();

  for (size_t row_idx = 0; row_idx < longest_column; ++row_idx) {
    for (size_t column_idx = 0; column_idx < columns.size(); ++column_idx) {
      const std::vector<std::string> &column = columns.at(column_idx);
      const size_t column_width = widths.at(column_idx);
      if (row_idx >= column.size()) {
        out << std::string(column_width, ' ');
      } else {
        switch (alignments.at(column_idx)) {
        case Alignment::left:
            out << std::format("{:<{}}", column.at(row_idx), column_width);
            break;
        case Alignment::right:
            out << std::format("{:>{}}", column.at(row_idx), column_width);
            break;
        }
      }

      out << std::string(column_idx < paddings.size() ? paddings.at(column_idx) : 0, ' ');
    }

    out << std::endl;
  }
}

template <size_t N>
void output_columns(
    std::vector<std::string> &out_vec,
    const std::array<std::vector<std::string>, N> &columns,
    const std::array<Alignment, N> &alignments,
    const std::array<size_t, N - 1> &paddings) {
  std::ostringstream out;
  output_columns(out, columns, alignments, paddings);

  const std::string out_str = std::move(out).str();
  std::string::size_type begin_pos = 0;
  std::string::size_type end_pos;
  while ((end_pos = out_str.find('\n', begin_pos)) != std::string::npos) {
    out_vec.emplace_back(out_str.substr(begin_pos, end_pos - begin_pos));
    begin_pos = end_pos + 1;
  }
}

}

#endif // FNSOLVER_UTIL_OUTPUT_HPP
