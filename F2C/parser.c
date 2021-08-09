//
//  parser.c
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall. All rights reserved.
//

#include "parser.h"
#include "tokeniser.h"
#include "codegen.h"
#include "typetable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

/*
 TODO: INTEGER PROMOTIONS
 
 This means that all small integer types, no matter signedness, get implicitly
 converted to (signed) int when used in most expressions.
 
 In all expressions:
 
 i8  -> i32
 u8  -> i32
 i16 -> i32
 u16 -> i32
 
 THEN: if types match, then that's easy
 NEXT: if both same signedness, convert to biggers
 NEXT: the unsigned type is bigger (e.g. u64), the the signed type goes to the unsigned, bigger type (e.g. i32 -> u64)
 NEXT: the signed type is bigger and all values from the other can fit within it, go to the signed, bigger, type
 NEXT: convert to the unsigned type corresponding to the signed type
 
 
 Or, as C99 puts it:
 
 If an int can represent all values of the original type, the value is converted to an int;
 otherwise, it is converted to an unsigned int. These are called the integer promotions.
 All other types are unchanged by the integer promotions.
 
 */


int PsPromote(int a, int b) {
    if (a == TYPE_DEFAULT) {
        a ^= b;
        b ^= a;
        a ^= b;
    }
    if (b == TYPE_DEFAULT) {
        if (a == TYPE_UINT32) return a;
        else return TYPE_INT32;
        
    } else {
        a = PsPromote(a, TYPE_DEFAULT);
        b = PsPromote(b, TYPE_DEFAULT);
        
        if (!TYPE_ISSIGNED(a)) return a;
        if (!TYPE_ISSIGNED(b)) return b;
        
        return TYPE_INT32;
    }
}

TkToken* PsGetState() {
    return parseToken;
}

void PsRestoreState(TkToken* state) {
    parseToken = state;
}

void PsParseError(char* msg) {
    printf("\n");
    fflush(stdout);
    fprintf(stderr, "Parse error: %s\n", msg);
    exit(1);
}

TkToken* PsCheckToken() {
    if (!parseToken) {
        return NULL;
    }
    return parseToken;
}

void PsAdvanceToken() {
    parseToken = parseToken->next;
}

bool PsEatToken(TkTokenType type) {
    if (!parseToken) {
        PsParseError(" ** COMPILER ERROR **");
        return false;
    }
    if (parseToken->type == type) {
        parseToken = parseToken->next;
        return true;
    }
    return false;
}

PsResult PsUnaryExpression() {
    TkToken* tkn = PsCheckToken();
    
    if (tkn->type == TOKEN_PLUS) {
        PsEatToken(TOKEN_PLUS);
        return PsUnaryExpression();
        
    } else if (tkn->type == TOKEN_MINUS) {
        PsEatToken(TOKEN_MINUS);
        
        PsResult res = PsUnaryExpression();
        if (res.reg == -1) {
            res.literal = -res.literal;
            return res;
        }
        if (res.reg == -2) {
            res.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", res, res.identifier);
        }
        CgEmit("neg %r", res);
        return res;
    
    } else if (tkn->type == TOKEN_TIDLE) {
        PsEatToken(TOKEN_TIDLE);

        PsResult res = PsUnaryExpression();
        if (res.reg == -1) {
            res.literal = ~res.literal;
            return res;
        }
        if (res.reg == -2) {
            res.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", res, res.identifier);
        }
        CgEmit("not %r", res);
        return res;
    
    } else if (tkn->type == TOKEN_EXCLAIMATION) {
        PsEatToken(TOKEN_EXCLAIMATION);

        PsResult res = PsUnaryExpression();
        if (res.reg == -1) {
            res.literal = !res.literal;
            return res;
        }
        if (res.reg == -2) {
            res.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", res, res.identifier);
        }
        CgEmit("test %r, %r", res, res);
        CgEmit("setz %r", res);
        return res;
    }
    
    return PsPrimaryExpression();
}

