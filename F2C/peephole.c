//
//  peephole.c
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall.
//

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "peephole.h"

#define READ_NEXT_LINE if (!fgets(line, 127, in)) return false;
#define CHECK_LINE_MATCHES if (n != strlen(line)) return false;

#define MULTIPLY_3 fprintf(out, "lea R%d, [R%d * 2 + R%d]\n", regn, regn, regn);
#define MULTIPLY_5 fprintf(out, "lea R%d, [R%d * 4 + R%d]\n", regn, regn, regn);
#define MULTIPLY_9 fprintf(out, "lea R%d, [R%d * 8 + R%d]\n", regn, regn, regn);
#define MULTIPLY_DO_SHIFT if (shifts) fprintf(out, "shl R%d, %d\n", regn, shifts);

bool OpPeepDoubleNegation(char* line, FILE* in, FILE* out) {
    int regn, regn2, ogRegn;
    int n = 0;
    sscanf(line, "test R%d, R%d\n%n", &regn, &regn2, &n);
    ogRegn = regn;
    if (regn != regn2) return false;
    CHECK_LINE_MATCHES; READ_NEXT_LINE;
    sscanf(line, "setz R%d\n%n", &regn, &n);
    if (regn != ogRegn) return false;
    CHECK_LINE_MATCHES; READ_NEXT_LINE;
    sscanf(line, "test R%d, R%d\n%n", &regn, &regn2, &n);
    if (regn != ogRegn || regn2 != ogRegn) return false;
    CHECK_LINE_MATCHES; READ_NEXT_LINE;
    sscanf(line, "setz R%d\n%n", &regn, &n);
    if (regn != ogRegn) return false;
    CHECK_LINE_MATCHES;

    fprintf(out, "test R%d, R%d \t \nsetnz R%d \t \n", regn, regn, regn);
    
    return true;
}

bool OpPeepFastMultiplyDivide(char* line, FILE* in, FILE* out) {
    int regn;
    int n = 0;
    int val = 0;
    char oper[16];
    sscanf(line, "%s R%d, %d\n%n", oper, &regn, &val, &n);
    CHECK_LINE_MATCHES;
    if (!strcmp(oper, "div") && val != 0) {
        bool negated = false;
        if (val < 0) {
            val = -val;
            negated = true;
        }
        if (val == 1) {
            if (negated) fprintf(out, "neg R%d\n", regn);
            return true;
        }
        if (((val & (val - 1)) == 0)) {
            if (negated) fprintf(out, "neg R%d\n", regn);
            fprintf(out, "shr R%d, %d\n", regn, __builtin_ctz(val));
            return true;
        }
    }
    if (!strcmp(oper, "mod") && val != 0) {
        if (val == 1) {
            return true;
        }
        if (((val & (val - 1)) == 0)) {
            fprintf(out, "and R%d, %d\n", regn, val - 1);
            return true;
        }
    }
    if (!strcmp(oper, "mul") && val != 0) {
        bool negated = false;
        if (val < 0) {
            val = -val;
            negated = true;
        }
        if (val == 1) {
            if (negated) fprintf(out, "neg R%d\n", regn);
            return true;
        }
        if (((val & (val - 1)) == 0)) {
            if (negated) fprintf(out, "neg R%d\n", regn);
            fprintf(out, "shl R%d, %d\n", regn, __builtin_ctz(val));
            return true;
        }

        int xval = val;
        int shifts = 0;
        while (!(xval & 1)) {
            shifts++;
            xval >>= 1;
        }
        
        if (xval == 3 || xval == 5 || xval == 9) {
            if (xval == 3) MULTIPLY_3;
            if (xval == 5) MULTIPLY_5;
            if (xval == 9) MULTIPLY_9;
            MULTIPLY_DO_SHIFT;
            return true;
            
        } else if (xval == 15) { MULTIPLY_3; MULTIPLY_5; MULTIPLY_DO_SHIFT; return true;
        } else if (xval == 25) { MULTIPLY_5; MULTIPLY_5; MULTIPLY_DO_SHIFT; return true;
        } else if (xval == 27) { MULTIPLY_3; MULTIPLY_9; MULTIPLY_DO_SHIFT; return true;
        } else if (xval == 45) { MULTIPLY_5; MULTIPLY_9; MULTIPLY_DO_SHIFT; return true;
        } else if (xval == 81) { MULTIPLY_9; MULTIPLY_9; MULTIPLY_DO_SHIFT; return true;
        }
    }
    return false;
}

