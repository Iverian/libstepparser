#include <stp/parse.hpp>

#include "step_loader.hpp"
#include "step_parser.hpp"

#include <fstream>

namespace stp {

std::vector<gm::Shell> parse(std::istream& is)
{
    StepLoader load(is);
    StepParser parse(load);
    return parse.parse().geom();
}

std::vector<gm::Shell> parse(const std::string& str)
{
    std::fstream is(str, std::ios_base::in);
    return parse(is);
}

} // namespace stp