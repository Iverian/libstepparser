#ifndef HEXMESH_SRC_TOKENIZER_TOKENIZER_H_
#define HEXMESH_SRC_TOKENIZER_TOKENIZER_H_

#include "token.h"

#include <memory>
#include <sstream>
#include <string>

struct Tokenizer {
    Tokenizer(const Tokenizer& other) = default;
    Tokenizer(Tokenizer&& other) = default;
    Tokenizer& operator=(const Tokenizer& other) = default;
    Tokenizer& operator=(Tokenizer&& other) = default;

    Tokenizer();
    Tokenizer(std::istream* is, const std::string& delim, const std::string& lit);

    Tokenizer& operator++();
    const Tokenizer operator++(int);
    const Token& operator*() const;
    const Token* operator->() const;

    Tokenizer& next();
    std::string get();
    Tokenizer& set_istream(std::istream& is);

    bool operator==(const Tokenizer& other) const;
    bool operator!=(const Tokenizer& other) const;

    bool eof() const;

private:
    double get_number();
    std::string get_word();

    std::istream& is() const;
    bool is_delim(char c) const;
    bool is_lit(char c) const;
    bool is_word(char c) const;

    std::istream* is_;
    Token token_;
    std::string delimeters_;
    std::string literal_;
};

#endif //
