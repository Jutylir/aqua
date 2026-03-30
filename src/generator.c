#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/tokenizer.h"
#include "headers/structs.h"
#include "headers/parser.h"

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

void freeIdentifierList(struct identifierListe *liste)
{
    struct identifier *current = liste->head;
    while (current != NULL)
    {
        struct identifier *next = current->next;
        free(current);
        current = next;
    }
    liste->head = NULL;
}

void generator(FILE *input_file, struct TokenListe tokenList)
{
    int boucleCount = 0;
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
                current = parser(current, identifierListe, output_file, &boucleCount);
                printf("fin de parsing\n");
                fprintf(output_file, "   mov rdi, rax\n");
                fprintf(output_file, "   mov rax, 60\n");
                fprintf(output_file, "   syscall\n");
            }
        }
        else if (strcmp(current->type, "IDENTIFIER") == 0)
        {
            if (current->next && current->next->next)
            {

                if (strcmp(current->next->value, "=") == 0)
                {
                    if (isDeclared(current->value, &identifierListe) == 1)
                    {
                        position = stackPos(current->value, &identifierListe);
                        current = parser(current->next->next, identifierListe, output_file, &boucleCount);
                        printf("fin de parsing\n");
                        fprintf(output_file, "   mov [rbp - %d], rax\n", position * 8);
                    }
                    else
                    {
                        ajouterIdentifier(&identifierListe, current->value, stackPosCount);
                        current = parser(current->next->next, identifierListe, output_file, &boucleCount);
                        printf("fin de parsing\n");
                        fprintf(output_file, "   push rax\n");
                        stackPosCount++;
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