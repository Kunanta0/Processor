#include <stdio.h>
//#define DEBUG
typedef int StackElem_t;
#define NO_VERIFY
#define NO_CANARY
//#define NO_POIZON
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stack.h"
#include "proc.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define BLUE "\033[34m"

/*enum
{
    HLT = 0,//
    PUSH = 1,//
    ADD = 2,//
    SUB = 3,//
    DIV = 4,//
    OUT = 5,//
    MUL = 6,//
    POP = 7,//
    PUSHr = 8,//
    JMP = 9,//
    JA = 10,//
    JAE = 11,//
    JB = 12,//
    JBE = 13,//
    JE = 14,//
    JNE = 15,//
    CALL = 16,
    RET = 17,
    IN = 18
};*/

typedef struct
{
    int* data;
    size_t length;
    int pc;
    int registr[4];
} Processor;

char* ReadFile(const char* name, size_t* length);
void CreateProcessor(Processor* code, char* buffer, size_t length);
size_t GetSize(const char* filename);
void Run(Processor* code);
void ProcessorDump(Processor* code);

void FillPush(stack_t* stk, Processor* code);
void FillAdd(stack_t* stk, Processor* code);
void FillSub(stack_t* stk, Processor* code);
void FillDiv(stack_t* stk, Processor* code);
void FillOut(stack_t* stk, Processor* code);
void FillMul(stack_t* stk, Processor* code);
void FillPop(stack_t* stk, Processor* code);
void FillPushr(stack_t* stk, Processor* code);
void FillJa(stack_t* stk, Processor* code);
void FillJae(stack_t* stk, Processor* code);
void FillJb(stack_t* stk, Processor* code);
void FillJbe(stack_t* stk, Processor* code);
void FillJe(stack_t* stk, Processor* code);
void FillJne(stack_t* stk, Processor* code);
void FillCall(stack_t* stk, Processor* code);
void FillRet(stack_t* stk, Processor* code);
void FillIn(stack_t* stk, Processor* code);

int main()
{
    size_t length = 0;
    char* buffer = ReadFile("code.txt", &length);

    Processor code = {};
    CreateProcessor(&code, buffer, length);
    Run(&code);

    free(buffer);
    free(code.data);
    getchar();
    return 0;
}

void FillPush(stack_t* stk, Processor* code)
{
    StackPush(stk, code->data[code->pc + 1]);
    code->pc += 2;
}
void FillAdd(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    code->pc += 1;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    StackPush(stk, value_1 + value_2);
}
void FillSub(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    code->pc += 1;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    StackPush(stk, value_2 - value_1);
}
void FillDiv(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    code->pc += 1;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    StackPush(stk, value_2 / value_1);
}
void FillOut(stack_t* stk, Processor* code)
{
    int answer = 0;
    code->pc += 1;
    StackPop(stk, &answer);
    printf("%d\n", answer);
}
void FillMul(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    code->pc += 1;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    StackPush(stk, value_1 * value_2);
}
void FillPop(stack_t* stk, Processor* code)
{
    StackPop(stk, &(code->registr[code->data[code->pc + 1]]));
    code->pc += 2;
}
void FillPushr(stack_t* stk, Processor* code)
{
    StackPush(stk, code->registr[code->data[code->pc + 1]]);
    code->pc += 2;
}
void FillJa(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    //printf("%d %d\n", value_1, value_2);
    if (value_1 > value_2) code->pc = code->data[code->pc + 1];
    else code->pc += 2;
}
void FillJae(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    //printf("%d %d\n", value_1, value_2);
    if (value_1 >= value_2) code->pc = code->data[code->pc + 1];
    else code->pc += 2;
}
void FillJb(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    //printf("%d %d\n", value_1, value_2);
    if (value_1 < value_2) code->pc = code->data[code->pc + 1];
    else code->pc += 2;
}
void FillJbe(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    //printf("%d %d\n", value_1, value_2);
    if (value_1 <= value_2) code->pc = code->data[code->pc + 1];
    else code->pc += 2;
}
void FillJe(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    //printf("%d %d\n", value_1, value_2);
    if (value_1 == value_2) code->pc = code->data[code->pc + 1];
    else code->pc += 2;
}
void FillJne(stack_t* stk, Processor* code)
{
    int value_1 = 0, value_2 = 0;
    StackPop(stk, &value_1);
    StackPop(stk, &value_2);
    //printf("%d %d\n", value_1, value_2);
    if (value_1 != value_2) code->pc = code->data[code->pc + 1];
    else code->pc += 2;
}
void FillCall(stack_t* foo, Processor* code)
{
    StackPush(foo, code->pc + 2);
    //printf("%d\n", code->pc + 2);
    code->pc = code->data[code->pc + 1];
}
void FillRet(stack_t* foo, Processor* code)
{
    int answer = 0;
    StackPop(foo, &answer);
    code->pc = answer;
}
void FillIn(stack_t* stk, Processor* code)
{
    int answer = 0;
    code->pc += 1;
    scanf("%d", &answer);
    //printf("%d\n", answer);
    StackPush(stk, answer);
    //printf("%d\n", code->pc);
}

