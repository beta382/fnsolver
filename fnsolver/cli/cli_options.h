#ifndef FNSOLVER_CLI_CLI_OPTIONS_H
#define FNSOLVER_CLI_CLI_OPTIONS_H

#include <fnsolver/solver/options.h>

namespace cli_options {
Options parse(int argc, char **argv);

struct ParseExit : public std::runtime_error {
  const int return_code;
  ParseExit(int return_code);
};
}

#endif // FNSOLVER_CLI_CLI_OPTIONS_H