PsResult PsCastExpression() {
    // <cast-expression>           ::= cast <cast-expression> as <type-name>
    //                                 <unary-expression>
    
    if (PsEatToken(TOKEN_CAST)) {
        PsResult castExpr = PsCastExpression();
        if (!castExpr.seen) PsParseError("expected cast expression");
        if (!PsEatToken(TOKEN_AS)) PsParseError("expected 'as'");
        TkToken* typename = PsCheckToken();
        if (!PsEatToken(TOKEN_TYPE_NAME)) PsParseError("expected type name");
        if (!strcmp(typename->lexeme, "u8")) {
            castExpr.type = TYPE_UINT8;
            if (castExpr.reg == -1) {
                castExpr.literal = (uint8_t) castExpr.literal;
                return castExpr;
            }
            if (castExpr.reg == -2) {
                castExpr.reg = CgAllocateRegister();
                CgEmit("mov %r, [%s]", castExpr, castExpr.identifier);
            }
            CgEmit("and %r, 0xFF", castExpr);
        }
        return castExpr;
    }
    
    PsResult res = PsUnaryExpression();
    if (!res.seen) {
        PsParseError("expected primary expression");
    }
    
    return res;
}

PsResult PsMultiplicativeExpression() {
    // <multiplicative-expression> ::= <cast-expression> * <multiplicative-expression>
    //                                 <cast-expression> / <multiplicative-expression>
    //                                 <cast-expression> % <multiplicative-expression>
    //                                 <cast-expression>

    PsResult res = PsCastExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
    
    TkToken* tkn;
        
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_ASTERISK || tkn->type == TOKEN_SLASH || tkn->type == TOKEN_MODULO) {
        PsResult old = res;
                
        PsAdvanceToken();
        res = PsCastExpression();
        if (!res.seen) {
            PsParseError("expected cast expression");
        }
        
        if (tkn->type == TOKEN_ASTERISK) {
            if (res.reg == -1 && old.reg == -1) {
                res.literal *= old.literal;
                continue;
            }
            if (res.reg == -1) {
                PsResult tmp = old;
                old = res;
                res = tmp;
            }
            if (res.reg == -2) {
                res.reg = CgAllocateRegister();
                CgEmit("mov %r, [%s]", res, res.identifier);
            }
            CgEmit("mul %r, %r", res, old);

        } else if (tkn->type == TOKEN_SLASH || tkn->type == TOKEN_MODULO) {
            if (res.reg == -1 && old.reg == -1) {
                if (tkn->type == TOKEN_SLASH) res.literal = old.literal / res.literal;
                else res.literal = old.literal % res.literal;
                continue;
            }
            if (old.reg == -1) {
                old.reg = CgAllocateRegister();
                CgEmit("mov %r, %d", old, old.literal);
            }
            if (old.reg == -2) {
                old.reg = CgAllocateRegister();
                CgEmit("mov %r, [%s]", old, old.identifier);
            }
            CgEmit(tkn->type == TOKEN_SLASH ? "div %r, %r" : "mod %r, %r", old, res);
            
            PsResult tmp = old;
            old = res;
            res = tmp;
        }
    }
    
    return (PsResult){.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsAdditiveExpression() {
    // <additive-expression>       ::= <multiplicative-expression> + <additive-expression>
    //                                 <multiplicative-expression> - <additive-expression>
    //                                 <multiplicative-expression>

    PsResult res = PsMultiplicativeExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
    
    
    TkToken* tkn;
        
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_PLUS || tkn->type == TOKEN_MINUS) {
        PsResult old = res;
        PsAdvanceToken();
        res = PsMultiplicativeExpression();
        if (!res.seen) {
            PsParseError("expected multiplicative expression");
        }
        
        if (tkn->type == TOKEN_PLUS) {
            if (res.reg == -1 && old.reg == -1) {
                res.literal += old.literal;
                continue;
            }
            if (res.reg == -1) {
                PsResult tmp = old;
                old = res;
                res = tmp;
                res.literal = 0;
            }
            if (res.reg == -2) {
                res.reg = CgAllocateRegister();
                CgEmit("mov %r, [%s]", res, res.identifier);
            }
            CgEmit("add %r, %r", res, old);

        } else if (tkn->type == TOKEN_MINUS) {
            if (res.reg == -1 && old.reg == -1) {
                res.literal = old.literal - res.literal;
                continue;
            }
            if (old.reg == -1) {
                old.reg = CgAllocateRegister();
                CgEmit("mov %r, %d", old, old.literal);
            }
            if (old.reg == -2) {
                old.reg = CgAllocateRegister();
                CgEmit("mov %r, [%s]", old, old.identifier);
            }
            CgEmit("sub %r, %r", old, res);

            PsResult tmp = old;
            old = res;
            res = tmp;
        }
    }
    
    return (PsResult) {.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsShiftExpression() {
    // <shift-expression>          ::= <additive-expression> >> <shift-expression>
    //                                 <additive-expression> << <shift-expression>
    //                                 <additive-expression>

    PsResult res = PsAdditiveExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
    
    TkToken* tkn;
    
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_LEFT_SHIFT || tkn->type == TOKEN_RIGHT_SHIFT) {
        PsResult old = res;

        PsAdvanceToken();
        res = PsAdditiveExpression();
        if (!res.seen) {
            PsParseError("expected additive expression");
        }
        
        if (old.reg == -1) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, %d", old, old.literal);
        }
        if (old.reg == -2) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", old, old.identifier);
        }
        CgEmit(tkn->type == TOKEN_LEFT_SHIFT ? "shl %r, %r" : "shr %r, %r", old, res);
        
        PsResult tmp = old;
        old = res;
        res = tmp;
    }
      
    return (PsResult) {.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsEqualityExpression() {
    // <equity-expression>         ::= <relational-expression> == <equity-expression>
    //                                 <relational-expression> != <equity-expression>
    //                                 <relational-expression>

    PsResult res = PsRelationalExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
        
    TkToken* tkn;
    
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_DOUBLE_EQUALS || tkn->type == TOKEN_NOT_EQUALS) {
        PsResult old = res;

        PsAdvanceToken();
        res = PsRelationalExpression();
        if (!res.seen) {
            PsParseError("expected shift expression");
        }
        
        if (old.reg == -1) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, %d", old, old.literal);
        }
        if (old.reg == -2) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", old, old.identifier);
        }
        
        CgEmit("cmp %r, %r", old, res);
        if (tkn->type == TOKEN_DOUBLE_EQUALS) CgEmit("setz %r", old);
        if (tkn->type == TOKEN_NOT_EQUALS)    CgEmit("setnz %r", old);
        
        PsResult tmp = old;
        old = res;
        res = tmp;

    }
    
    return (PsResult) {.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsRelationalExpression() {
    // <relational-expression>     ::= <shift-expression> >= <relational-expression>
    //                                 <shift-expression> <= <relational-expression>
    //                                 <shift-expression> > <relational-expression>
    //                                 <shift-expression> < <relational-expression>
    //                                 <shift-expression>

    PsResult res = PsShiftExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
        
    TkToken* tkn;
    
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_GREATER || tkn->type == TOKEN_GREATER_OR_EQUAL || \
                                         tkn->type == TOKEN_LESSER || tkn->type == TOKEN_LESSER_OR_EQUAL) {
        PsResult old = res;

        PsAdvanceToken();
        res = PsShiftExpression();
        if (!res.seen) {
            PsParseError("expected shift expression");
        }
        
        if (old.reg == -1) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, %d", old, old.literal);
        }
        if (old.reg == -2) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", old, old.identifier);
        }
        
        CgEmit("cmp %r, %r", old, res);
        if (tkn->type == TOKEN_GREATER)          CgEmit("setg %r", old);
        if (tkn->type == TOKEN_LESSER)           CgEmit("setl %r", old);
        if (tkn->type == TOKEN_GREATER_OR_EQUAL) CgEmit("setge %r", old);
        if (tkn->type == TOKEN_LESSER_OR_EQUAL)  CgEmit("setle %r", old);
        
        PsResult tmp = old;
        old = res;
        res = tmp;
    }
    
    return (PsResult) {.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsAndExpression() {
    PsResult res = PsEqualityExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
        
    TkToken* tkn;
    
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_AMPERSAND) {
        PsResult old = res;
        
        PsAdvanceToken();
        res = PsEqualityExpression();
        if (!res.seen) {
            PsParseError("expected equality expression");
        }
        
        if (res.reg == -1 && old.reg == -1) {
            res.literal = old.literal & res.literal;
            continue;
        }
        if (old.reg == -1) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, %d", old, old.literal);
        }
        if (old.reg == -2) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", old, old.identifier);
        }
        CgEmit("and %r, %r", old, res);

        PsResult tmp = old;
        old = res;
        res = tmp;
    }
    
    return (PsResult) {.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsXorExpression() {
    PsResult res = PsAndExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
        
    TkToken* tkn;
    
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_CARET) {
        PsResult old = res;
        
        PsAdvanceToken();
        res = PsAndExpression();
        if (!res.seen) {
            PsParseError("expected equality expression");
        }
        
        if (res.reg == -1 && old.reg == -1) {
            res.literal = old.literal & res.literal;
            continue;
        }
        if (old.reg == -1) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, %d", old, old.literal);
        }
        if (old.reg == -2) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", old, old.identifier);
        }
        CgEmit("xor %r, %r", old, res);

        PsResult tmp = old;
        old = res;
        res = tmp;
    }
    
    return (PsResult) {.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsInclusiveOrExpression() {
    PsResult res = PsXorExpression();
    if (!res.seen) {
        return (PsResult){.seen = false};
    }
        
    TkToken* tkn;
    
    while ((void)(tkn = PsCheckToken()), tkn->type == TOKEN_PIPE) {
        PsResult old = res;
        
        PsAdvanceToken();
        res = PsXorExpression();
        if (!res.seen) {
            PsParseError("expected equality expression");
        }
        
        if (res.reg == -1 && old.reg == -1) {
            res.literal = old.literal & res.literal;
            continue;
        }
        if (old.reg == -1) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, %d", old, old.literal);
        }
        if (old.reg == -2) {
            old.reg = CgAllocateRegister();
            CgEmit("mov %r, [%s]", old, old.identifier);
        }
        CgEmit("or %r, %r", old, res);

        PsResult tmp = old;
        old = res;
        res = tmp;
    }
    
    return (PsResult) {.seen = true, .reg = res.reg, .identifier = res.identifier, .literal = res.literal, .type = res.type};
}

