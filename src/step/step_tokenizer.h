#ifndef HEXMESH_SRC_STEP_STEP_TOKENIZER_H_
#define HEXMESH_SRC_STEP_STEP_TOKENIZER_H_

#include <tokenizer/tokenizer.h>

#include <sstream>

class StepTokenizer : public Tokenizer {
public:
    StepTokenizer() = default;
    StepTokenizer(const StepTokenizer&) = default;
    StepTokenizer(StepTokenizer&&) = default;
    StepTokenizer& operator=(const StepTokenizer&) = default;
    StepTokenizer& operator=(StepTokenizer&&) = default;
    ~StepTokenizer() = default;

    explicit StepTokenizer(const std::string& str);
private:
    std::istringstream is_;
};

#endif // HEXMESH_SRC_STEP_STEP_TOKENIZER_H_
