#ifndef HEXMESH_SRC_TOKENIZER_TOKEN_H_
#define HEXMESH_SRC_TOKENIZER_TOKEN_H_

#include <string>
#include <variant>

class Token {
public:
    enum class Id { NIL, STR, NUMBER };
    // using data_t = ; // util::union_t<std::string, double>;

    explicit Token(const Token::Id& i = Token::Id::NIL);
    explicit Token(const std::string& str);
    explicit Token(double num);

    bool empty() const;
    Token& clear();

    Token::Id get_id() const;
    std::string to_str() const;
    double to_number() const;
    std::string raw() const;

private:
    Token::Id id_;
    std::variant<std::string, double> data_;
};

std::ostream& operator<<(std::ostream& os, const Token& x);

#endif //