void Run(Processor* code)
{
    stack_t foo = {};
    stack_t stak = {};
    StackCtor(&foo, 1);
    stack_t* stk = &stak;
    StackCtor(stk, 1);
    code->pc = 0;
    while (code->pc < code->length)
    {
    //ProcessorDump(code);
    switch (code->data[code->pc]){
        case HLT:
            return;
        case PUSH:
            FillPush(stk, code);
            break;
        case ADD:
            FillAdd(stk, code);;
            break;
        case SUB:
            FillSub(stk, code);
            break;
        case DIV:
            FillDiv(stk, code);
            break;
        case OUT:
            FillOut(stk, code);
            break;
        case MUL:
            FillMul(stk, code);
            break;
        case POP:
            FillPop(stk, code);
            break;
        case PUSHr:
            FillPushr(stk, code);
            break;
        case JMP:
            code->pc = code->data[code->pc + 1];
            break;
        case JA:
            FillJa(stk, code);
            break;
        case JAE:
            FillJae(stk, code);
            break;
        case JB:
            FillJb(stk, code);
            break;
        case JBE:
            FillJbe(stk, code);
            break;
        case JE:
            FillJe(stk, code);
            break;
        case JNE:
            FillJne(stk, code);
            break;
        case IN:
            FillIn(stk, code);
            break;
        case CALL:
            FillCall(&foo, code);
            break;
        case RET:
            FillRet(&foo, code);
            break;
        }
    }
    StackDtor(stk);
    StackDtor(&foo);
}

size_t GetSize(const char* filename)
{
    struct stat file_info;
    stat(filename, &file_info);
    return file_info.st_size;
}

char* ReadFile(const char* name, size_t* length)
{
    size_t SIZE = GetSize(name);
    FILE* input_file = fopen(name, "r");
    char* buffer = (char*)calloc(SIZE, sizeof(char));
    *length = fread(buffer, sizeof(char), SIZE, input_file);
    fclose(input_file);
    return buffer;
}

void CreateProcessor(Processor* code, char* buffer, size_t length)
{
    code->data = (int*)calloc(length, sizeof(int));
    size_t i = 0;
    int offset = 0;
    while (sscanf(buffer, "%d%n", code->data + i, &offset) == 1)
    {
        //printf("%d\n", code->data[i]);
        ++i;
        buffer += offset;
    }
    code->data = (int*)realloc(code->data, sizeof(int) * i);
    code->length = i;
}

void ProcessorDump(Processor* code)
{
    printf("pc = %d\nprocessor = ", code->pc);
    for (int i = 0; i < code->length; ++i)
    {
        if (i == code->pc) printf(GREEN);
        printf("%d ", code->data[i]);
        printf(RESET);
    }
    printf("\n\n\n");
    getchar();
}
