#ifndef STEPPARSE_SRC_STEP_STEP_TOKENIZER_HPP_
#define STEPPARSE_SRC_STEP_STEP_TOKENIZER_HPP_

#include <tokenizer/tokenizer.hpp>

#include <sstream>

class StepTokenizer : public Tokenizer {
public:
    StepTokenizer() = default;
    // StepTokenizer(const StepTokenizer&) = default;
    StepTokenizer(StepTokenizer&&) = default;
    // StepTokenizer& operator=(const StepTokenizer&) = default;
    StepTokenizer& operator=(StepTokenizer&&) = default;
    ~StepTokenizer() = default;

    explicit StepTokenizer(const std::string& str);

private:
    std::istringstream is_;
};

#endif // STEPPARSE_SRC_STEP_STEP_TOKENIZER_HPP_
