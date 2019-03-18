#ifndef STEPPARSE_SRC_STEP_STEP_LOADER_HPP_
#define STEPPARSE_SRC_STEP_STEP_LOADER_HPP_

#include <istream>
#include <map>
#include <string>
#include <vector>

class StepString : public std::string {
public:
    StepString() = default;
    StepString(const StepString&) = default;
    StepString(StepString&&) = default;
    StepString& operator=(const StepString&) = default;
    StepString& operator=(StepString&&) = default;

    explicit StepString(const std::string& str);

    StepString& cut();
    size_t id() const;
    std::string entity_name();

private:
    StepString(size_t id, const std::string& str);

    size_t id_;
};

class StepLoader {
public:
    using data_t = std::map<size_t, std::string>;

    static constexpr auto eol = ';';

    explicit StepLoader(std::istream& is);
    StepString readline();

    const data_t& data() const;

private:
    std::istream& is_;
    data_t data_;
};

#endif // STEPPARSE_SRC_STEP_STEP_LOADER_HPP_
