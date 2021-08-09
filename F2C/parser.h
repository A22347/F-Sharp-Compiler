//
//  parser.h
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall. All rights reserved.
//

#ifndef parser_h
#define parser_h

#include <stdbool.h>

#include "tokeniser.h"

#define TYPE_INT32      0
#define TYPE_UINT32     1
#define TYPE_INT16      2
#define TYPE_UINT16     3
#define TYPE_INT8       4
#define TYPE_UINT8      5
#define TYPE_DEFAULT    (-1)

#define TYPE_ISSIGNED(x) (!(x & 1))

typedef struct PsResult {
    char* identifier;
    bool seen;
    int type;
    int reg;
    int literal;
    
} PsResult;

TkToken* PsGetState(void);
void PsRestoreState(TkToken* state);
void PsParseError(char* msg);
TkToken* PsCheckToken(void);
void PsAdvanceToken(void);
bool PsEatToken(TkTokenType type);

PsResult PsExpression(void);
PsResult PsEqualityExpression(void);
PsResult PsRelationalExpression(void);
PsResult PsShiftExpression(void);
PsResult PsAdditiveExpression(void);
PsResult PsMultiplicativeExpression(void);
PsResult PsCastExpression(void);
PsResult PsPrimaryExpression(void);

#endif /* parser_h */
