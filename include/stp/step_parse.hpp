#ifndef STEPPARSE_INCLUDE_STP_STEP_PARSE_HPP_
#define STEPPARSE_INCLUDE_STP_STEP_PARSE_HPP_

#include "exports.hpp"

#include <gm/shell.hpp>

#include <iostream>
#include <string>
#include <vector>

STP_EXPORT std::vector<gm::Shell> step_parse(const std::string& str);
STP_EXPORT std::vector<gm::Shell> step_parse(std::istream& is);

#endif // STEPPARSE_INCLUDE_STP_STEP_PARSE_HPP_
