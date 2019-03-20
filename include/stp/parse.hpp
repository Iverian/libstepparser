#ifndef STEPPARSE_INCLUDE_STP_PARSE_HPP_
#define STEPPARSE_INCLUDE_STP_PARSE_HPP_

#include "exports.hpp"

#include <gm/shell.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace stp {
STP_EXPORT std::vector<gm::Shell> parse(const std::string& str);
STP_EXPORT std::vector<gm::Shell> parse(std::istream& is);
} // namespace stp

#endif // STEPPARSE_INCLUDE_STP_PARSE_HPP_