bool OpPeepIncDec(char* line, FILE* in, FILE* out) {
    int regn;
    int n = 0;
    char oper[16];
    sscanf(line, "%s R%d, 1\n%n", oper, &regn, &n);
    CHECK_LINE_MATCHES;
    if (!strcmp(oper, "add")) {
        fprintf(out, "inc R%d\n", regn);
        return true;
    }
    if (!strcmp(oper, "sub")) {
        fprintf(out, "dec R%d\n", regn);
        return true;
    }
    return false;
}

bool OpPeepMovUnaryBoolCombine(char* line, FILE* in, FILE* out){
    int n = 0;
    int regn, regn2;
    int ogRegn;
    int val;
    char oper[16];

    sscanf(line, "mov R%d, %d\n%n", &regn, &val, &n);
    ogRegn = regn;
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "test R%d, R%d\n%n", &regn, &regn2, &n);
    if (regn != ogRegn) return false;
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d\n%n", oper, &regn, &n);
    if (regn != ogRegn) return false;
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    
    bool retV = false;
    int newVal = 0;
    
    if (!strcmp(oper, "setz")) {newVal = !val; retV = true;}
    if (!strcmp(oper, "setnz")) {newVal = !!val; retV = true;}

    if (retV) fprintf(out, "mov R%d, %d\n", regn, newVal);
    return retV;
}

bool OpPeepMovUnaryCombine(char* line, FILE* in, FILE* out) {
    int n = 0;
    int regn, regn2;
    int val;
    char oper[16];

    sscanf(line, "mov R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d\n%n", oper, &regn2, &n);
    CHECK_LINE_MATCHES;
    
    if (regn != regn2) return false;
    
    bool retV = false;
    int newVal = 0;
    
    if (!strcmp(oper, "neg")) {newVal = -val; retV = true;}
    if (!strcmp(oper, "not")) {newVal = ~val; retV = true;}

    if (retV) fprintf(out, "mov R%d, %d\n", regn, newVal);
    return retV;
}

bool OpPeepMovArithCombine(char* line, FILE* in, FILE* out) {
    int n = 0;
    int regn, regn2;
    int val, val2;
    char oper[16];

    sscanf(line, "mov R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d, %d\n%n", oper, &regn2, &val2, &n);
    CHECK_LINE_MATCHES;
    
    if (regn != regn2) return false;
    
    bool retV = false;
    int newVal = 0;
    
    if (!strcmp(oper, "add")) {newVal = val + val2; retV = true;}
    if (!strcmp(oper, "sub")) {newVal = val - val2; retV = true;}
    if (!strcmp(oper, "and")) {newVal = val & val2; retV = true;}
    if (!strcmp(oper, "or"))  {newVal = val | val2; retV = true;}
    if (!strcmp(oper, "xor")) {newVal = val ^ val2; retV = true;}
    if (!strcmp(oper, "mul")) {newVal = val * val2; retV = true;}
    if (!strcmp(oper, "div")) {newVal = val / val2; retV = true;}
    if (!strcmp(oper, "mod")) {newVal = val % val2; retV = true;}
    if (!strcmp(oper, "shl")) {newVal = val << val2; retV = true;}
    if (!strcmp(oper, "shr")) {newVal = val >> val2; retV = true;}
    if (!strcmp(oper, "mov")) {newVal = val2; retV = true;}

    if (retV) fprintf(out, "mov R%d, %d\n", regn, newVal);
    return retV;
}

