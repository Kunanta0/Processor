#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "proc.h"
#define LABELS_SIZE 50

enum
{
    NO_LABEL = 0,
    WITH_LABEL = 1,
    ERROR = 2
};


typedef struct
{
    int line[2];
    size_t length;
} Line;

typedef struct
{
    Line* code;
    size_t current_translating;
    int labels[LABELS_SIZE];
} Translator;

typedef struct
{
    char* buffer;
    size_t length;
    size_t nlines;
    int* index_lines;
} Text;

Translator FillCode(Text* txt);
size_t GetSize(const char* filename);
size_t GetLines(Text* txt);
char* ReadFile(const char* name, size_t* length);
void GetLine(Translator* Machine, char* line, size_t i);
void WriteCode(Translator* arr, const char* filename, size_t nlines);
int GetCommand(Translator* Machine, char* cmd, char* line, size_t i);
void FillLabels(Translator* Machine);
Text FillText();
void Assemble(Translator* Machine, Text* txt);
void Destroy(Translator* Machine, Text* txt);
void FillRegisters(Translator* Machine, char* cmd, char* line, size_t i);

int main()
{
    Text txt = FillText();
    Translator code = FillCode(&txt);

    WriteCode(&code, "machine_code.txt", txt.nlines);
    Destroy(&code, &txt);
    return 0;
}

void Destroy(Translator* code, Text* txt)
{
    free(txt->buffer);
    free(txt->index_lines);
    free(code->code);
}

void WriteCode(Translator* arr, const char* filename, size_t nlines)
{
    FILE* output = fopen(filename, "w");
    for (size_t i = 0; i < nlines; ++i)
    {
        Line curr = (arr->code)[i];
        for (size_t j = 0; j < curr.length; ++j)
        {
            fprintf(output, "%d", curr.line[j]);
            if (j < curr.length - 1) fprintf(output, " ");
        }
        if (i < nlines - 1 && curr.length != 0) fprintf(output, "\n");
    }

}

Text FillText()
{
    Text txt = {};
    txt.buffer = ReadFile("code.txt", &(txt.length));
    txt.nlines = GetLines(&txt);

    txt.index_lines = (int*)calloc(txt.nlines, sizeof(int));

    size_t j = 0;
    txt.index_lines[0] = 0;
    for (size_t i = 0; i < txt.length; ++i)
    {
        if (txt.buffer[i] == '\0')
        {
            txt.index_lines[++j] = i + 1;
        }
    }

    return txt;
}

size_t GetSize(const char* filename)
{
    struct stat file_info;
    stat(filename, &file_info);
    return file_info.st_size;
}

size_t GetLines(Text* txt)
{
    size_t nlines = 0;
    for (size_t i = 0; i < txt->length; ++i)
    {
        if (txt->buffer[i] == '\n')
        {
            txt->buffer[i] = '\0';
            ++nlines;
        }
    }
    ++nlines;

    return nlines;
}

void FillLabels(Translator* Machine)
{
    for (int i = 0; i < LABELS_SIZE; ++i) Machine->labels[i] = -1;
}

void PrintLabels(Translator* Machine)
{
    printf("LABELS:\n");
    for (int i = 0; i < LABELS_SIZE; ++i) printf("%d ", Machine->labels[i]);
    printf("\n");
    getchar();
}

Translator FillCode(Text* txt)
{
    Translator Machine = {};
    Machine.code = (Line*)calloc(txt->nlines, sizeof(Line));

    FillLabels(&Machine);
    //PrintLabels(&Machine);

    Assemble(&Machine, txt);
    Assemble(&Machine, txt);

    return Machine;
}

void Assemble(Translator* Machine, Text* txt)
{
    Machine->current_translating = 0;
    for (size_t i = 0; i < txt->nlines; ++i)
    {
        //printf("[%lld]\n", Machine.current_translating);
        GetLine(Machine, txt->buffer + txt->index_lines[i], i);
        Machine->current_translating += Machine->code[i].length;
    }
}

