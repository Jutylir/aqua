#include "headers/structs.h"
#include "headers/generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parser(struct Token *current, struct identifierListe identifierListe, FILE *output_file)
{
    printf("Parsing token: Type: %s, Value: %s\n", current->type, current->value);
    if (strcmp(current->type, "NUMBER") == 0)
    {
        fprintf(output_file, "   mov rax, %s\n", current->value);
        if (strcmp(current->next->type, "SYMBOL") == 0)
        {
            parser(current->next, identifierListe, output_file);
        }
    }
    else if (strcmp(current->type, "IDENTIFIER") == 0)
    {
        if (isDeclared(current->value, &identifierListe) == 1)
        {
            int position = stackPos(current->value, &identifierListe);
            fprintf(output_file, "   mov rax, [rbp - %d]\n", position * 8);
            if (strcmp(current->next->type, "SYMBOL") == 0)
            {
                parser(current->next, identifierListe, output_file);
            }
        }
        else
        {
            fprintf(stderr, "Error: Undeclared identifier %s\n", current->value);
            exit(EXIT_FAILURE);
        }
    }else if(strcmp(current->type, "SYMBOL") == 0)
    {
        if(strcmp(current->next->type, "EOL") == 0 || strcmp(current->next->type, "NEWLINE") == 0 || current->next == NULL){
            fprintf(stderr, "Error: Unexpected end of line after symbol %s\n", current->value);
            exit(EXIT_FAILURE);
        }
        if(strcmp(current->value, "+") == 0){
            fprintf(output_file, "   push rax\n");
            parser(current->next, identifierListe, output_file);
            fprintf(output_file, "   pop rbx\n");
            fprintf(output_file, "   add rax, rbx\n");
        }
    }
    else
    {
        fprintf(stderr, "Error: Unexpected token type %s\n", current->type);
        exit(EXIT_FAILURE);
    }
}