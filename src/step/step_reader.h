#ifndef HEXMESH_SRC_STEP_STEP_READER_H_
#define HEXMESH_SRC_STEP_STEP_READER_H_

#include <geom_model/vec.h>

#include <util/debug.h>

#include "step_tokenizer.h"

#include <string>
#include <tuple>
#include <vector>

EXCEPT(unexpected_symbol, "");

template <class... Args>
struct result_type {
    using type = decltype(std::tuple_cat(typename Args::tuple_t()...));
};

template <class T, class... Args>
struct StepReader {
    static typename result_type<T, Args...>::type exec(Tokenizer& tok)
    {
        auto first = StepReader<T>::exec(tok);
        auto second = StepReader<Args...>::exec(tok);
        return std::tuple_cat(first, second);
    }
};

template <class T>
struct StepReader<T> {
    static typename result_type<T>::type exec(Tokenizer& tok) { return std::make_tuple(typename T::value_t()); }
};

template <class T, class... Args>
typename result_type<T, Args...>::type step_read(Tokenizer& tok, size_t id = 0)
{
    try {
        return StepReader<T, Args...>::exec(tok);
    } catch (const err::unexpected_symbol& ex) {
        THROW(err::unexpected_symbol, std::string(ex.what()) + " at id = #" + std::to_string(id));
    }
}

template <class T, class... Args>
typename result_type<T, Args...>::type step_read(const std::string& str, size_t id = 0)
{
    StepTokenizer tok(str);
    return step_read<T, Args...>(tok, id);
};

struct str_ {
    using value_t = std::string;
    using tuple_t = std::tuple<value_t>;
};
struct ref_ {
    using value_t = size_t;
    using tuple_t = std::tuple<value_t>;
};
struct int_ {
    using value_t = size_t;
    using tuple_t = std::tuple<value_t>;
};
struct bool_ {
    using value_t = bool;
    using tuple_t = std::tuple<value_t>;
};
struct float_ {
    using value_t = double;
    using tuple_t = std::tuple<value_t>;
};
struct vec_ {
    using value_t = Vec;
    using tuple_t = std::tuple<value_t>;
};
template <class T>
struct list_ {
    using value_t = std::vector<typename T::value_t>;
    using tuple_t = std::tuple<value_t>;
};
template <class... Args>
struct br_ {
    using tuple_t = typename result_type<Args...>::type;
};
template <class T, class... Args>
struct i_ {
    using tuple_t = std::tuple<>;
};
template <class T>
using mat_ = list_<list_<T>>;
using rlist_ = list_<ref_>;

template <>
struct StepReader<str_> {
    static typename result_type<str_>::type exec(Tokenizer& tok) { return std::make_tuple((++tok)->to_str()); }
};

template <>
struct StepReader<ref_> {
    static typename result_type<ref_>::type exec(Tokenizer& tok)
    {
        size_t result = 0;
        if ((++tok)->to_str() == "#")
            result = (size_t)(++tok)->to_number();
        return std::make_tuple(result);
    }
};

template <>
struct StepReader<int_> {
    static typename result_type<int_>::type exec(Tokenizer& tok)
    {
        return std::make_tuple((size_t)(++tok)->to_number());
    }
};

template <>
struct StepReader<bool_> {
    static typename result_type<bool_>::type exec(Tokenizer& tok) { return std::make_tuple((++tok)->raw() == ".T."); }
};

template <>
struct StepReader<float_> {
    static typename result_type<float_>::type exec(Tokenizer& tok) { return std::make_tuple((++tok)->to_number()); }
};

template <>
struct StepReader<vec_> {
    static typename result_type<vec_>::type exec(Tokenizer& tok)
    {
        std::array<double, 3> result{};
        CHECK_IF((++tok)->raw().front() != '(', err::unexpected_symbol);
        for (size_t i = 0; i < 3; ++i)
            result[i] = (++tok)->to_number();
        CHECK_IF((++tok)->raw().front() != ')', err::unexpected_symbol);
        return std::make_tuple(Vec(result));
    }
};

template <class T>
struct StepReader<list_<T>> {
    static typename result_type<list_<T>>::type exec(Tokenizer& tok)
    {
        typename list_<T>::value_t result;
        typename T::value_t elem;

        if ((++tok)->raw().front() == '(') {
            std::tie(elem) = StepReader<T>::exec(tok);
            while (!tok.eof() && tok->raw().front() != ')') {
                result.emplace_back(elem);
                std::tie(elem) = StepReader<T>::exec(tok);
            }
            CHECK_IF(tok->raw().front() != ')', err::unexpected_symbol);
        }

        return std::make_tuple(result);
    }
};

template <class T>
struct StepReader<mat_<T>> {
    using list_t = list_<T>;
    using elem_t = typename list_<T>::value_t;
    using vec_t = typename mat_<T>::value_t;

    static typename result_type<mat_<T>>::type exec(Tokenizer& tok)
    {
        vec_t result;
        elem_t elem;

        if ((++tok)->raw().front() == '(') {
            std::tie(elem) = StepReader<list_t>::exec(tok);
            while (!tok.eof() && !elem.empty()) {
                result.emplace_back(elem);
                std::tie(elem) = StepReader<list_t>::exec(tok);
            }
            CHECK_IF(tok->raw().front() != ')', err::unexpected_symbol);
        }

        return std::make_tuple(result);
    }
};

template <class... Args>
struct StepReader<br_<Args...>> {
    static typename result_type<br_<Args...>>::type exec(Tokenizer& tok)
    {
        CHECK_IF((++tok)->raw().front() != '(', err::unexpected_symbol);
        auto result = StepReader<Args...>::exec(tok);
        CHECK_IF((++tok)->raw().front() != ')', err::unexpected_symbol);
        return result;
    }
};

template <class T, class... Args>
struct StepReader<i_<T, Args...>> {
    static typename result_type<i_<T, Args...>>::type exec(Tokenizer& tok)
    {
        StepReader<T, Args...>::exec(tok);
        return std::tuple<>();
    }
};

#endif // HEXMESH_SRC_STEP_STEP_READER_H_
