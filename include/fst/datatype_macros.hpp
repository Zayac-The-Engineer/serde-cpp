#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "cursed_macros.h"

// FIELD is (TAG, [TYPE])

/******************************************************************************/
#define DATA_INITIALIZER_TYPE_IMPL(TAG, ...)                              \
    __VA_OPT__(const CAR(__VA_ARGS__) TAG##_value;)                       \
    TAG() = default;                                                      \
    ~TAG() = default;                                                     \
    TAG(TAG&&) = delete;                                                  \
    TAG(const TAG&) = delete;                                             \
    __VA_OPT__(TAG(const CAR(__VA_ARGS__) value) : TAG##_value(value) {}) \

#define _DATA_INITIALIZER_TYPE(TAG, ...)             \
    __VA_OPT__(template<typename CAR(__VA_ARGS__)>)  \
    struct TAG {                                     \
        DATA_INITIALIZER_TYPE_IMPL(TAG, __VA_ARGS__) \
    };
#define DATA_INITIALIZER_TYPE(FIELD) _DATA_INITIALIZER_TYPE FIELD
/******************************************************************************/

#define DATA_INITIALIZER_TYPES(...) FOREACH(DATA_INITIALIZER_TYPE, __VA_ARGS__)

/******************************************************************************/
#define _DATA_ENUM(TAG, ...) TAG,
#define DATA_ENUM(FIELD) _DATA_ENUM FIELD
/******************************************************************************/

/******************************************************************************/
#define _DATA_FIELD(TAG, ...) __VA_OPT__(CAR(__VA_ARGS__) TAG##_val;)
#define DATA_FIELD(FIELD) _DATA_FIELD FIELD
/******************************************************************************/

/******************************************************************************/
#define _DATA_CONSTRUCTOR_BODY(TAG, ...) \
    (TAG __VA_OPT__(<CAR(__VA_ARGS__)>) &&TAG) : tag(Tag::TAG) __VA_OPT__(, TAG##_val(TAG.TAG##_value)) {}

#define DATA_CONSTRUCTOR_BODY(FIELD) _DATA_CONSTRUCTOR_BODY FIELD
#define DATA_CONSTRUCTOR(TYPENAME) TYPENAME DATA_CONSTRUCTOR_BODY
/******************************************************************************/

/******************************************************************************/
#define _DATA_COPY_CONSTRUCTOR(TAG, ...)                                   \
    case Tag::TAG:                                                         \
    __VA_OPT__(new (&this->TAG##_val) CAR(__VA_ARGS__)(source.TAG##_val);) \
    break;

#define DATA_COPY_CONSTRUCTOR(FIELD) _DATA_COPY_CONSTRUCTOR FIELD
/******************************************************************************/

/******************************************************************************/
#define _DATA_DESTRUCTOR(TAG, ...)                   \
    case Tag::TAG:                                   \
    __VA_OPT__(this->TAG##_val.~CAR(__VA_ARGS__)();) \
               break;

#define DATA_DESTRUCTOR(FIELD) _DATA_DESTRUCTOR FIELD
/******************************************************************************/

/**
 * @brief   Generic definition of a Rust-like Enum (sum type)
 * @details Usage:
 *              data(NAME,
 *                   (TAG, TYPE),
 *                   ...)
 * @param   NAME name of the surrounding struct
 * @param   ... Type definition
 */
#define DATA(NAME, ...)            \
    public:                                             \
    FOREACH(DATA_CONSTRUCTOR(NAME), __VA_ARGS__)        \
    NAME(const NAME &source) : tag(source.tag) {        \
        switch (source.tag) {                           \
            FOREACH(DATA_COPY_CONSTRUCTOR, __VA_ARGS__) \
        }                                               \
    }                                                   \
    ~NAME() {                                           \
        switch (this->tag) {                            \
            FOREACH(DATA_DESTRUCTOR, __VA_ARGS__)       \
        }                                               \
    }                                                   \
    union {                                             \
        FOREACH(DATA_FIELD, __VA_ARGS__)                \
    };                                                  \
    enum class Tag {FOREACH(DATA_ENUM, __VA_ARGS__)} tag

/**
 * @brief   Rust-like match statement (not an expression!)
 * @details Match statement only works with enums directly accessible in the namespace.
 *          Cannot specify multiple patterns of same tag with different predicates;
 *          that behavior is to be handled in #of_default.
 *          Usage:
 *              match(EXPR) {{
 *                  of(TAG, ...)
 *                  ...
 *                  [of_default]
 *              }}
 * @param   EXPR Expression returning a tagged union
 */
#define match(EXPR) \
    switch (auto ___self = EXPR; ___self.tag)

/******************************************************************************/
#define OF_PATTERN_PREDICATE(...) \
    __VA_OPT__(if (!(CAR __VA_ARGS__)) goto ___default;)
/******************************************************************************/

/**
 * @brief Specifies branch of match statement
 * @details Usage:
 *          of(TAG, ...) statement
 * @param TAG enum value to match to;
 * @param ... [(TEMP[, (PREDICATE)])]
 *            If predicate doesn't hold, forwards to of_default
 */
#define of(TAG, ...)                                         \
    break;}                                                  \
    case decltype(___self)::Tag::TAG:{                       \
        __VA_OPT__(auto CAR __VA_ARGS__ = ___self.TAG##_val; \
                   OF_PATTERN_PREDICATE(CDR __VA_ARGS__))

/**
 * @brief default brainch of match statement (kinda like _ in Rust)
 * @details Usage:
 *          of_default statement
 */
#define of_default \
    break;}        \
    default:{ ___default:

#endif // DATATYPE_H_