bool OpPeepArithCombine(char* line, FILE* in, FILE* out) {
    int n = 0;
    int regn, regn2;
    int val, val2;
    char oper[16];
    char oper2[16];

    sscanf(line, "%s R%d, %d\n%n", oper, &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d, %d\n%n", oper2, &regn2, &val2, &n);
    CHECK_LINE_MATCHES;
    
    if (regn != regn2) return false;
        
    if (!strcmp(oper, "add") && !strcmp(oper2, "sub")) {
        fprintf(out, "add R%d, %d\n", regn, val - val2);
        return true;
    }
    if (!strcmp(oper, "sub") && !strcmp(oper2, "add")) {
        fprintf(out, "sub R%d, %d\n", regn, val - val2);
        return true;
    }
    
    if (strcmp(oper, oper2)) return false;
    
    if (!strcmp(oper, "add")) {
        fprintf(out, "add R%d, %d\n", regn, val + val2);
        return true;
    }
    if (!strcmp(oper, "or")) {
        fprintf(out, "or R%d, %d\n", regn, val | val2);
        return true;
    }
    if (!strcmp(oper, "sub")) {
        fprintf(out, "sub R%d, %d\n", regn, val + val2);
        return true;
    }
    if (!strcmp(oper, "mul")) {
        fprintf(out, "mul R%d, %d\n", regn, val * val2);
        return true;
    }
    
    return false;
}

bool OpPeepConstantCompare(char* line, FILE* in, FILE* out) {
    int regn;
    int regn2;
    int regn3;
    int val;
    int val2;
    int n = 0;
    char oper[16];
    
    sscanf(line, "mov R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "cmp R%d, %d\n%n", &regn2, &val2, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d\n%n", oper, &regn3, &n);
    CHECK_LINE_MATCHES;
    
    if (regn != regn2 || regn != regn3) return false;
    if (!strcmp(oper, "setg")) {
        if (val > val2) fprintf(out, "mov R%d, 1\n", regn);
        else fprintf(out, "xor R%d, R%d\n", regn, regn);
        return true;
    }
    if (!strcmp(oper, "setge")) {
        if (val >= val2) fprintf(out, "mov R%d, 1\n", regn);
        else fprintf(out, "xor R%d, R%d\n", regn, regn);
        return true;
    }
    if (!strcmp(oper, "setl")) {
        if (val < val2) fprintf(out, "mov R%d, 1\n", regn);
        else fprintf(out, "xor R%d, R%d\n", regn, regn);
        return true;
    }
    if (!strcmp(oper, "setle")) {
        if (val <= val2) fprintf(out, "mov R%d, 1\n", regn);
        else fprintf(out, "xor R%d, R%d\n", regn, regn);
        return true;
    }
    
    return false;
}

bool OpPeepMoveToSelf(char* line, FILE* in, FILE* out) {
    int regn, regn2, n;
    sscanf(line, "mov R%d, R%d\n%n", &regn, &regn2, &n);
    CHECK_LINE_MATCHES;
    return regn == regn2;
}

bool OpPeepArithWithZero(char* line, FILE* in, FILE* out) {
    int regn;
    int n = 0;
    char oper[16];
    sscanf(line, "%s R%d, 0\n%n", oper, &regn, &n);
    CHECK_LINE_MATCHES;
    if (!strcmp(oper, "add") ||
        !strcmp(oper, "sub") ||
        !strcmp(oper, "xor") ||
        !strcmp(oper, "or")) {
        return true;
    }
    if (!strcmp(oper, "and") || !strcmp(oper, "mul")|| !strcmp(oper, "mov")) {
        fprintf(out, "xor R%d, R%d\n", regn, regn);
        return true;
    }
    return false;
}

bool OpPeepUnneededMemoryRead(char* line, FILE* in, FILE* out){
    int regn, regn2;
    int n = 0;
    char ident[150];
    char ident2[150];
    sscanf(line, "mov %s R%d\n%n", ident, &regn, &n);
    if (ident[strlen(ident)-1] == ',') ident[strlen(ident)-1] = 0;
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "mov R%d, %s\n%n", &regn2, ident2, &n);
    CHECK_LINE_MATCHES;
    
    if (!strcmp(ident, ident2) && ident[0] == '[' && ident[strlen(ident)-1] == ']') {
        fprintf(out, "mov %s, R%d\nmov R%d, R%d\n", ident, regn, regn2, regn);
        return true;
    }
    
    return false;
}

