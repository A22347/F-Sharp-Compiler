//
//  tokeniser.h
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall.
//

#ifndef tokeniser_h
#define tokeniser_h

typedef enum TkTokenType {
    TOKEN_SIZEOF,               // sizeof
    TOKEN_CAST,                 // cast
    TOKEN_AS,                   // as
    TOKEN_IF,                   // if
    TOKEN_THEN,                 // then
    TOKEN_ELSE,                 // else
    TOKEN_LET,                  // let
    TOKEN_WHILE,                // while
    TOKEN_DO,                   // do
    TOKEN_TO,                   // to
    TOKEN_FOR,                  // for
    TOKEN_DOWNTO,               // downto
    TOKEN_REC,                  // rec
    TOKEN_PUBLIC,               // public
    TOKEN_MUTABLE,              // mutable
    TOKEN_TYPE,                 // type
    TOKEN_DELEGATE,             // delegate
    TOKEN_WITH,                 // with
    TOKEN_SET,                  // set
    TOKEN_GET,                  // get
    TOKEN_AND_KEYWORD,          // and
    TOKEN_WHEN,                 // when
    TOKEN_MATCH,                // match
    TOKEN_EQUALS,               // =
    TOKEN_ASSIGNMENT,           // <-
    TOKEN_YIELD,                // ->
    TOKEN_UNDERSCORE,           // _
    TOKEN_DOUBLE_EQUALS,        // ==
    TOKEN_NOT_EQUALS,           // !=
    TOKEN_GREATER,              // >
    TOKEN_GREATER_OR_EQUAL,     // >=
    TOKEN_LESSER,               // <
    TOKEN_LESSER_OR_EQUAL,      // <=
    TOKEN_PLUS,                 // +
    TOKEN_MINUS,                // -
    TOKEN_ASTERISK,             // *
    TOKEN_SLASH,                // /
    TOKEN_AMPERSAND,            // &
    TOKEN_LOGICAL_AND,          // &&
    TOKEN_LOGICAL_OR,           // ||
    TOKEN_EXCLAIMATION,         // !
    TOKEN_TIDLE,                // ~
    TOKEN_PIPE,                 // |
    TOKEN_LEFT_SHIFT,           // <<
    TOKEN_RIGHT_SHIFT,          // >>
    TOKEN_CARET,                // ^
    TOKEN_MODULO,               // %
    TOKEN_POWER,                // pow
    TOKEN_COMMA,                // ,
    TOKEN_COLON,                // :
    TOKEN_QUESTION,             // ?
    TOKEN_LEFT_PARENTHESIS,     // (
    TOKEN_RIGHT_PARENTHESIS,    // )
    TOKEN_LEFT_SQUARE_BRACKET,  // [
    TOKEN_RIGHT_SQUARE_BRACKET, // ]
    TOKEN_LEFT_CURLY_BRACKET,   // {
    TOKEN_RIGHT_CURLY_BRACKET,  // }
    TOKEN_SEMICOLON,            // ;
    TOKEN_LEFT_C_BRACKET,       // @{
    TOKEN_RIGHT_C_BRACKET,      // }@
    TOKEN_ELLIPSIS,             // ...
    TOKEN_RANGE,                // ..
    TOKEN_DOUBLE_QUESTION,      // ?? 
    TOKEN_STRING_LITERAL,       //
    TOKEN_INTEGER_LITERAL,      //
    TOKEN_CHARACTER_LITERAL,    //
    TOKEN_IDENTIFIER,           //
    TOKEN_TYPE_NAME,            //
    TOKEN_PLEASE_CLASSIFY,      //
    TOKEN_NULL,                 //
    TOKEN_UNKNOWN,              //
    TOKEN_EOF,

} TkTokenType;

typedef struct TkToken {
    char* lexeme;
    TkTokenType type;
    struct TkToken* next;
    
} TkToken;

extern TkToken* parseToken;

TkTokenType TkClassifyToken(char* lexeme);
void TkAddToken(char* lexeme, TkTokenType type);
void TkInitTokenList(void);
int TkTokenise(char* str);

#endif