void GetLine(Translator* Machine, char* line, size_t i)
{
    char cmd[10];
    int number;
    (Machine->code[i]).length = sscanf(line, "%5s %d", cmd, &number);
    //printf("{%s} {%d}\n", cmd, Machine->code[i].length);

    if (strcmp(cmd, "POP") == 0 || (strcmp(cmd, "PUSH") == 0 && (Machine->code[i]).length == 1))
    {
        FillRegisters(Machine, cmd, line, i);
        return;
    }

    if (strcmp(cmd, ":") == 0)
    {
        Machine->code[i].length = 0;
        Machine->labels[number] = Machine->current_translating;
        return;
    }

    int state = GetCommand(Machine, cmd, line, i);

    if (state == NO_LABEL)
    {
        if ((Machine->code[i]).length == 2)
        {
            (Machine->code[i]).line[1] = number;
            //fprintf(stdout, "%d %d\n", (Machine->code[i]).line[0], (Machine->code[i]).line[1]);
        }
        /*if ((Machine->code[i]).length == 1)
        {
            fprintf(stdout, "%d\n", (Machine->code[i]).line[0]);
        }*/
        return;
    }

    if (state == WITH_LABEL) return;

    printf("Error by address %lld in file\n", Machine->current_translating);
    exit(EXIT_FAILURE);
}

void FillRegisters(Translator* Machine, char* cmd, char* line, size_t i)
{
    (Machine->code[i]).length = 2;
    if (strcmp(cmd, "POP") == 0) (Machine->code[i]).line[0] = POP;
    if (strcmp(cmd, "PUSH") == 0) (Machine->code[i]).line[0] = PUSHr;
    char reg[3];
    sscanf(line, "%s %s", cmd, reg);
    //printf("reg: %s\n", reg);
    if (strcmp(reg, "AX") == 0) (Machine->code[i]).line[1] = 0;
    if (strcmp(reg, "BX") == 0) (Machine->code[i]).line[1] = 1;
    if (strcmp(reg, "CX") == 0) (Machine->code[i]).line[1] = 2;
    if (strcmp(reg, "DX") == 0) (Machine->code[i]).line[1] = 3;
    //fprintf(stdout, "%d %d\n", (Machine->code[i]).line[0], (Machine->code[i]).line[1]);
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

int GetCommand(Translator* Machine, char* cmd, char* line, size_t i)
{
    if (strcmp(cmd, "SUB") == 0) {(Machine->code[i]).line[0] = SUB; return NO_LABEL;}
    if (strcmp(cmd, "PUSH") == 0) {(Machine->code[i]).line[0] = PUSH; return NO_LABEL;}
    if (strcmp(cmd, "OUT") == 0) {(Machine->code[i]).line[0] = OUT; return NO_LABEL;}
    if (strcmp(cmd, "HLT") == 0) {(Machine->code[i]).line[0] = HLT; return NO_LABEL;}
    if (strcmp(cmd, "DIV") == 0) {(Machine->code[i]).line[0] = DIV; return NO_LABEL;}
    if (strcmp(cmd, "ADD") == 0) {(Machine->code[i]).line[0] = ADD; return NO_LABEL;}
    if (strcmp(cmd, "MUL") == 0) {(Machine->code[i]).line[0] = MUL; return NO_LABEL;}
    if (strcmp(cmd, "RET") == 0) {(Machine->code[i]).line[0] = RET; return NO_LABEL;}
    if (strcmp(cmd, "IN") == 0) {(Machine->code[i]).line[0] = IN; return NO_LABEL;}

    {
        if (strcmp(cmd, "CALL") == 0) (Machine->code[i]).line[0] = CALL;
        if (strcmp(cmd, "JMP") == 0) (Machine->code[i]).line[0] = JMP;
        if (strcmp(cmd, "JA") == 0) (Machine->code[i]).line[0] = JA;
        if (strcmp(cmd, "JAE") == 0) (Machine->code[i]).line[0] = JAE;
        if (strcmp(cmd, "JB") == 0) (Machine->code[i]).line[0] = JB;
        if (strcmp(cmd, "JBE") == 0) (Machine->code[i]).line[0] = JBE;
        if (strcmp(cmd, "JE") == 0) (Machine->code[i]).line[0] = JE;
        if (strcmp(cmd, "JNE") == 0) (Machine->code[i]).line[0] = JNE;
        //printf("!%d!\n", Machine->code[i].length);
        if (Machine->code[i].length == 1)
        {
            char a[5] = {};
            int label = 0;
            sscanf(line, "%5s : %d", a, &label);
            //printf("label = %d\n", label);
            //PrintLabels(Machine);
            Machine->code[i].line[1] = (Machine->labels)[label];
            Machine->code[i].length = 2;
            //printf("{%d}\n", Machine->code[i].length);
            //printf("code[1] = %d\n", Machine->code[i].line[1]);
            return WITH_LABEL;
        }

        if (Machine->code[i].length == 2) return NO_LABEL;
    }

    return ERROR;
}