bool OpPeepIncAddCombine1(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "add R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "inc R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "add R%d, %d\n", regn, val + 1);
    return true;
}

bool OpPeepIncAddCombine2(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "inc R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "add R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "add R%d, %d\n", regn, val + 1);
    return true;
}

bool OpPeepDecAddCombine1(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "add R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "dec R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "add R%d, %d\n", regn, val - 1);
    return true;
}

bool OpPeepDecAddCombine2(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "dec R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "add R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "add R%d, %d\n", regn, val - 1);
    return true;
}

bool OpPeepIncSubCombine1(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "sub R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "inc R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "sub R%d, %d\n", regn, val - 1);
    return true;
}

bool OpPeepIncSubCombine2(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "inc R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "sub R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "sub R%d, %d\n", regn, val - 1);
    return true;
}

bool OpPeepDecSubCombine1(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "sub R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "dec R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "sub R%d, %d\n", regn, val + 1);
    return true;
}

bool OpPeepDecSubCombine2(char* line, FILE* in, FILE* out) {
    int regn, regn2, val, n;
    sscanf(line, "dec R%d\n%n", &regn2, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "sub R%d, %d\n%n", &regn, &val, &n);
    CHECK_LINE_MATCHES;
    if (regn != regn2) return false;
    fprintf(out, "sub R%d, %d\n", regn, val + 1);
    return true;
}

bool OpPeepSymmetricalArithWithMemoryPreloaded(char* line, FILE* in, FILE* out) {
    /*
     mov [b], R0
     mov R0, [a]
     cmp R0, [b]
     
     mov [b], R0
     cmp R0, [a]
     */
    
    
    /*
    mov [b], R0
    mov R0, [a]
    add R0, [b]
    
    mov [b], R0
    add R0, [a]
    */
    
    char ident1[140];
    char ident2[140];
    char ident3[140];
    char oper[16];
    int regn, regn2, regn3, n;
    
    /*
     mov [b], R0
     mov R0, [a]
     add R0, [b]
     */
    
    sscanf(line, "mov %s R%d\n%n", ident1, &regn, &n);
    if (ident1[strlen(ident1)-1] == ',') ident1[strlen(ident1)-1] = 0;
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "mov R%d, %s\n%n", &regn2, ident2, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d, %s\n%n", oper, &regn3, ident3, &n);
    CHECK_LINE_MATCHES;
    
    if (regn != regn2 || regn2 != regn3) return false;
    if (strcmp(ident1, ident3)) return false;
    if (!strcmp(ident1, ident2)) return false;
    if (ident1[0] != '[' || ident2[0] != '[' || ident3[0] != '[') return false;
    if (ident1[strlen(ident1)-1] != ']' || ident2[strlen(ident2)-1] != ']' || ident3[strlen(ident3)-1] != ']') return false;
    
    if (!strcmp(oper, "add") || !strcmp(oper, "mul")) {
        fprintf(out, "mov %s, R%d\n%s R%d, %s\n", ident1, regn, oper, regn, ident2);
        return true;
    }
    if (!strcmp(oper, "cmp")) {
        READ_NEXT_LINE;
        char oper2[16];
        int regn4;
        sscanf(line, "%s R%d\n%n", oper2, &regn4, &n);
        CHECK_LINE_MATCHES;
        if ((regn4 == regn) && (!strcmp(oper2, "setz") || !strcmp(oper2, "setnz"))) {
            fprintf(out, "mov %s, R%d\n%s R%d, %s\n%s R%d\n", ident1, regn, oper, regn, ident2, oper2, regn4);
            return true;
        }
        if ((regn4 == regn) && !strcmp(oper2, "setg")) {
            fprintf(out, "mov %s, R%d\n%s R%d, %s\nsetl R%d\n", ident1, regn, oper, regn, ident2, regn4);
            return true;
        }
        if ((regn4 == regn) && !strcmp(oper2, "setl")) {
            fprintf(out, "mov %s, R%d\n%s R%d, %s\nsetg R%d\n", ident1, regn, oper, regn, ident2, regn4);
            return true;
        }
        if ((regn4 == regn) && !strcmp(oper2, "setge")) {
            fprintf(out, "mov %s, R%d\n%s R%d, %s\nsetle R%d\n", ident1, regn, oper, regn, ident2, regn4);
            return true;
        }
        if ((regn4 == regn) && !strcmp(oper2, "setle")) {
            fprintf(out, "mov %s, R%d\n%s R%d, %s\nsetge R%d\n", ident1, regn, oper, regn, ident2, regn4);
            return true;
        }
    }
    return false;
}


