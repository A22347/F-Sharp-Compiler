//
//  tokeniser.c
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall. All rights reserved.
//

#include "tokeniser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

TkToken* headToken;
TkToken* tailToken;
TkToken* parseToken;

TkTokenType TkClassifyToken(char* lexeme) {
    return TOKEN_UNKNOWN;
}

void TkAddToken(char* lexeme, TkTokenType type) {
    if (type == TOKEN_PLEASE_CLASSIFY) {
        type = TkClassifyToken(lexeme);
    }
    
    char* allocLexeme = malloc(1 + strlen(lexeme));
    strcpy(allocLexeme, lexeme);
        
    tailToken->next = malloc(sizeof(TkToken));
    tailToken = tailToken->next;
    tailToken->lexeme = allocLexeme;
    tailToken->next = NULL;
    tailToken->type = type;
}

void TkInitTokenList() {
    headToken = malloc(sizeof(TkToken));
    tailToken = headToken;
    
    headToken->lexeme = NULL;
    headToken->next = NULL;
    headToken->type = TOKEN_NULL;
    
    parseToken = headToken;
}

#define TOKEN_LENGTH_LIMIT 255

#define __TOKEN_KEYWORD            0
#define __TOKEN_OPERATOR           1
#define __TOKEN_IDENTIFIER         2
#define __TOKEN_STRING_LITERAL     3
#define __TOKEN_INTEGER_LITERAL    4
#define __TOKEN_CHAR_LITERAL       5

#define C_OK                            0
#define C_TOKEN_ERROR                   1
#define C_TOKEN_NO_FILE                 2
#define C_PP_CANNOT_CREATE_TMPFILE      3
#define C_PP_CANNOT_OPEN_SOURCE_FILE    4
#define C_TRIGRAPH_LONG_LINE            5

char tokenTypeStrs[][12] = {
    "KEYWORD   ",
    "OPERATOR  ",
    "IDENTIFIER",
    "STRING LIT",
    "INT LIT   ",
    "CHAR LIT  ",
};

char tokens[][16] = {
    "return",
    "while",
    "void",
    "else",
    "char",
    "for",
    "int",
    "if",
    "%:%:",
    ">>=",
    "<<=",
    "/*",
    "*/",
    "<:",
    ":>",
    "<%",
    "%>",
    "%:",
    "<-",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "&=",
    "|=",
    "^=",
    ">>",
    "<<",
    "&&",
    "||",
    "++",
    "--",
    "!=",
    "==",
    ">=",
    "<=",
    "+",
    "-",
    "*",
    "!",
    "~",
    ".",
    "=",
    ">",
    "<",
    "&",
    "|",
    "^",
    "/",
    "%",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    ";",
    ",",
    "?",
    ":",
    "##",
    "#",
};


void TkFoundToken(char* token, int t)
{    
    //handle diagraphs
    if (!strcmp(token, "<:")) strcpy(token, "[");
    if (!strcmp(token, ":>")) strcpy(token, "]");
    if (!strcmp(token, "<%")) strcpy(token, "{");
    if (!strcmp(token, "%>")) strcpy(token, "}");
    if (!strcmp(token, "%:%:")) strcpy(token, "##");
    if (!strcmp(token, "%:")) strcpy(token, "#");
    
    TkTokenType type = TOKEN_UNKNOWN;
    if (!strcmp(token, "<-")) type = TOKEN_ASSIGNMENT;
    else if (!strcmp(token, "+")) type = TOKEN_PLUS;
    else if (!strcmp(token, "-")) type = TOKEN_MINUS;
    else if (!strcmp(token, "^")) type = TOKEN_CARET;
    else if (!strcmp(token, "|")) type = TOKEN_PIPE;
    else if (!strcmp(token, "*")) type = TOKEN_ASTERISK;
    else if (!strcmp(token, "/")) type = TOKEN_SLASH;
    else if (!strcmp(token, "%")) type = TOKEN_MODULO;
    else if (!strcmp(token, "&")) type = TOKEN_AMPERSAND;
    else if (!strcmp(token, "(")) type = TOKEN_LEFT_PARENTHESIS;
    else if (!strcmp(token, ")")) type = TOKEN_RIGHT_PARENTHESIS;
    else if (!strcmp(token, "!")) type = TOKEN_EXCLAIMATION;
    else if (!strcmp(token, "<<")) type = TOKEN_LEFT_SHIFT;
    else if (!strcmp(token, ">>")) type = TOKEN_RIGHT_SHIFT;
    else if (!strcmp(token, "<")) type = TOKEN_LESSER;
    else if (!strcmp(token, "<=")) type = TOKEN_LESSER_OR_EQUAL;
    else if (!strcmp(token, ">")) type = TOKEN_GREATER;
    else if (!strcmp(token, ">=")) type = TOKEN_GREATER_OR_EQUAL;
    else if (!strcmp(token, "~")) type = TOKEN_TIDLE;
    else if (!strcmp(token, "==")) type = TOKEN_DOUBLE_EQUALS;
    else if (!strcmp(token, "!=")) type = TOKEN_NOT_EQUALS;
    else if (!strcmp(token, ":")) type = TOKEN_COLON;
    else if (!strcmp(token, "=")) type = TOKEN_EQUALS;

    else if (!strcmp(token, "cast")) type = TOKEN_CAST;
    else if (!strcmp(token, "as")) type = TOKEN_AS;
    else if (!strcmp(token, "type")) type = TOKEN_TYPE;
    else if (!strcmp(token, "let")) type = TOKEN_LET;

    else if (!strcmp(token, "char")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "short")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "int")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "u8")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "u16")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "u32")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "i8")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "i16")) type = TOKEN_TYPE_NAME;
    else if (!strcmp(token, "i32")) type = TOKEN_TYPE_NAME;
    else {
        if (t == __TOKEN_IDENTIFIER) type = TOKEN_IDENTIFIER;
        if (t == __TOKEN_INTEGER_LITERAL) type = TOKEN_INTEGER_LITERAL;
    }
    
    TkAddToken(token, type);

    //printf("Found %s: %s, %d\n", tokenTypeStrs[t], token, (int) type);
}

