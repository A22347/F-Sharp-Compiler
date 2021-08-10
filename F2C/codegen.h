//
//  codegen.h
//  F2C
//
//  Created by Alex Boxall on 1/8/21.
//  Copyright © 2021 Alex Boxall.
//

#ifndef codegen_h
#define codegen_h

#define MAX_INTERNAL_REGS 100

int CgAllocateRegister(void);
void CgEmit(const char* format, ...);
void CgInitialise(void);
void CgDeinitialise(void);
void CgEmitLabel(int);

#endif
