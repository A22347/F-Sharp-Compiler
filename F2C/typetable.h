//
//  typetable.h
//  
//
//  Created by Alex Boxall on 9/8/21.
//

#ifndef typetable_h
#define typetable_h

#include <stdbool.h>

void PsInitTypeTable(void);
void PsAllocateType(char* identifier, char* type, bool isMutable);
char* PsGetIdentifierType(char* identifier);
bool PsIsMutable(char* identifier);
void PsIncreaseScope(void);
void PsDecreaseScope(void);
int PsGetIdentifierScope(char* identifier);

#endif /* typetable_h */
