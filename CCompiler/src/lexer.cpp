#include "lexer.h"

#include <cctype>

std::vector<token> lexer::lex_contents()
{
    std::vector<token> tokens;

    while (current() != chardefs::eof)
    {
        tokens.push_back(next_token());
    }

    return tokens;
}

token lexer::next_token()
{
    // Cache starting index of token
    token_start_ = index_;

    // Cache first char
    const char first_char = current();

    // Nothing to lex if current() == eof
    if (first_char == chardefs::eof)
    {
        return create_token(token_type::eof);
    }

    // Look for keyword or literal
    if (first_char == chardefs::quote)
    {
        consume();
        return read_string();
    }
    if (std::isalpha(first_char) || first_char == chardefs::underscore)
    {
        return read_identifier();
    }
    if (std::isdigit(first_char))
    {
        return read_integer();
    }
    if (std::isspace(first_char))
    {
        skip_space();
        return next_token();
    }

    // Not keyword nor literal, so look for operator or separator
    switch (first_char)
    {
    case chardefs::plus:
        {
            consume();
            return create_token(token_type::plus);
        }
    case chardefs::dash:
        {
            consume();
            return create_token(token_type::minus);
        }
    case chardefs::asterisk:
        {
            consume();
            return create_token(token_type::asterisk);
        }
    case chardefs::forward_slash:
        {
            consume();
            return create_token(token_type::forward_slash);
        }
    case chardefs::open_paren:
        {
            consume();
            return create_token(token_type::open_paren);
        }
    case chardefs::close_paren:
        {
            consume();
            return create_token(token_type::close_paren);
        }
    case chardefs::open_brace:
        {
            consume();
            return create_token(token_type::open_brace);
        }
    case chardefs::close_brace:
        {
            consume();
            return create_token(token_type::close_brace);
        }
    case chardefs::open_angle:
        {
            consume();
            return create_token(token_type::open_angle);
        }
    case chardefs::close_angle:
        {
            consume();
            return create_token(token_type::close_angle);
        }
    case chardefs::open_square:
        {
            consume();
            return create_token(token_type::open_square);
        }
    case chardefs::close_square:
        {
            consume();
            return create_token(token_type::close_square);
        }
    case chardefs::comma:
        {
            consume();
            return create_token(token_type::comma);
        }
    case chardefs::semicolon:
        {
            consume();
            return create_token(token_type::semicolon);
        }
    }

    // Could not identify the token
    return read_unknown();
}

token lexer::read_string()
{
    // TODO: Handle missing closing quote gracefully

    while (current() != chardefs::quote && 
           current() != chardefs::back_slash &&
           current() != chardefs::eof)
    {
        consume();
    }

    const char breaking_char = current();

    switch (breaking_char)
    {
    case chardefs::quote:
        {
            consume();
            break;
        }
    case chardefs::back_slash:
        {
            consume();
            return read_escaped();
        }
    case chardefs::eof:
        {
            return create_token(token_type::invalid);
        }
    }

    return create_token(token_type::string);
}

token lexer::read_escaped()
{
    const char first_char = current();

    // TODO: Handle octal and hex

    switch (first_char)
    {
    case '\'':
    case '\"':
    case '\?':
    case '\\':
    case '\a':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
        {
            consume();
        }
    }

    // If no cases matched, ignore backslash

    return read_string();
}

token lexer::read_identifier()
{
    while (std::isalnum(current()) || current() == chardefs::underscore)
    {
        consume();
    }
    if (is_keyword())
    {
        return create_token(token_type::keyword);
    }
    return create_token(token_type::identifier);
}

void lexer::skip_space()
{
    while (std::isspace(current()))
    {
        next();
    }
}

token lexer::read_integer()
{
    while (std::isdigit(current()))
    {
        consume();
    }

    // A non-digit char was hit
    const char breaking_char = current();

    // Handle special cases
    switch (breaking_char)
    {
    case chardefs::period:
        {
            consume();
            return read_double();
        }
    case 'e':
        {
            consume();
            return read_exponent();
        }
    }

    return create_token(token_type::integer);
}

token lexer::read_double()
{
    // Keep track of mantissa length
    std::size_t mantissa_length = 0;

    while (std::isdigit(current()))
    {
        mantissa_length++;
        consume();
    }

    // A double must have a mantissa
    if (mantissa_length == 0)
    {
        return read_unknown();
    }

    // A non-digit char was hit
    const char breaking_char = current();

    // Handle special cases
    switch (breaking_char)
    {
    case chardefs::period:
        {
            return read_unknown();
        }
    case 'e':
        {
            consume();
            return read_exponent();
        }
    case 'f':
        {
            consume();
            return read_float();
        }
    }

    return create_token(token_type::double_precision);
}

token lexer::read_float()
{
    // This method is only called when current() == 'f',
    // at which point the float token is complete.
    return create_token(token_type::single_precision);
}

token lexer::read_exponent()
{
    // Consume an optional '+' or '-' character
    if (current() == chardefs::plus || current() == chardefs::dash)
    {
        consume();
    }

    std::size_t exponent_length = 0;
    while (std::isdigit(current()))
    {
        exponent_length++;
        consume();
    }

    // An exponent must have a value
    if (exponent_length == 0)
    {
        return read_unknown();
    }

    // Check for float suffix
    if (current() == 'f')
    {
        consume();
        return read_float();
    }

    return create_token(token_type::double_precision);
}

token lexer::read_unknown()
{
    while (!std::isspace(current()) && !current() == chardefs::eof)
    {
        consume();
    }
    return create_token(token_type::invalid);
}

bool lexer::is_keyword() const
{
    // TODO: Maybe automate this
    const std::string text = buffer_.str();
    return (text == keyworddefs::char_kw     ||
            text == keyworddefs::int_kw      ||
            text == keyworddefs::double_kw   ||
            text == keyworddefs::float_kw    ||
            text == keyworddefs::struct_kw   ||
            text == keyworddefs::enum_kw     ||
            text == keyworddefs::void_kw     ||
            text == keyworddefs::short_kw    ||
            text == keyworddefs::long_kw     ||
            text == keyworddefs::const_kw    ||
            text == keyworddefs::static_kw   ||
            text == keyworddefs::if_kw       ||
            text == keyworddefs::else_kw     ||
            text == keyworddefs::for_kw      ||
            text == keyworddefs::while_kw    ||
            text == keyworddefs::break_kw    ||
            text == keyworddefs::continue_kw ||
            text == keyworddefs::return_kw);
}
