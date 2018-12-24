#include "tokenizer.h"

#include <stdexcept>

using namespace std;

Tokenizer::Tokenizer()
    : is_(nullptr)
    , token_()
    , delimeters_()
    , literal_()
{
}

Tokenizer::Tokenizer(istream* is, const string& delim, const string& lit)
    : is_(is)
    , token_()
    , delimeters_(delim)
    , literal_(lit)
{
}

Tokenizer& Tokenizer::operator++()
{
    char c;
    while (!eof() && is().get(c) && is_delim(c))
        ;
    if (eof()) {
        token_ = Token();
    } else {
        is().putback(c);
        if (isdigit(c) || c == '-')
            token_ = Token(get_number());
        else if (isalpha(c) || c == '_')
            token_ = Token(get_word());
        else if (is_lit(c)) {
            auto mark = c;
            string literal = string(1, c);
            is().get(c);
            while (is().get(c) && c != mark)
                literal += c;
            literal += mark;
            token_ = Token(literal);
        } else {
            is().get(c);
            token_ = Token(string(1, c));
        }
    }
    return *this;
}

const Tokenizer Tokenizer::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

double Tokenizer::get_number()
{
    double result = 0;
    is() >> result;
    return result;
}

string Tokenizer::get_word()
{
    string result;
    char c;
    while (is().get(c) && !is_word(c))
        ;
    if (!eof())
        for (result += c; is().get(c) && is_word(c);)
            result += c;
    is().putback(c);
    return result;
}

bool Tokenizer::is_word(char c) const { return isalnum(c) || c == '_'; }

bool Tokenizer::is_delim(char c) const
{
    return delimeters_.find(c) != string::npos;
}

bool Tokenizer::is_lit(char c) const
{
    return literal_.find(c) != string::npos;
}

istream& Tokenizer::is() const
{
    if (!is_) throw runtime_error(
        "Null pointer exception: tokenizer input stream is not set");
    return *is_;
}

bool Tokenizer::eof() const
{
    auto result = !(is_ && is());
    return result;
}

const Token& Tokenizer::operator*() const { return token_; }

const Token* Tokenizer::operator->() const { return &token_; }

bool Tokenizer::operator==(const Tokenizer& other) const
{
    return (eof() && other.eof()) || (is_ == other.is_);
}

bool Tokenizer::operator!=(const Tokenizer& other) const
{
    return !(*this == other);
}

Tokenizer& Tokenizer::next() { return ++(*this); }

string Tokenizer::get() { return token_.raw(); }

Tokenizer& Tokenizer::set_istream(istream& is)
{
    is_ = &is;
    return *this;
}