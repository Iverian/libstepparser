#include "step_loader.hpp"
#include "step_entities.hpp"
#include "step_tokenizer.hpp"

#include <iterator>
#include <sstream>

using namespace std;

StepLoader::StepLoader(istream& is)
    : is_(is)
{
    StepString str;
    string entity;

    while (is_ && readline() != "DATA")
        ;
    while (is_ && (str = readline()) != "ENDSEC") {
        str.cut();
        entity = str.entity_name();
        if (is_whitelisted(entity)) {
            data_.emplace(str.id(), str);
        }
    }
}

StepString StepLoader::readline()
{
    char c = '\0';
    string result;
    while (is_.get(c) && bool(isspace(c)))
        ;
    is_.putback(c);
    getline(is_, result, eol);
    return StepString(result);
}

const map<size_t, string>& StepLoader::data() const
{
    return data_;
}

StepString::StepString(size_t id, const string& str)
    : string(str)
    , id_(id)
{
}

StepString::StepString(const string& str)
    : StepString(0, str)
{
}

StepString& StepString::cut()
{
    const string& str = *this;
    size_t id, i = 0;
    string idstr;
    for (; !bool(isdigit(str[i])); ++i)
        ;
    for (; bool(isdigit(str[i])); ++i)
        idstr += str[i];
    istringstream(idstr) >> id;

    while (str[i++] != '=')
        ;
    for (; bool(isspace(str[i])); ++i)
        ;
    return (*this = StepString(id, str.substr(i)));
}

size_t StepString::id() const
{
    return id_;
}

string StepString::entity_name()
{
    if (id_ == 0) {
        cut();
    }
    return StepTokenizer(*this).next().get();
}
