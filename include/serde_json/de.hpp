#include <cstring>

#include "serde/serde.hpp"
#include "serde_json/comma_separated.hpp"
#include "fst/fst.hpp"

namespace json {

namespace de {
    using fst::result::Ok;
    using fst::result::Err;
    using fst::error::Error;

    struct Deserializer {
        const char *input;
        Deserializer(const char *in) : input(in) {};

        using Error = Error;
        template<typename T>
        using Result = result::Result<T, Error>;

        // Parsing
        Result<char> peek_char() {
            if (*this->input == '\0') {
                return Err{Error("EOF")};
            } else {
                return Ok(*this->input);
            }
        }
        Result<char> next_char() {
            char ch = TRY(this->peek_char());
            this->input++;
            return Ok(ch);
        }
        Result<bool> parse_bool() {
            if (strncmp(this->input, "true", 4) == 0) {
                this->input += strlen("true");
                return Ok(true);
            } else if (strncmp(this->input, "false", 5) == 0) {
                return Ok(false);
            }
            return Err(Error("Expected bool"));
        }
        template<typename T>
        Result<T> parse_unsigned() {
            char ch = TRY(this->next_char());
            if (ch <= '0' || ch >= '9')
                return Err(Error("Expected unsigned integer"));
            T res = ch - '0';
            while (true) {
                match(this->peek_char()) {{
                    of(Ok, (ch, (ch >= '0' && ch <= '9'))) {
                        this->input++;
                        res *= 10;
                        res += ch - '0';
                    }
                    of_default { return Ok(res); }
                }}
            }
        }
        template<typename T>
        Result<T> parse_signed() {
            bool neg = TRY(this->peek_char()) == '-';
            if (neg) this->input++;
            T res = TRY(this->parse_unsigned<T>());
            return Ok(neg ? -res : res);
        }
        Result<str> parse_string() {
            if (TRY(this->next_char()) != '"') return Err(Error("Expected string"));
            const char *end = strchr(this->input, '"');
            if (end == NULL) return Err(Error("EOF"));
            str res(this->input, end - this->input);
            this->input = end + 1;
            return Ok(res);
        }
        // Deserializer trait
        template<typename V>
        Result<typename V::Value> deserialize_any(V visitor) {
            switch (TRY(this->peek_char())) {
            case 't':
            case 'f':
                this->deserialize_bool(visitor);
                break;
            case '"':
                this->deserialize_str(visitor);
                break;
            case '0' ... '9':
                this->deserialize_ulong_long(visitor);
                break;
            case '-':
                this->deserialize_long_long(visitor);
                break;
            }
        }
        template<typename V>
        Result<typename V::Value> deserialize_bool(V visitor) {
            return visitor.visit_bool(TRY(this->parse_bool()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_short(V visitor) {
            return visitor.visit_short(TRY(this->parse_signed<short>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_int(V visitor) {
            return visitor.visit_int(TRY(this->parse_signed<int>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_long(V visitor) {
            return visitor.visit_long(TRY(this->parse_signed<long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_long_long(V visitor) {
            return visitor.visit_long_long(TRY(this->parse_signed<long long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_ushort(V visitor) {
            return visitor.visit_short(TRY(this->parse_unsigned<unsigned short>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_uint(V visitor) {
            return visitor.visit_int(TRY(this->parse_unsigned<unsigned int>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_ulong(V visitor) {
            return visitor.visit_long(TRY(this->parse_unsigned<unsigned long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_ulong_long(V visitor) {
            return visitor.visit_long_long(TRY(this->parse_unsigned<unsigned long long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_str(V visitor) {
            return visitor.visit_str(TRY(this->parse_string()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_identifier(V visitor) {
            return this->deserialize_str(visitor);
        }
        template<typename V>
        Result<typename V::Value> deserialize_map(V visitor) {
            if (TRY(this->next_char()) == '{') {
                auto value = TRY(visitor.visit_map(CommaSeparated(*this)));
                if (TRY(this->next_char()) == '}') {
                    return Ok(value);
                } else {
                    return Err(Error("Expected map end"));
                }
            } else {
                return Err(Error("Expected map"));
            }
        }
        template<typename V>
        Result<typename V::Value>
        deserialize_struct(
            const char *_name,
            const char *_fields[],
            V visitor
        ) {
            return deserialize_map(visitor);
        }
    };
}
namespace de {
    template<typename T>
    using Result = result::Result<T, Error>;

    using serde::de::Visitor;

    struct BoolVisitor : Visitor<bool> {
        Result<Value> visit_bool(Value value) override {
            return Ok(value);
        }
    };
    struct LongLongVisitor : Visitor<long long> {
        Result<Value> visit_long_long(Value value) override {
            return Ok(value);
        }
    };
    struct StrVisitor : Visitor<str> {
        Result<Value> visit_str(Value value) {
            return Ok(value);
        }
    };
    template<typename T>
    struct StructVisitor : Visitor<T> {
        using Value = typename Visitor<T>::Value;

        Result<Value> visit_map(Value value) {

        }
    };
}
}

