#include "headers/structs.h"
#include "headers/generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Token *parser(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount)
{
    printf("Parsing token: Type: %s, Value: %s\n", current->type, current->value);
    if (strcmp(current->type, "NUMBER") == 0)
    {
        fprintf(output_file, "   mov rax, %s\n", current->value);
        if (strcmp(current->next->type, "SYMBOL") == 0)
        {
            current = current->next;
            parser(current, identifierListe, output_file, boucleCount);
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
                current = current->next;
                parser(current, identifierListe, output_file, boucleCount);
            }
        }
        else
        {
            fprintf(stderr, "Error: Undeclared identifier %s\n", current->value);
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(current->type, "SYMBOL") == 0)
    {
        if (strcmp(current->next->type, "EOF") == 0 || strcmp(current->next->type, "NEWLINE") == 0 || current->next == NULL)
        {
            if (strcmp(current->value, "++") == 0)
            {
                if (strcmp(current->next->type, "NEWLINE") == 0 || strcmp(current->next->type, "EOF") == 0)
                {
                    fprintf(output_file, "   inc rax\n");
                }
                else
                {
                    fprintf(stderr, "Error: Unexpected token after ++ operator\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if (strcmp(current->value, "--") == 0)
            {
                if (strcmp(current->next->type, "NEWLINE") == 0 || strcmp(current->next->type, "EOF") == 0)
                {
                    fprintf(output_file, "   dec rax\n");
                }
                else
                {
                    fprintf(stderr, "Error: Unexpected token after -- operator\n");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                fprintf(stderr, "Error: Unsupported unary operator %s\n", current->value);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (strcmp(current->value, "+") == 0)
            {
                fprintf(output_file, "   push rax\n");
                current = current->next;
                parser(current, identifierListe, output_file, boucleCount);
                fprintf(output_file, "   pop rbx\n");
                fprintf(output_file, "   add rax, rbx\n");
            }
            else if (strcmp(current->value, "-") == 0)
            {
                fprintf(output_file, "   push rax\n");
                current = current->next;
                parser(current, identifierListe, output_file, boucleCount);
                fprintf(output_file, "   pop rbx\n");
                fprintf(output_file, "   sub rbx, rax\n");
                fprintf(output_file, "   mov rax, rbx\n");
            }
            else if (strcmp(current->value, "*") == 0)
            {
                current = current->next;
                if (strcmp(current->type, "NUMBER") == 0)
                {
                    printf("Parsing token: Type: %s, Value: %s\n", current->type, current->value);
                    fprintf(output_file, "   mov rbx, %s\n", current->value);
                }
                else if (strcmp(current->type, "IDENTIFIER") == 0)
                {
                    printf("Parsing token: Type: %s, Value: %s\n", current->type, current->value);
                    if (isDeclared(current->value, &identifierListe) == 1)
                    {
                        int position = stackPos(current->value, &identifierListe);
                        fprintf(output_file, "   mov rbx, [rbp - %d]\n", position * 8);
                    }
                    else
                    {
                        fprintf(stderr, "Error: Undeclared identifier %s\n", current->value);
                        exit(EXIT_FAILURE);
                    }
                }
                else if (strcmp(current->value, "(") == 0)
                {
                    current = current->next;
                    parser(current->next, identifierListe, output_file, boucleCount);
                }
                fprintf(output_file, "   imul rax, rbx\n");
                if (strcmp(current->next->type, "SYMBOL") == 0)
                {
                    current = current->next;
                    parser(current, identifierListe, output_file, boucleCount);
                }
            }
            else if (strcmp(current->value, "/") == 0)
            {
                current = current->next;
                if (strcmp(current->type, "NUMBER") == 0)
                {
                    printf("Parsing token: Type: %s, Value: %s\n", current->type, current->value);
                    fprintf(output_file, "   mov rbx, %s\n", current->value);
                }
                else if (strcmp(current->type, "IDENTIFIER") == 0)
                {
                    printf("Parsing token: Type: %s, Value: %s\n", current->type, current->value);
                    if (isDeclared(current->value, &identifierListe) == 1)
                    {
                        int position = stackPos(current->value, &identifierListe);
                        fprintf(output_file, "   mov rbx, [rbp - %d]\n", position * 8);
                    }
                    else
                    {
                        fprintf(stderr, "Error: Undeclared identifier %s\n", current->value);
                        exit(EXIT_FAILURE);
                    }
                }
                else if (strcmp(current->value, "(") == 0)
                {
                    current = current->next;
                    parser(current->next, identifierListe, output_file, boucleCount);
                }
                fprintf(output_file, "   cdq\n");
                fprintf(output_file, "   idiv ebx\n");
                if (strcmp(current->next->type, "SYMBOL") == 0)
                {
                    current = current->next;
                    parser(current, identifierListe, output_file, boucleCount);
                }
            }
            else if (strcmp(current->value, "%") == 0)
            {
                fprintf(output_file, "   push rax\n");
                current = current->next;
                parser(current, identifierListe, output_file, boucleCount);
                fprintf(output_file, "   pop rbx\n");
                fprintf(output_file, "   xchg rax, rbx\n");
                fprintf(output_file, "   cdq\n");
                fprintf(output_file, "   idiv ebx\n");
                fprintf(output_file, "   mov rax, rdx\n");
            }
            else if (strcmp(current->value, "**") == 0)
            {
                fprintf(output_file, "   push rax\n");
                current = current->next;
                parser(current, identifierListe, output_file, boucleCount);
                fprintf(output_file, "   pop rbx\n");
                fprintf(output_file, "   mov rcx, rax\n");
                fprintf(output_file, "   mov rax, 1\n");
                fprintf(output_file, "power_loop_%d:\n", *boucleCount);
                fprintf(output_file, "   test rcx, rcx\n");
                fprintf(output_file, "   jz power_done_%d\n", *boucleCount);
                fprintf(output_file, "   imul rax, rbx\n");
                fprintf(output_file, "   dec rcx\n");
                fprintf(output_file, "   jmp power_loop_%d\n", *boucleCount);
                fprintf(output_file, "power_done_%d:\n", *boucleCount);
                (*boucleCount)++;
            }
            else
            {
                fprintf(stderr, "Error: Unsupported operator %s\n", current->value);
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        fprintf(stderr, "Error: Unexpected token type %s\n", current->type);
        exit(EXIT_FAILURE);
    }
    return current;
}