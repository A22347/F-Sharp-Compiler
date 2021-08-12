//
//  main.c
//  F2C
//
//  Created by Alex Boxall on 29/7/21.
//  Copyright © 2021 Alex Boxall.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include "tokeniser.h"
#include "peephole.h"
#include "parser.h"
#include "codegen.h"
#include "regreuse.h"
#include "typetable.h"

int main(int argc, char **argv)
{
    TkInitTokenList();
    PsInitTypeTable();
    CgInitialise();
    
    // 2 * 15 + hello / ((4 - 18) * 3 - 1) * 6 + 1 + world
    
    char buffer[512];
    FILE* f = fopen("/Users/alex_boxall21/Desktop/Programming/Banana Apps/F2C/F2C/IN.TXT", "r");
    if (!f) return -2;
    while(fgets(buffer, 511, f)) {
        TkTokenise(buffer);
    }
    fclose(f);

    TkAddToken("", TOKEN_EOF);
    TkAddToken("", TOKEN_NULL);
    TkAddToken("", TOKEN_NULL);
    PsEatToken(TOKEN_NULL);
        
    while (PsCheckToken()->type != TOKEN_EOF) {
        PsExpressionGroup();
        printf("---------------\n");
    }
    printf("\n");
    
    CgDeinitialise();
        
    OpPerformRegisterReuse("/Users/alex_boxall21/Desktop/out.txt", "/Users/alex_boxall21/Desktop/out3.txt");
    
    bool optimised = OpPerformPeephole("/Users/alex_boxall21/Desktop/out3.txt", "/users/alex_boxall21/Desktop/out2.txt", true);
    remove("/Users/alex_boxall21/Desktop/out3.txt");

    while (optimised) {
        optimised = OpPerformPeephole("/Users/alex_boxall21/Desktop/out2.txt", "/users/alex_boxall21/Desktop/out3.txt", false);
        optimised = OpPerformPeephole("/Users/alex_boxall21/Desktop/out3.txt", "/users/alex_boxall21/Desktop/out2.txt", false);
        remove("/Users/alex_boxall21/Desktop/out3.txt");
    }
    
    
    OpPerformPeephole("/Users/alex_boxall21/Desktop/out2.txt",
                      "/Users/alex_boxall21/Desktop/Programming/Banana Apps/F2C/F2C/OUT.TXT", false);
    
    // TODO: when you convert to actual registers, remeber to do movzx after any setXX instruction

    return 0;
}


/*
 a <- 12345
 b <- 9 + (-a - 1)
 c <- (16 & !!c) | (e ^ 555)
 d <- !c - ~a + 1
 v2 <- 4 + ~(variable <- (v8 * 1920 + hello / ((4 - v3) * 3 - v9) * 81 + v5 + 4 - 89 + 987 - 46 + world))
 */