PsResult PsIntegralExpression() {
    return PsInclusiveOrExpression();
}

PsResult PsExpression() {
    TkToken* next = PsCheckToken();
    if (next->type == TOKEN_LET) {
        PsEatToken(TOKEN_LET);
        
        TkToken* identifier = PsCheckToken();
        char* typename = "*";
        if (!PsEatToken(TOKEN_IDENTIFIER)) PsParseError("expected identifier");
        if (PsEatToken(TOKEN_COLON)) {
            TkToken* typen = PsCheckToken();
            if (!PsEatToken(TOKEN_TYPE_NAME)) PsParseError("expected type name");
            typename = typen->lexeme;
        }
        
        PsAllocateType(identifier->lexeme, typename);
        
        if (!PsEatToken(TOKEN_EQUALS)) PsParseError("expected assignment");

        PsResult n = PsIntegralExpression();
        if (n.reg == -2) {
            PsResult ncopy = n;
            ncopy.reg = CgAllocateRegister();
            
            CgEmit("mov %r, %r", ncopy, n);
            CgEmit("mov [%s], %r", identifier->lexeme, ncopy);

        } else {
            CgEmit("mov [%s], %r", identifier->lexeme, n);
        }
        
        return n;
    }
        
    PsResult res = PsIntegralExpression();
    if (!res.seen) {
        PsParseError("expected expression");
        return (PsResult) {.seen = false};
    }

    next = PsCheckToken();
    if (next->type == TOKEN_ASSIGNMENT) {
        PsEatToken(TOKEN_ASSIGNMENT);
        
        PsResult n = PsExpression();
        if (n.reg == -2) {
            PsResult ncopy = n;
            ncopy.reg = CgAllocateRegister();
            
            CgEmit("mov %r, %r", ncopy, n);
            CgEmit("mov %r, %r", res, ncopy);

        } else {
            CgEmit("mov %r, %r", res, n);
        }
        
        return n;
    }
    
    return res;
}

PsResult PsPrimaryExpression() {
    // <primary-expression>        ::= <identifier>
    //                                 <literal>
    //                                 ( <expression> )
       
    TkToken* tkn = PsCheckToken();

    if (PsEatToken(TOKEN_IDENTIFIER)) {
        return (PsResult) {.seen = true, .identifier = tkn->lexeme, .reg = -2};
        
    } else if (PsEatToken(TOKEN_INTEGER_LITERAL)) {
        return (PsResult) {.seen = true, .literal = atoi(tkn->lexeme), .type = TYPE_INT32, .reg = -1};
        
    } else if (PsEatToken(TOKEN_STRING_LITERAL)) {
        return (PsResult) {.seen = true, .reg=999};
    
    } else if (PsEatToken(TOKEN_LEFT_PARENTHESIS)) {
        PsResult exprRes = PsExpression();
        if (!exprRes.seen) {
            PsParseError("expected expression");
        }
        if (!PsEatToken(TOKEN_RIGHT_PARENTHESIS)) {
            PsParseError("expected )");
        }
        return exprRes;
        
    } else {
        PsParseError("expected primary expression");
        return (PsResult) {.seen = false};
    }
}
