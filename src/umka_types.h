#ifndef UMKA_TYPES_H_INCLUDED
#define UMKA_TYPES_H_INCLUDED

#include "umka_common.h"
#include "umka_lexer.h"


typedef enum
{
    TYPE_NONE,
    TYPE_FORWARD,
    TYPE_VOID,
    TYPE_NULL,          // Base type for 'null' constant only
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_REAL32,
    TYPE_REAL,
    TYPE_PTR,
    TYPE_ARRAY,
    TYPE_DYNARRAY,
    TYPE_STR,           // Pointer of a special kind that admits assignment of string literals, concatenation and comparison by content
    TYPE_STRUCT,
    TYPE_INTERFACE,
    TYPE_FIBER,
    TYPE_FN
} TypeKind;


typedef union
{
    int64_t intVal;
    uint64_t uintVal;
    int64_t ptrVal;
    double realVal;
} Const;


typedef struct
{
    IdentName name;
    unsigned int hash;
    struct tagType *type;
    int offset;
} Field;


typedef struct
{
    IdentName name;
    unsigned int hash;
    struct tagType *type;
    Const defaultVal;
} Param;


typedef struct
{
    int numParams, numDefaultParams, numResults;
    bool method;
    int offsetFromSelf;                     // For interface methods
    Param *param[MAX_PARAMS];
    struct tagType *resultType[MAX_RESULTS];
} Signature;


typedef struct tagType
{
    TypeKind kind;
    int block;
    struct tagType *base;                   // For pointers and arrays
    int numItems;                           // For arrays, structures and interfaces
    bool weak;                              // For pointers
    struct tagIdent *typeIdent;             // For types that have identifiers
    union
    {
        Field *field[MAX_FIELDS];           // For structures and interfaces
        Signature sig;                      // For functions, including methods
    };
    struct tagType *next;
} Type;


typedef struct
{
    Type *first, *last;
    Error *error;
} Types;


void typeInit(Types *types, Error *error);
void typeFree(Types *types, int startBlock /* < 0 to free in all blocks*/);

Type *typeAdd       (Types *types, Blocks *blocks, TypeKind kind);
void typeDeepCopy   (Type *dest, Type *src);
Type *typeAddPtrTo  (Types *types, Blocks *blocks, Type *type);

int typeSizeNoCheck (Type *type);
int typeSize        (Types *types, Type *type);


static inline bool typeKindInteger(TypeKind typeKind)
{
    return typeKind == TYPE_INT8  || typeKind == TYPE_INT16  || typeKind == TYPE_INT32  || typeKind == TYPE_INT ||
           typeKind == TYPE_UINT8 || typeKind == TYPE_UINT16 || typeKind == TYPE_UINT32 || typeKind == TYPE_UINT;
}


static inline bool typeInteger(Type *type)
{
    return typeKindInteger(type->kind);
}


static inline bool typeOrdinal(Type *type)
{
    return typeInteger(type) || type->kind == TYPE_CHAR;
}


static inline bool typeCastable(Type *type)
{
    return typeOrdinal(type) || type->kind == TYPE_BOOL;
}


static inline bool typeKindReal(TypeKind typeKind)
{
    return typeKind == TYPE_REAL32 || typeKind == TYPE_REAL;
}


static inline bool typeReal(Type *type)
{
    return typeKindReal(type->kind);
}


static inline bool typeStructured(Type *type)
{
    return type->kind == TYPE_ARRAY  || type->kind == TYPE_DYNARRAY  ||
           type->kind == TYPE_STRUCT || type->kind == TYPE_INTERFACE || type->kind == TYPE_FIBER;
}


static inline bool typeKindGarbageCollected(TypeKind typeKind)
{
    return typeKind == TYPE_PTR    || typeKind == TYPE_STR       || typeKind == TYPE_ARRAY  || typeKind == TYPE_DYNARRAY ||
           typeKind == TYPE_STRUCT || typeKind == TYPE_INTERFACE || typeKind == TYPE_FIBER;
}


bool typeGarbageCollected(Type *type);


static inline bool typeCharArrayPtr(Type *type)
{
    return type->kind == TYPE_PTR && type->base->kind == TYPE_ARRAY && type->base->base->kind == TYPE_CHAR;
}


static inline bool typeFiberFunc(Type *type)
{
    return type->kind                            == TYPE_FN    &&
           type->sig.numParams                   == 2          &&
           type->sig.numDefaultParams            == 0          &&
           type->sig.param[0]->type->kind        == TYPE_PTR   &&
           type->sig.param[0]->type->base->kind  == TYPE_FIBER &&
           type->sig.param[1]->type->kind        == TYPE_PTR   &&
           type->sig.param[1]->type->base->kind  != TYPE_VOID  &&
           type->sig.numResults                  == 1          &&
           type->sig.resultType[0]->kind         == TYPE_VOID  &&
           !type->sig.method;
}


bool typeEquivalent         (Type *left, Type *right);
bool typeAssertEquivalent   (Types *types, Type *left, Type *right);
bool typeCompatible         (Type *left, Type *right, bool symmetric);
bool typeAssertCompatible   (Types *types, Type *left, Type *right, bool symmetric);

bool typeValidOperator      (Type *type, TokenKind op);
bool typeAssertValidOperator(Types *types, Type *type, TokenKind op);

bool typeAssertForwardResolved(Types *types);

bool typeOverflow           (TypeKind typeKind, Const val);

Field *typeFindField        (Type *structType, char *name);
Field *typeAssertFindField  (Types *types, Type *structType, char *name);
Field *typeAddField         (Types *types, Type *structType, Type *fieldType, char *name);

Param *typeFindParam        (Signature *sig, char *name);
Param *typeAddParam         (Types *types, Signature *sig, Type *type, char *name);

int typeParamSizeUpTo   (Types *types, Signature *sig, int index);
int typeParamSizeTotal  (Types *types, Signature *sig);

char *typeKindSpelling  (TypeKind kind);
char *typeSpelling      (Type *type, char *buf);


#endif // UMKA_TYPES_H_INCLUDED
