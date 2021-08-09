//
//  typetable.h
//  
//
//  Created by Alex Boxall on 9/8/21.
//

#ifndef typetable_h
#define typetable_h

void PsInitTypeTable(void);
void PsAllocateType(char* identifier, char* type);
char* PsGetIdentifierType(char* identifier);

#endif /* typetable_h */
