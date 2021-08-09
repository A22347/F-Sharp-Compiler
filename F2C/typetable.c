//
//  typetable.c
//  
//
//  Created by Alex Boxall on 9/8/21.
//

#include "typetable.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PsDeclaredType {
    char* identifier;
    char* type;
    struct PsDeclaredType* next;
    
} PsDeclaredType;

PsDeclaredType* typeHead = 0;
PsDeclaredType* typeTail = 0;

void PsInitTypeTable() {
    typeHead = malloc(sizeof(PsDeclaredType));
    typeTail = typeHead;
    
    typeHead->next = NULL;
    typeHead->type = NULL;
}

void PsAllocateType(char* identifier, char* type) {
    if (PsGetIdentifierType(identifier)) {
        char* x = malloc(strlen(identifier) + 60);
        sprintf(x, "redeclaration of identifier '%s'", identifier);
        PsParseError(x);
        return;
    }
    
    char* allocIdent = malloc(1 + strlen(identifier));
    strcpy(allocIdent, identifier);
      
    char* allocType = malloc(1 + strlen(type));
    strcpy(allocType, type);
    
    typeTail->next = malloc(sizeof(PsDeclaredType));
    typeTail = typeTail->next;
    typeTail->identifier = allocIdent;
    typeTail->next = NULL;
    typeTail->type = allocType;
}

char* PsGetIdentifierType(char* identifier) {
    PsDeclaredType* curr = typeHead;
    
    while (curr) {
        if (curr->type != NULL && curr->identifier != NULL) {
            if (!strcmp(curr->identifier, identifier)) {
                return curr->type;
            }
        }
        
        curr = curr->next;
    }
    
    return NULL;
}
