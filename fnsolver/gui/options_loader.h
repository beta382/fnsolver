#ifndef FNSOLVER_OPTIONS_LOADER_H
#define FNSOLVER_OPTIONS_LOADER_H
#include "fnsolver/solver/options.h"

namespace options_loader {
Options default_options();

/**
 *
 * @throws std::runtime_error If the file cannot be loaded or is invalid.
 */
Options load_from_file(const std::string& filename);

void save_to_file(const std::string& filename, const Options& options);
}

#endif //FNSOLVER_OPTIONS_LOADER_H
