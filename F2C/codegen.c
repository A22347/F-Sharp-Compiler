//
//  codegen.c
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall.
//

#include "codegen.h"
#include "parser.h"
#include "typetable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int CgRegReferenceCounters[MAX_INTERNAL_REGS];

FILE* CgOutFile;

void CgDeinitialise() {
    fclose(CgOutFile);
}

void CgInitialise() {
    CgOutFile = fopen("/Users/alex_boxall21/Desktop/out.txt", "w");
}

int CgAllocateRegister() {
    for (int i = 0; i < MAX_INTERNAL_REGS; ++i) {
        if (CgRegReferenceCounters[i] == 0) {
            CgRegReferenceCounters[i] = 1;
            return i;
        }
    }
    printf("OUT OF REGS\n");
    exit(2);
    return -1;
}

void CgEmit(const char* format, ...) {
    char instr[256];
    memset(instr, 0, 256);
    
    va_list ap;
    va_start(ap, format);
    
    int i = 0;
    while (format[i]) {
        char c = format[i++];
        if (c == '%') {
            c = format[i++];
            if (c == '%') strcat(instr, "%");
            else if (c == 'c') sprintf(instr + strlen(instr), "%c", va_arg(ap, int));
            else if (c == 'd') sprintf(instr + strlen(instr), "%d", va_arg(ap, int));
            else if (c == 'X') sprintf(instr + strlen(instr), "%X", va_arg(ap, int));
            else if (c == 'x') sprintf(instr + strlen(instr), "%x", va_arg(ap, int));
            else if (c == 's') sprintf(instr + strlen(instr), "%s", va_arg(ap, char*));
            else if (c == 'r') {
                PsResult res = va_arg(ap, PsResult);
                
                if (res.identifier && res.reg == -2) {
                    char* type = PsGetIdentifierType(res.identifier);
                    if (!type) {
                        char x[512];
                        sprintf(x, "undeclared identifier '%s'", res.identifier);
                        PsParseError(x);
                    }
                    sprintf(instr + strlen(instr), "[%s]", res.identifier);
                } else if (res.reg == -1) {
                    sprintf(instr + strlen(instr), "%d", res.literal);
                } else {
                    sprintf(instr + strlen(instr), "R%d", res.reg);
                }
            }

        } else {
            sprintf(instr + strlen(instr), "%c", c);
        }
    }
    
    puts(instr);
    va_end(ap);
    
    fprintf(CgOutFile, "%s\n", instr);
}
