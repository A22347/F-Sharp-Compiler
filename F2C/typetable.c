//
//  typetable.c
//  F2C
//
//  Created by Alex Boxall on 9/8/21.
//  Copyright © 2021 Alex Boxall.
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
    bool isMutable;
    int scope;
    
} PsDeclaredType;

PsDeclaredType* typeHead = 0;
PsDeclaredType* typeTail = 0;

int PsScopeLevel = 0;
void PsIncreaseScope() {
    PsScopeLevel++;
}

void PsDecreaseScope() {
    PsDeclaredType* curr = typeHead;

    while (curr) {
        if (curr->type != NULL && curr->identifier != NULL) {
            if (curr->scope >= PsScopeLevel) {
                curr->type = NULL;
                curr->identifier = NULL;
                //TODO: free strings?
                //TODO: actually delete from linked list
            }
        }
        
        curr = curr->next;
    }
    
    PsScopeLevel--;
}

void PsInitTypeTable() {
    PsScopeLevel = 0;
    typeHead = malloc(sizeof(PsDeclaredType));
    typeTail = typeHead;
    
    typeHead->next = NULL;
    typeHead->type = NULL;
}

void PsAllocateType(char* identifier, char* type, bool isMutable) {
    int scope = PsGetIdentifierScope(identifier);
    if (scope != -1) {
        char* x = malloc(strlen(identifier) + 60);
        if (PsScopeLevel > scope) {
            sprintf(x, "declaration of identifier '%s' shadows identifier in broader scope", identifier);
        } else {
            sprintf(x, "redeclaration of identifier '%s'", identifier);
        }
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
    typeTail->scope = PsScopeLevel;
    typeTail->isMutable = isMutable;
    typeTail->next = NULL;
    typeTail->type = allocType;
}

int PsGetIdentifierScope(char* identifier) {
    PsDeclaredType* curr = typeHead;
    
    while (curr) {
        if (curr->type != NULL && curr->identifier != NULL) {
            if (!strcmp(curr->identifier, identifier)) {
                return curr->scope;
            }
        }
        
        curr = curr->next;
    }
    
    return -1;
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

bool PsIsMutable(char* identifier) {
    PsDeclaredType* curr = typeHead;
    
    while (curr) {
        if (curr->type != NULL && curr->identifier != NULL) {
            if (!strcmp(curr->identifier, identifier)) {
                return curr->isMutable;
            }
        }
        
        curr = curr->next;
    }
    
    return false;
}
