#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

size_t GetSize(const char* filename);

enum
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
    CALL = 16,//
    RET = 17,//
    IN = 18
};

int main()
{
    FILE* machine_code = fopen("machine_code.txt", "r");
    FILE* code = fopen("code.txt", "a");

    size_t size = GetSize("machine_code.txt");
    char* line = (char*)calloc(size, sizeof(char));
    while (fgets(line, sizeof(line), machine_code))
    {
        int cmd;
        int number;
        int n = sscanf(line, "%d %d", &cmd, &number);

        char command[10] = {};
        if (cmd == SUB) strcpy(command, "SUB");
        if (cmd == PUSH || cmd == PUSHr) strcpy(command, "PUSH");
        if (cmd == OUT) strcpy(command, "OUT");
        if (cmd == HLT) strcpy(command, "HLT");
        if (cmd == DIV) strcpy(command, "DIV");
        if (cmd == MUL) strcpy(command, "MUL");
        if (cmd == POP) strcpy(command, "POP");
        if (cmd == ADD) strcpy(command, "ADD");

        if (cmd == JMP) strcpy(command, "JMP");
        if (cmd == JA) strcpy(command, "JA");
        if (cmd == JAE) strcpy(command, "JAE");
        if (cmd == JB) strcpy(command, "JB");
        if (cmd == JBE) strcpy(command, "JBE");
        if (cmd == JE) strcpy(command, "JE");
        if (cmd == JNE) strcpy(command, "JNE");
        if (cmd == CALL) strcpy(command, "CALL");
        if (cmd == RET) strcpy(command, "RET");
        if (cmd == IN) strcpy(command, "IN");

        if (cmd != PUSHr && cmd != POP) {
            if (n == 2) fprintf(code, "%s %d\n", command, number);
            else fprintf(code, "%s\n", command);
        }

        else
        {
            fprintf(code, "%s ", command);
            switch (number) {
            case 0:
                fprintf(code, "AX\n");
                break;
            case 1:
                fprintf(code, "BX\n");
                break;
            case 2:
                fprintf(code, "CX\n");
                break;
            case 3:
                fprintf(code, "DX\n");
                break;
            }
        }
    }

    free(line);
    fclose(code);
    fclose(machine_code);
    return 0;
}

size_t GetSize(const char* filename)
{
    struct stat file_info;
    stat(filename, &file_info);
    size_t SIZE = file_info.st_size;
    return SIZE;
}
