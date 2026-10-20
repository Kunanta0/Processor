#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

enum
{
    HLT,
    PUSH,
    ADD,
    SUB,
    DIV,
    OUT,
    MUL,
    POP,
    PUSHr,
    JMP,
    JA,
    JAE,
    JB,
    JBE,
    JE,
    JNE,
    CALL,
    RET,
    IN
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
} Translator;

Translator FillCode(char* buffer, size_t length, size_t* nlines, int* labels);
size_t GetSize(const char* filename);
size_t GetLines(char* buffer, size_t length);
char* ReadFile(const char* name, size_t* length);
Line GetLine(char* line, int* labels);
void WriteCode(Translator* arr, const char* filename, size_t nlines);
void GetCommand(Line* command, char* cmd);

int main()
{
    int labels[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    size_t length = 0;
    char* buffer = ReadFile("code.txt", &length);
    //printf("%s\n", buffer);

    size_t nlines = 0;
    Translator code = FillCode(buffer, length, &nlines, labels);

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

Translator FillCode(char* buffer, size_t length, size_t* nlines, int* labels)
{
    *nlines = GetLines(buffer, length);
    //printf("%lld\n", *nlines);

    Translator Machine = {};
    Machine.code = (Line*)calloc(*nlines, sizeof(Line));

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

    Machine.current_translating = 0;
    for (size_t i = 0; i < *nlines; ++i)
    {
        //printf("%d\n", Machine.current_translating);
        Machine.code[i] = GetLine(buffer + index_lines[i], labels);
        Machine.current_translating += Machine.code[i].length;
    }
    free(index_lines);

    return Machine;
}

Line GetLine(char* line, int* labels)
{
    Line command = {};
    char cmd[10];
    int number;
    command.length = sscanf(line, "%9s %d", cmd, &number);

    if (strcmp(cmd, "POP") == 0 || (strcmp(cmd, "PUSH") == 0 && command.length == 1))
    {
        command.length = 2;
        if (strcmp(cmd, "POP") == 0) command.line[0] = POP;
        if (strcmp(cmd, "PUSH") == 0) command.line[0] = PUSHr;
        char reg[3];
        sscanf(line, "%s %s", cmd, reg);
        //printf("reg: %s\n", reg);
        if (strcmp(reg, "AX") == 0) command.line[1] = 0;
        if (strcmp(reg, "BX") == 0) command.line[1] = 1;
        if (strcmp(reg, "CX") == 0) command.line[1] = 2;
        if (strcmp(reg, "DX") == 0) command.line[1] = 3;
        fprintf(stdout, "%d %d\n", command.line[0], command.line[1]);
        return command;
    }

    GetCommand(&command, cmd);
    if (strcmp(cmd, ":") == 0)
    {
        command.length = 0;
    }

    if (command.length == 2)
    {
        command.line[1] = number;
        fprintf(stdout, "%d %d\n", command.line[0], command.line[1]);
    }
    if (command.length == 1)
    {
        fprintf(stdout, "%d\n", command.line[0]);
    }
    return command;
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

void GetCommand(Line* command, char* cmd)
{
    if (strcmp(cmd, "SUB") == 0) command->line[0] = SUB;
    if (strcmp(cmd, "PUSH") == 0) command->line[0] = PUSH;
    if (strcmp(cmd, "OUT") == 0) command->line[0] = OUT;
    if (strcmp(cmd, "HLT") == 0) command->line[0] = HLT;
    if (strcmp(cmd, "DIV") == 0) command->line[0] = DIV;
    if (strcmp(cmd, "ADD") == 0) command->line[0] = ADD;
    if (strcmp(cmd, "MUL") == 0) command->line[0] = MUL;

    {
        if (strcmp(cmd, "JMP") == 0) command->line[0] = JMP;
        if (strcmp(cmd, "JA") == 0) command->line[0] = JA;
        if (strcmp(cmd, "JAE") == 0) command->line[0] = JAE;
        if (strcmp(cmd, "JB") == 0) command->line[0] = JB;
        if (strcmp(cmd, "JBE") == 0) command->line[0] = JBE;
        if (strcmp(cmd, "JE") == 0) command->line[0] = JE;
        if (strcmp(cmd, "JNE") == 0) command->line[0] = JNE;
    }

    if (strcmp(cmd, "CALL") == 0) command->line[0] = CALL;
    if (strcmp(cmd, "RET") == 0) command->line[0] = RET;
    if (strcmp(cmd, "IN") == 0) command->line[0] = IN;
}

//походу придется либо редачить передачей транслятора по адресу, либо добавить поле label в Line. Второе легче, но много памяти жрет впустую
