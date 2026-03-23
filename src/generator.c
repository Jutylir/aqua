#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/tokenizer.h"
#include "headers/structs.h"

int isDeclared(char *name, struct identifierListe *liste)
{
    struct identifier *current = liste->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

int stackPos(char *name, struct identifierListe *liste)
{
    struct identifier *current = liste->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current->stackPos;
        }
        current = current->next;
    }
    return -1;
}

int isEndOfLine(struct Token *token)
{
    if (token->next == NULL)
    {
        return 1;
    }
    else if (strcmp(token->next->type, "NEWLINE") == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void ajouterIdentifier(struct identifierListe *liste, char name[1024], int stackPos)
{
    struct identifier *current = liste->head;
    struct identifier *newIdentifier = (struct identifier *)malloc(sizeof(struct identifier));
    newIdentifier->stackPos = stackPos;
    strcpy(newIdentifier->name, name);
    if (current == NULL)
    {
        liste->head = newIdentifier;
    }
    else
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newIdentifier;
    }
}

void freeIdentifierList(struct identifierListe *liste) {
    struct identifier *current = liste->head;
    while (current != NULL) {
        struct identifier *next = current->next;
        free(current);
        current = next;
    }
    liste->head = NULL;
}

void generator(FILE *input_file, struct TokenListe tokenList)
{
    struct Token *current = tokenList.head;
    if (current == NULL)
    {
        fprintf(stderr, "Error: Expected an identifier\n");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }

    struct identifierListe identifierListe;
    identifierListe.head = NULL;
    int position;
    int stackPosCount = 1;

    FILE *output_file = fopen("./src/output.asm", "w");
    fprintf(output_file, "section .text\nglobal _start\n_start:\n   push rbp\n   mov rbp, rsp\n");

    while (current != NULL)
    {
        if (strcmp(current->type, "STATEMENT") == 0)
        {
            if (strcmp(current->value, "return") == 0)
            {
                current = current->next;
                if (current != NULL && strcmp(current->type, "NUMBER") == 0 && atoi(current->value) < 255)
                {
                    if (isEndOfLine(current) == 1)
                    {
                        fprintf(output_file, "   mov rax, 60\n");
                        fprintf(output_file, "   mov rdi, %s\n", current->value);
                        fprintf(output_file, "   syscall\n");
                    }
                    else
                    {
                        fprintf(stderr, "Error: Unexpected token after return value\n");
                        fclose(input_file);
                        exit(EXIT_FAILURE);
                    }
                }
                else if (current != NULL && strcmp(current->type, "IDENTIFIER") == 0 && isDeclared(current->value, &identifierListe) == 1)
                {
                    position = stackPos(current->value, &identifierListe);
                    if (isEndOfLine(current))
                    {
                        fprintf(output_file, "   mov rax, 60\n");
                        fprintf(output_file, "   mov rdi, [rbp - %d]\n", position * 8);
                        fprintf(output_file, "   syscall\n");
                    }
                    else
                    {
                        fprintf(stderr, "Error: Unexpected token after return value\n");
                        fclose(input_file);
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    fprintf(stderr, "Error: Expected an expression after return\n");
                    fclose(input_file);
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (strcmp(current->type, "IDENTIFIER") == 0)
        {
            if (current->next && current->next->next)
            {

                if (strcmp(current->next->value, "=") == 0 && strcmp(current->next->next->type, "NUMBER") == 0)
                {
                    if (isEndOfLine(current->next->next) == 1)
                    {
                        if (isDeclared(current->value, &identifierListe) == 1)
                        {
                            position = stackPos(current->value, &identifierListe);
                            fprintf(output_file, "   mov rax, %s", current->next->next->value);
                            fprintf(output_file, "   mov [rbp - %d]", position * 4);
                        }
                        else
                        {
                            fprintf(output_file, "   mov rax, %s\n", current->next->next->value);
                            fprintf(output_file, "   push rax\n");
                            ajouterIdentifier(&identifierListe, current->value, stackPosCount);
                            stackPosCount++;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Error: Unexpected token after identifier value\n");
                        fclose(input_file);
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    fprintf(stderr, "Error: Unexpected token or end of line\n");
                    fclose(input_file);
                    exit(EXIT_FAILURE);
                }
            }
        }
        current = current->next;
    }
    freeIdentifierList(&identifierListe);
    fclose(output_file);
    return;
}