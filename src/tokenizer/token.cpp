#include "token.h"

using namespace std;

Token::Token(const Token::Id& i)
        :id_(i)
{
    if (i==Id::NUMBER) {
        data_ = 0.;
    }
    else {
        data_ = string();
    }
}

Token::Token(const string& str)
        :id_(Id::STR)
{
    data_ = str;
}

Token::Token(double num)
        :id_(Id::NUMBER)
{
    data_ = num;
}

bool Token::empty() const { return id_==Id::NIL; }

Token& Token::clear()
{
    id_ = Id::NIL;
    data_ = string();
    return *this;
}

Token::Id Token::get_id() const { return id_; }

string Token::to_str() const
{
    string result;
    if (holds_alternative<string>(data_))
        result = get<string>(data_);
    return result;
}

double Token::to_number() const
{
    double result = 0;
    if (holds_alternative<double>(data_))
        result = get<double>(data_);
    return result;
}

string Token::raw() const
{
    string result;
    switch (id_) {
    case Id::NIL:
        break;
    case Id::STR:
        result = to_str();
        break;
    case Id::NUMBER:
        result = to_string(to_number());
        break;
    }
    return result;
}

ostream& operator<<(ostream& os, const Token& x) { return os << x.raw(); }
