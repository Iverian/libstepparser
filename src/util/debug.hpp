#ifndef STEPPARSE_SRC_UTIL_DEBUG_HPP_
#define STEPPARSE_SRC_UTIL_DEBUG_HPP_

#include <exception>
#include <iostream>

#include <fmt/ostream.h>

#ifdef _MSC_VER
#define _WHAT_MODIFIERS
#elif defined(__GNUG__)
#define _WHAT_MODIFIERS _GLIBCXX_USE_NOEXCEPT
#else
#define _WHAT_MODIFIERS
#endif

#ifdef NDEBUG
#define DEBUG_FLAG false
#define dbg if (false)
#else
#define DEBUG_FLAG true
#define dbg if (true)
#endif

#define cdbg                                                                  \
    if (!DEBUG_FLAG) {                                                        \
    } else                                                                    \
        std::cerr

#define THROW_(exception, fmt_string, ...)                                    \
    do {                                                                      \
        auto what = fmt::format("[ERROR]: ({0}, {1}): ", __FILE__, __LINE__); \
        what += fmt::format((fmt_string), __VA_ARGS__);                       \
        throw exception(what);                                                \
    } while (0)

#define CHECK_(condition, exception, fmt_string, ...)                         \
    do {                                                                      \
        if (!(condition)) {                                                   \
            THROW_(exception, fmt_string, __VA_ARGS__);                       \
        }                                                                     \
    } while (0)

namespace err {
struct my_except : std::exception {
    const char* what() const _WHAT_MODIFIERS override
    {
        return msg_.c_str();
    }

protected:
    std::string msg_;
};
}

#define EXCEPT(exc_name, def_msg)                                             \
    namespace err {                                                           \
        struct exc_name : my_except {                                         \
            explicit exc_name(const std::string& msg)                         \
                : my_except()                                                 \
            {                                                                 \
                std::string def(def_msg);                                     \
                msg_ = "[ERROR](" + std::string(#exc_name) + ") " + def       \
                    + (def.empty() ? "" : " ") + msg;                         \
            }                                                                 \
        };                                                                    \
    }

#ifndef NDEBUG
#define _THROW_DBG_STR (std::string(__FILE__) + ":" + std::to_string(__LINE__))
#else
#define _THROW_DBG_STR std::string("")
#endif

#define THROW(exc_name, ...)                                                  \
    throw exc_name(_THROW_DBG_STR + " " + std::string(__VA_ARGS__))

#define CHECK_IF(cond, exc_name, ...)                                         \
    do {                                                                      \
        if (cond)                                                             \
            THROW(exc_name, __VA_ARGS__);                                     \
    } while (0)

#endif // STEPPARSE_SRC_UTIL_DEBUG_HPP_
