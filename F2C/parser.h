//
//  parser.h
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall. 
//

#ifndef parser_h
#define parser_h

#include <stdbool.h>

#include "tokeniser.h"

typedef struct PsResult {
    char* identifier;
    char* type;
    bool seen;
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
