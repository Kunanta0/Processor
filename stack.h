#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED
#include <math.h>
#include <malloc.h>

#ifndef NO_CANARY
const double CanaryLeft = -675.249;
const double CanaryRight = 52.267;
#endif

typedef struct
{
    const char* file;
    int line;
    const char* func;
} log_ctx_t;

#include "log.h"
#include <time.h>
#include <string.h>

#define StackPush(stk, val) Stack_Push((stk), (val), &(log_ctx_t){__FILE__, __LINE__, __func__})
#define StackPop(stk, last) Stack_Pop((stk), (last), &(log_ctx_t){__FILE__, __LINE__, __func__})
#define StackCtor(stk, capacity) Stack_Ctor((stk), (capacity), &(log_ctx_t){__FILE__, __LINE__, __func__})
#define StackDtor(stk) Stack_Dtor((stk), &(log_ctx_t){__FILE__, __LINE__, __func__})

#define PRINT_STACK(stk, specificator, delta, poizon, state)\
{\
    if (state != STACK_IS_NULL)\
    {\
        printf("stack with address %p: ", stk);\
        if (state != DATA_IS_NULL && state != CALL_CONSTRUCTOR_FOR_EXISTING_STACK)\
        {\
            size_t size = stk->size;\
            printf("size = %lld, capacity = %lld\ndata with address %p\n{\n", (stk)->size, ((stk)->capacity) - delta, (stk)->data);\
            if (state == STACK_OVERFLOW) size = stk->capacity - delta;\
            for (size_t i = 0; i < (stk)->size; ++i)\
            {\
                printf("\tdata[%lld] = %"#specificator"\n", i, (stk)->data[i]);\
            }\
            if (poizon)\
            for (size_t i = (stk)->size; i < (stk)->capacity - delta; ++i)\
            {\
                printf("*\tdata[%lld] = %"#specificator" (POIZON)\n", i, (stk)->data[i]);\
            }\
            printf("}\n");\
        }\
    }\
}

#define PRINT_STACK_DEBUG(stk, specificator, delta, poizon, state)\
{\
    if (state != STACK_IS_NULL)\
    {\
        FILE* log = fopen("log.txt", "a");\
        fprintf(log, "%p\n", (stk));\
        if (state == DATA_IS_NULL || state == CALL_CONSTRUCTOR_FOR_EXISTING_STACK) fclose(log);\
        else {\
            size_t size = stk->size;\
            fprintf(log, "size = %lld, capacity = %lld\ndata with address %p\n{\n", (stk)->size, ((stk)->capacity) - delta, (stk)->data);\
            if (state == STACK_OVERFLOW) size = stk->capacity - delta;\
            for (size_t i = 0; i < size; ++i)\
            {\
                fprintf(log, "\tdata[%lld] = %"#specificator"\n", i, (stk)->data[i]);\
            }\
            if (poizon)\
            for (size_t i = size; i < (stk)->capacity - delta; ++i)\
            {\
                fprintf(log, "*\tdata[%lld] = %"#specificator" (POIZON)\n", i, (stk)->data[i]);\
            }\
            fprintf(log, "}\n");\
            fclose(log);\
        }\
    }\
}

enum ERRORS
{
    OK = 0,
    CALL_CONSTRUCTOR_FOR_EXISTING_STACK =  1,
    STACK_OVERFLOW = 2,
    STACK_IS_NULL = 3,
    DATA_IS_NULL = 4,
    STACK_UNDERFLOW = 5,
    CANARY_DAMAGED = 6,
    NOT_A_STACK = 7
};

typedef struct
{
    StackElem_t* data;
    size_t size;
    size_t capacity;
} stack_t;

typedef struct
{
    stack_t* data;
    size_t size;
    size_t capacity;
} mini_stack;

int StackVerify(stack_t*, const log_ctx_t* ctx);
int CheckNull(stack_t*, const log_ctx_t* ctx);
int Stack_Ctor(stack_t*, size_t, log_ctx_t* ctx);
void Stack_Push(stack_t*, StackElem_t, log_ctx_t* ctx);
int Stack_Pop(stack_t*, StackElem_t*, log_ctx_t* ctx);
void Stack_Dtor(stack_t*, log_ctx_t* ctx);

int StackVerify(stack_t* stk, const log_ctx_t* ctx)
{
    FILE* log = fopen("log.txt", "a");

    #ifndef NO_POIZON
    int poizon = 1;
    #else
    int poizon = 0;
    #endif

    #ifdef NO_CANARY
    int delta = 0;
    #else
    int delta = 2;
    #endif

    int state = OK;
    if (stk == NULL) return STACK_IS_NULL;
    if (stk->data == NULL && stk->capacity != 0) return DATA_IS_NULL;
    if (stk->size > stk->capacity - delta) return STACK_OVERFLOW;
    #ifndef NO_CANARY
    if (stk->data[-1] != CanaryLeft || stk->data[stk->capacity - 2] != CanaryRight) state = CANARY_DAMAGED;
    #endif

    if (state != OK)
    {
        write_log(log, LOG_ERROR, "StackVerify returns ERROR CODE: %d\n", state);
        printf("ERROR: %d in file: %s, line: %d, function^ %s\n", state, ctx->file, ctx->line, ctx->func);
        PRINT_STACK_DEBUG(stk, d, delta, poizon, state);
        #ifdef DEBUG
        PRINT_STACK(stk, d, delta, poizon, state);
        #endif
        if (fclose(log) != 0) printf("Error of closing log file\n");
        return state;
    }
    #ifdef DEBUG
    write_log(log, LOG_INFO, "StackVerify returns: %d\n", OK);
    PRINT_STACK_DEBUG(stk, d, delta, poizon, state);
    PRINT_STACK(stk, d, delta, poizon, state);
    #endif
    if (fclose(log) != 0) printf("Error of closing log file\n");
    return OK;
}

int CheckNull(stack_t* stk, const log_ctx_t* ctx)
{
    #ifndef NO_POIZON
    int poizon = 1;
    #else
    int poizon = 0;
    #endif

    #ifdef NO_CANARY
    int delta = 0;
    #else
    int delta = 2;
    #endif

    FILE* log = fopen("log.txt", "a");
    int state = OK;

    if (stk == NULL) state = STACK_IS_NULL;

    if (stk->capacity == 0 && stk->size == 0 && stk->data == NULL)
    {
        if (fclose(log) != 0) printf("Error of closing log file\n");
        return OK;
    }
    else state = CALL_CONSTRUCTOR_FOR_EXISTING_STACK;

    write_log(log, LOG_ERROR, "CheckNull returns ERROR CODE: %d\n", state);
    printf("ERROR: %d in file: %s, line: %d, function: %s\n", state, ctx->file, ctx->line, ctx->func);
    PRINT_STACK_DEBUG(stk, lg, delta, poizon, state);
    #ifdef DEBUG
    PRINT_STACK(stk, d, delta, poizon, state);
    #endif

    if (fclose(log) != 0) printf("Error of closing log file\n");
    return state;
}

int Stack_Ctor(stack_t* stk, size_t capacity, log_ctx_t* ctx)
{
    if (capacity == 0) capacity = 1;
    #ifdef NO_CANARY
    int delta = 0;
    int gamma = 0;
    #else
    int delta = 2;
    int gamma = 1;
    #endif

    #ifndef NO_VERIFY
    int state = CheckNull(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return state;
    }
    #endif

    stk->capacity = capacity + delta;
    StackElem_t* ptr = (StackElem_t*)calloc(stk->capacity, sizeof(StackElem_t));

    #ifndef NO_VERIFY
    if (ptr == NULL)
    {
        printf("Error of creating stack with code: %d\n", DATA_IS_NULL);
        return DATA_IS_NULL;
    }
    #endif

    stk->data = ptr + gamma;

    #ifndef NO_CANARY
    stk->data[-1] = CanaryLeft;
    #endif

    #ifndef NO_POIZON
    for (size_t i = 0; i < capacity; ++i)
    {
        stk->data[i] = NAN;
    }
    #endif

    #ifndef NO_CANARY
    stk->data[capacity] = CanaryRight;
    #endif

    #ifndef NO_VERIFY
    state = StackVerify(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return state;
    }
    #endif

    return OK;
}

void Stack_Push(stack_t* stk, StackElem_t val, log_ctx_t* ctx)
{
    #ifndef NO_VERIFY
    int state = StackVerify(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return;
    }
    #endif

    #ifdef NO_CANARY
    int delta = 0;
    int gamma = 0;
    #else
    int delta = 2;
    int gamma = 1;
    #endif

    if (stk->size == stk->capacity - delta)
    {
        StackElem_t* ptr = (StackElem_t* )realloc(stk->data - gamma, sizeof(StackElem_t) * 2 * (stk->capacity - gamma));
        #ifndef NO_VERIFY
        if (ptr == NULL)
        {
            printf("Error of increasing data; element haven't pushed\n");
            return;
        }
        #endif
        stk->data = ptr + gamma;
        stk->capacity = 2 * (stk->capacity - gamma);

        #ifndef NO_POIZON
        for (size_t i = stk->size; i < stk->capacity; ++i) stk->data[i] = NAN;
        #endif

        #ifndef NO_CANARY
        stk->data[stk->capacity - 2] = CanaryRight;
        #endif

    }

    stk->data[stk->size++] = val;

    #ifndef NO_VERIFY
    state = StackVerify(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return;
    }
    #endif
}

int Stack_Pop(stack_t* stk, StackElem_t* last, log_ctx_t* ctx)
{
    FILE* log = fopen("log.txt", "a");

    #ifndef NO_POIZON
    int poizon = 1;
    #else
    int poizon = 0;
    #endif
    #ifdef NO_CANARY
    int delta = 0;
    int gamma = 0;
    #else
    int delta = 2;
    int gamma = 1;
    #endif

    #ifndef NO_VERIFY
    int state = StackVerify(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return state;
    }

    if (stk->size == 0)
    {
        write_log(log, LOG_WARNING, "StackVerify returns warning with code: %d\n", STACK_UNDERFLOW);
        printf("ERROR: %d in file: %s, line: %d, function^ %s\n", STACK_UNDERFLOW, ctx->file, ctx->line, ctx->func);
        PRINT_STACK_DEBUG(stk, d, delta, poizon, state);
        #ifdef DEBUG
        PRINT_STACK(stk, d, delta, poizon, state);
        #endif
        if (fclose(log) != 0) printf("Error of closing log file\n");
        return STACK_UNDERFLOW;
    }
    #endif

    *last = stk->data[--stk->size];
    size_t cap_two = (stk->capacity % 2 == 0) ? stk->capacity : stk->capacity - 1;
    if (2 * (stk->size + gamma) == cap_two && cap_two - delta != 0)
    {
        StackElem_t* ptr = (StackElem_t*)realloc(stk->data - gamma, sizeof(StackElem_t) * (stk->size + gamma));
        stk->data = ptr + gamma;
        stk->capacity = stk->size + delta;
    }

    #ifndef NO_POIZON
    for (size_t i = stk->size; i < stk->capacity - delta; ++i) stk->data[i] = NAN;
    #endif

    #ifndef NO_CANARY
    stk->data[stk->capacity - 2] = CanaryRight;
    #endif

    #ifndef NO_VERIFY
    state = StackVerify(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return state;
    }
    #endif

    return OK;
}

void Stack_Dtor(stack_t* stk, log_ctx_t* ctx)
{
    #ifndef NO_VERIFY
    int state = StackVerify(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return;
    }
    #endif

    #ifdef NO_CANARY
    int gamma = 0;
    #else
    int gamma = 1;
    #endif

    stk->data -= gamma;
    free(stk->data);
    stk->data = NULL;
    stk->capacity = 0;
    stk->size = 0;

    #ifndef NO_VERIFY
    state = CheckNull(stk, ctx);
    if (state != OK)
    {
        printf("Error: %d\n", state);
        return;
    }
    #endif
}

#endif // STACK_H_INCLUDED


//NO_CANARY убирает канарейки
//NO_POIZON убирает яд
//NO_VERIFY убирает верификацию стека
//capacity нечетный надо исправить
