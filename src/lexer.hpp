#pragma once

#include <cstdint>
#include <expected>
#include <fstream>
#include <string>
#include <vector>

namespace og::gs2 {
    using string = std::string;
    using string_view = std::string_view;
    using byte = uint8_t;

    struct source_position {
        int line, column;
    };

    enum class token_kind : byte {
        eof,
        keyword,
        identifier,
        number,
        string,
        comment,
        invalid,
        dot,                // .
        comma,              // ,
        question,           // ?
        colon,              // :
        semicolon,          // ;
        lbrace,             // {
        rbrace,             // }
        lparen,             // (
        rparen,             // )
        lbracket,           // [
        rbracket,           // ]
        op_add,             // +
        op_subtract,        // -
        op_multiply,        // *
        op_divide,          // /
        op_modulo,          // %
        op_exponent,        // ^
        op_concat,          // @
        op_lt,              // <
        op_lte,             // <=
        op_gt,              // >
        op_gte,             // >=
        op_eq,              // ==
        op_neq,             // !=
        op_and,             // &&
        op_or,              // ||
        op_not,             // !
        op_assign,          // =
        op_assign_add,      // +=
        op_assign_subtract, // -=
        op_assign_multiply, // *=
        op_assign_divide,   // /=
        op_assign_concat,   // @=
        op_increment,       // ++
        op_decrement,       // --
        op_in,              // in
    };

    auto token_kind_string(token_kind kind) -> string_view;

    enum class keyword_kind : byte {
        unknown,
        if_,
        else_,
        while_,
        for_,
        foreach_,
        function,
        return_,
        new_,
        this_,
        thiso,
        temp,
        with,
        in,
        break_,
        continue_,
        null,
    };

    struct token {
        token_kind kind;
        string lexeme;
        keyword_kind keyword;
        source_position position;
        double value;
    };

    enum class lexer_error_kind : byte {
        bad_stream,
        bad_token
    };

    struct lexer_error {
        lexer_error_kind kind;
        string message;
        source_position position;
    };

    using tokens = std::vector<token>;
    using tokenize_result = std::expected<tokens, lexer_error>;
    using stream = std::ifstream;

    auto tokenize(stream &ifs) -> tokenize_result;
}
