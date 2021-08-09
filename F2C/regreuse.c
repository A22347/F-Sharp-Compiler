//
//  regreuse.c
//  F2C
//
//  Created by Alex Boxall on 3/8/21.
//  Copyright © 2021 Alex Boxall. All rights reserved.
//

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "regreuse.h"
#include "codegen.h"

int firstRegUsageLines[MAX_INTERNAL_REGS];
int lastRegUsageLines[MAX_INTERNAL_REGS];
int oldNewRegMappings[MAX_INTERNAL_REGS];

void OpPerformRegisterReuse(const char* infile, const char* outfile) {
    memset(lastRegUsageLines, 0, sizeof(lastRegUsageLines));
    memset(firstRegUsageLines, 0xFF, sizeof(lastRegUsageLines));
    memset(oldNewRegMappings, 0xFF, sizeof(lastRegUsageLines));

    FILE* in = fopen(infile, "r");
    FILE* out = fopen(outfile, "w");
    
    char line[128];
    
    int lineNo = 0;
    while (fgets(line, 127, in)) {
        for (int i = 0; line[i]; ++i) {
            if (line[i] == 'R') {
                int regn, n;
                sscanf(line + i + 1, "%d%n", &regn, &n);
                lastRegUsageLines[regn] = lineNo;
                if (firstRegUsageLines[regn] == -1) {
                    firstRegUsageLines[regn] = lineNo;
                }
                i += n + 1;
            }
        }
        
        lineNo++;
    }
       
    // for each register (from 1 onwards)
    //      work out if any other register is completely done before this one gets used
    //      if so remap the register, in the table and free this register by setting firstRegUsage to -1, and lastREgUsage to 0
    //      otherwise, map the registser to itself
    
    oldNewRegMappings[0] = 0;       // R0 -> R0
    for (int i = 1; i < MAX_INTERNAL_REGS; ++i) {
        oldNewRegMappings[i] = i;       //Rx -> Rx
        if (firstRegUsageLines[i] == -1) continue;

        for (int j = 0; j < MAX_INTERNAL_REGS; ++j) {
            if (i == j) continue;
            
            if (firstRegUsageLines[j] == -1 || (lastRegUsageLines[j] < firstRegUsageLines[i])) {
                oldNewRegMappings[i] = j;
                firstRegUsageLines[j] = firstRegUsageLines[i];
                lastRegUsageLines[j] = lastRegUsageLines[i];
                firstRegUsageLines[i] = -1;
                break;
            }
        }
    }
    
    rewind(in);
    
    while (fgets(line, 127, in)) {
        for (int i = 0; line[i]; ++i) {
            if (line[i] == 'R') {
                int regn, n;
                sscanf(line + i + 1, "%d%n", &regn, &n);
                regn = oldNewRegMappings[regn];
                fprintf(out, "R%d", regn);
                i += n;
            } else {
                fputc(line[i], out);
            }
        }
        
        lineNo++;
    }
    
    fclose(in);
    fclose(out);
}