bool OpPeepImmAssignThenUse(char* line, FILE* in, FILE* out) {
    int n = 0;
    int regn;
    char ident1[256];
    char ident2[256];
    int64_t imm;

    sscanf(line, "mov %s %lld\n%n", ident1, &imm, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "mov R%d, %s\n%n", &regn, ident2, &n);
    CHECK_LINE_MATCHES;
    if (ident1[strlen(ident1)-1] == ',') ident1[strlen(ident1)-1] = 0;
    if (strcmp(ident1, ident2)) return false;
        
    fprintf(out, "mov R%d, %lld\nmov %s, R%d\n", regn, imm, ident1, regn);
    return true;
}

bool OpPeepInvertedComparisions(char* line, FILE* in, FILE* out) {
    char oper[16];
    char oper2[16];
    int n, regn1, regn2, regn3, regn4;
    sscanf(line, "%s R%d\n%n", oper, &regn1, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "test R%d, R%d\n%n", &regn2, &regn3, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d\n%n", oper2, &regn4, &n);
    CHECK_LINE_MATCHES;
    
    if (regn1 != regn2 || regn1 != regn3 || regn1 != regn4) return false;
    
    if (!strcmp(oper2, "setz")) {
        if (!strcmp(oper, "setg")) {
            fprintf(out, "setle R%d\n", regn1);
            return true;
        }
        if (!strcmp(oper, "setl")) {
            fprintf(out, "setge R%d\n", regn1);
            return true;
        }
        if (!strcmp(oper, "setge")) {
            fprintf(out, "setl R%d\n", regn1);
            return true;
        }
        if (!strcmp(oper, "setle")) {
            fprintf(out, "setg R%d\n", regn1);
            return true;
        }
    } else if (!strcmp(oper2, "setnz")) {
        fprintf(out, "%s R%d\n", oper, regn1);
        return true;
    }

    return false;
}

bool OpPeepNotNegDecInc(char* line, FILE* in, FILE* out) {
    int n = 0;
    int regn, regn2;
    char oper[16];
    char oper2[16];

    sscanf(line, "%s R%d\n%n", oper, &regn, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s R%d\n%n", oper2, &regn2, &n);
    CHECK_LINE_MATCHES;
    
    if (regn != regn2) return false;
    
    if (!strcmp(oper, "not") && !strcmp(oper2, "neg")) {
        fprintf(out, "inc R%d\n", regn);
        return true;
    }
    
    if (!strcmp(oper, "neg") && !strcmp(oper2, "not")) {
        fprintf(out, "dec R%d\n", regn);
        return true;
    }
    
    if (!strcmp(oper, "neg") && !strcmp(oper2, "dec")) {
        fprintf(out, "not R%d\n", regn);
        return true;
    }
    
    if (!strcmp(oper, "not") && !strcmp(oper2, "inc")) {
        fprintf(out, "neg R%d\n", regn);
        return true;
    }
    
    return false;
}

bool OpMoveOverwritesMove(char* line, FILE* in, FILE* out) {
    int n = 0;
    char ident[256];
    char ident2[256];
    int int1, int2;
    
    sscanf(line, "mov %s %d\n%n", ident, &int1, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "mov %s %d\n%n", ident2, &int2, &n);
    CHECK_LINE_MATCHES;
    
    if (!strcmp(ident, ident2) && ident[strlen(ident)-1] == ',') {
        fprintf(out, "mov %s %d\n", ident2, int2);
        return true;
    }
    return false;
}

bool OpCompareThenJump(char* line, FILE* in, FILE* out) {
    int n = 0;
    int regn, regn2, regn3, labl;
    char oper[16];
    char oper2[16];

    sscanf(line, "%s R%d\n%n", oper, &regn, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "test R%d, R%d\n%n", &regn2, &regn3, &n);
    CHECK_LINE_MATCHES;
    READ_NEXT_LINE;
    sscanf(line, "%s .l%d\n%n", oper2, &labl, &n);
    CHECK_LINE_MATCHES;
    
    if (regn != regn2 || regn != regn3) return false;
    
    if (!strcmp(oper, "setg") && !strcmp(oper2, "jz")) {
        fprintf(out, "%s R%d\njng .l%d\n", oper, regn, labl);
        return true;
    }
    if (!strcmp(oper, "setge") && !strcmp(oper2, "jz")) {
        fprintf(out, "%s R%d\njl .l%d\n", oper, regn, labl);
        return true;
    }
    if (!strcmp(oper, "setl") && !strcmp(oper2, "jz")) {
        fprintf(out, "%s R%d\njnl .l%d\n", oper, regn, labl);
        return true;
    }
    if (!strcmp(oper, "setle") && !strcmp(oper2, "jz")) {
        fprintf(out, "%s R%d\njg .l%d\n", oper, regn, labl);
    return true;
    }
    
    return false;
}

int OpPerformPeephole(const char* infile, const char* outfile, bool firstTime) {
    FILE* in = fopen(infile, "r");
    FILE* out = fopen(outfile, "w");
    
    char line[128];
    char linecopy[128];

    bool didPeephole = false;
    
    long pos = 0;
    while (fgets(line, 127, in)) {
    retry:;
        pos = ftell(in);
        
        strcpy(linecopy, line);
        if (!OpPeepArithWithZero(line, in, out)) fseek(in, pos, SEEK_SET); else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepIncDec(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepFastMultiplyDivide(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepDoubleNegation(linecopy, in, out)) fseek(in, pos, SEEK_SET); else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepConstantCompare(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepArithCombine(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepMovArithCombine(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepMovUnaryCombine(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepMovUnaryBoolCombine(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepUnneededMemoryRead(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepNotNegDecInc(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepMoveToSelf(line, in, out)) fseek(in, pos, SEEK_SET); else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepIncAddCombine1(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepIncAddCombine2(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepDecAddCombine1(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepDecAddCombine2(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepIncSubCombine1(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepIncSubCombine2(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepDecSubCombine1(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepDecSubCombine2(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepSymmetricalArithWithMemoryPreloaded(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepImmAssignThenUse(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpPeepInvertedComparisions(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpCompareThenJump(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        strcpy(linecopy, line);
        if (!OpMoveOverwritesMove(linecopy, in, out)) {fseek(in, pos, SEEK_SET);} else {didPeephole=true;continue;}
        
        //TODO: combined multiplication/shift with addition/subtraction in LEA

        fseek(in, pos, SEEK_SET);
        fprintf(out, "%s", line);
    }
    
    fclose(in);
    fclose(out);
    
    return didPeephole;
}
