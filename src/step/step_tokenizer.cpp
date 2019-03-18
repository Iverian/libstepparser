#include "step_tokenizer.hpp"

#include <sstream>

using namespace std;

StepTokenizer::StepTokenizer(const string& str)
    : Tokenizer(nullptr, ", \n\t\r", ".'")
    , is_(str)
{
    Tokenizer::set_istream(is_);
}