int TkTokenise(char* str)
{
    int safe = 0;
    int current = 0;
    int iter = 0;
    int iterPtr = 0;
    char tkn[TOKEN_LENGTH_LIMIT];
    memset(tkn, 0, TOKEN_LENGTH_LIMIT);

    if (!str) {
        return C_TOKEN_NO_FILE;
    }

    while (current < strlen(str)) {
        char c = str[current++];

        if (c == tokens[iter][iterPtr]) {
            //found potentially part of a token
            tkn[iterPtr++] = c;

            //check for completed token
            if (strlen(tokens[iter]) == iterPtr) {
                TkFoundToken(tkn, isalpha(tkn[0]) ? __TOKEN_KEYWORD : __TOKEN_OPERATOR);

                memset(tkn, 0, TOKEN_LENGTH_LIMIT);
                safe = current;
                iter = 0;
                iterPtr = 0;
            }

        } else {
            memset(tkn, 0, TOKEN_LENGTH_LIMIT);

            //revert back to the start of the unknown token
            current = safe;

            //try the next token type
            iter++;
            iterPtr = 0;

            if (iter == sizeof(tokens) / sizeof(tokens[0])) {
                //now check for identifiers, string literals and integers

                c = str[current++];

                if (c == '"') {
                    tkn[iterPtr++] = '"';

                    bool escape = false;
                    while (1) {
                        c = str[current++];
                        if (!escape && c == '\\') {
                            escape = true;
                            tkn[iterPtr++] = '\\';
                            continue;
                        }
                        tkn[iterPtr++] = c;
                        if (!escape && c == '"') break;
                        escape = false;
                    }
                    ++current;

                    TkFoundToken(tkn, __TOKEN_STRING_LITERAL);

                } else if (c == '\'') {
                    tkn[iterPtr++] = '\'';

                    bool escape = false;
                    while (1) {
                        c = str[current++];
                        if (!escape && c == '\\') {
                            escape = true;
                            tkn[iterPtr++] = '\\';
                            continue;
                        }
                        tkn[iterPtr++] = c;
                        if (!escape && c == '\'') break;
                        escape = false;
                    }
                    ++current;

                    TkFoundToken(tkn, __TOKEN_CHAR_LITERAL);

                } else if (isdigit(c)) {
                    // TODO: hexadecimal, underscores to sepearate digits, binary, octal
                    do {
                        tkn[iterPtr++] = c;
                        c = str[current++];

                    //this makes up a 'preprocessing number'
                    } while (isalnum(c) || c == '_' || c == '.' || c == '+' || c == '-');

                    TkFoundToken(tkn, __TOKEN_INTEGER_LITERAL);

                } else if (isalpha(c) || c == '_') {
                    do {
                        tkn[iterPtr++] = c;
                        c = str[current++];
                    } while (isalnum(c) || c == '_');

                    TkFoundToken(tkn, __TOKEN_IDENTIFIER);

                } else if (isspace(c)) {
                    ++current;

                } else {
                    printf("c = %c\n", c);
                    printf("Token error.\n");
                    return C_TOKEN_ERROR;
                }

                memset(tkn, 0, TOKEN_LENGTH_LIMIT);
                --current;
                safe = current;
                iter = 0;
                iterPtr = 0;
            }
        }
    }

    return C_OK;
}
