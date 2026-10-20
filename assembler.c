#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#define LABELS_SIZE 50

enum
{
    HLT = 0,
    PUSH = 1,
    ADD = 2,
    SUB = 3,
    DIV = 4,
    OUT = 5,
    MUL = 6,
    POP = 7,
    PUSHr = 8,
    JMP = 9,
    JA = 10,
    JAE = 11,
    JB = 12,
    JBE = 13,
    JE = 14,
    JNE = 15,
    CALL = 16,
    RET = 17,
    IN = 18
};

enum
{
    NO_LABEL = 0,
    WITH_LABEL = 1
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

Translator FillCode(char* buffer, size_t length, size_t* nlines);
size_t GetSize(const char* filename);
size_t GetLines(char* buffer, size_t length);
char* ReadFile(const char* name, size_t* length);
void GetLine(Translator* Machine, char* line, size_t i);
void WriteCode(Translator* arr, const char* filename, size_t nlines);
int GetCommand(Translator* Machine, char* cmd, char* line, size_t i);
void FillLabels(Translator* Machine);

int main()
{
    size_t length = 0;
    char* buffer = ReadFile("code.txt", &length);
    //printf("%s\n", buffer);

    size_t nlines = 0;
    Translator code = FillCode(buffer, length, &nlines);

    WriteCode(&code, "machine_code.txt", nlines);
    free(buffer);
    free(code.code);
    return 0;
}

void WriteCode(Translator* arr, const char* filename, size_t nlines)
{
    FILE* output = fopen(filename, "a");
    for (size_t i = 0; i < nlines; ++i)
    {
        Line curr = (arr->code)[i];
        for (size_t j = 0; j < curr.length; ++j)
        {
            fprintf(output, "%d", curr.line[j]);
            if (j < curr.length - 1) fprintf(output, " ");
        }
        if (i < nlines - 1) fprintf(output, "\n");
    }

}

size_t GetSize(const char* filename)
{
    struct stat file_info;
    stat(filename, &file_info);
    return file_info.st_size;
}

size_t GetLines(char* buffer, size_t length)
{
    size_t nlines = 0;
    for (size_t i = 0; i < length; ++i)
    {
        if (buffer[i] == '\n')
        {
            buffer[i] = '\0';
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

Translator FillCode(char* buffer, size_t length, size_t* nlines)
{
    *nlines = GetLines(buffer, length);
    //printf("%lld\n", *nlines);

    Translator Machine = {};
    Machine.code = (Line*)calloc(*nlines, sizeof(Line));

    FillLabels(&Machine);
    //PrintLabels(&Machine);

    int* index_lines = (int*)calloc(*nlines, sizeof(int));

    size_t j = 0;
    index_lines[0] = 0;
    for (size_t i = 0; i < length; ++i)
    {
        if (buffer[i] == '\0')
        {
            index_lines[++j] = i + 1;
        }
    }

    for (size_t j = 0; j < 2; ++j)
    {
        Machine.current_translating = 0;
        for (size_t i = 0; i < *nlines; ++i)
        {
            printf("[%lld]\n", Machine.current_translating);
            GetLine(&Machine, buffer + index_lines[i], i);
            Machine.current_translating += Machine.code[i].length;
        }
    }

    free(index_lines);

    return Machine;
}

void GetLine(Translator* Machine, char* line, size_t i)
{
    char cmd[10];
    int number;
    (Machine->code[i]).length = sscanf(line, "%5s %d", cmd, &number);
    printf("{%s} {%d}\n", cmd, Machine->code[i].length);

    if (strcmp(cmd, "POP") == 0 || (strcmp(cmd, "PUSH") == 0 && (Machine->code[i]).length == 1))
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
        fprintf(stdout, "%d %d\n", (Machine->code[i]).line[0], (Machine->code[i]).line[1]);
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
            fprintf(stdout, "%d %d\n", (Machine->code[i]).line[0], (Machine->code[i]).line[1]);
        }
        if ((Machine->code[i]).length == 1)
        {
            fprintf(stdout, "%d\n", (Machine->code[i]).line[0]);
        }
    }
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
        printf("!%d!\n", Machine->code[i].length);
        if (Machine->code[i].length == 1)
        {
            char a[5] = {};
            int label = 0;
            sscanf(line, "%5s : %d", a, &label);
            //printf("label = %d\n", label);
            //PrintLabels(Machine);
            Machine->code[i].line[1] = (Machine->labels)[label];
            Machine->code[i].length = 2;
            printf("{%d}\n", Machine->code[i].length);
            //printf("code[1] = %d\n", Machine->code[i].line[1]);
            return WITH_LABEL;
        }
    }

    return NO_LABEL;
}

//походу придется либо редачить передачей транслятора по адресу, либо добавить поле label в Line. Второе легче, но много памяти жрет впустую
