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

void ajouterIdentifier(struct identifierListe *liste, char name[1024], int *stackPos)
{
    struct identifier *current = liste->head;
    struct identifier *newIdentifier = (struct identifier *)malloc(sizeof(struct identifier));
    newIdentifier->stackPos = *stackPos;
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

struct Token *tokenTreater(struct Token *current, struct identifierListe *identifierListe, FILE *output_file, FILE *input_file, int *boucleCount, int *stackPosCount)
{
    int position;
    if (strcmp(current->type, "STATEMENT") == 0)
    {
        if (strcmp(current->value, "return") == 0)
        {
            current = current->next;
            current = parser(current, identifierListe, output_file, boucleCount);
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
                if (isDeclared(current->value, identifierListe) == 1)
                {
                    position = stackPos(current->value, identifierListe);
                    current = parser(current->next->next, identifierListe, output_file, boucleCount);
                    fprintf(output_file, "   mov [rbp - %d], rax\n", position * 8);
                }
                else
                {
                    ajouterIdentifier(identifierListe, current->value, stackPosCount);
                    current = parser(current->next->next, identifierListe, output_file, boucleCount);
                    fprintf(output_file, "   push rax\n");
                    (*stackPosCount)++;
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
    else if (strcmp(current->type, "SYMBOL") == 0 && strcmp(current->value, "{") == 0)
    {
        int currentStackPos = *stackPosCount;
        current = current->next; // sauter "{"
        while (current != NULL && !(strcmp(current->type, "SYMBOL") == 0 && strcmp(current->value, "}") == 0))
        {
            current = tokenTreater(current, identifierListe, output_file, input_file, boucleCount, stackPosCount);
        }
        // Libérer les variables locales à ce bloc
        for (int i = currentStackPos; i < *stackPosCount; i++)
        {
            fprintf(output_file, "   pop rax\n");
        }
        struct identifier *currentIdentifier = identifierListe->head;
        struct identifier *prevIdentifier = NULL;
        while (currentIdentifier != NULL)
        {
            if (currentIdentifier->stackPos >= currentStackPos)
            {
                struct identifier *toFree = currentIdentifier;
                if (prevIdentifier == NULL)
                {
                    identifierListe->head = currentIdentifier->next;
                }
                else
                {
                    prevIdentifier->next = currentIdentifier->next;
                }
                currentIdentifier = currentIdentifier->next;
                free(toFree);
            }
            else
            {
                prevIdentifier = currentIdentifier;
                currentIdentifier = currentIdentifier->next;
            }
        }
    }
    if (current == NULL)
    {
        return NULL;
    }
    if (current->next == NULL)
    {
        return NULL;
    }
    return current->next;
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
        current = tokenTreater(current, &identifierListe, output_file, input_file, &boucleCount, &stackPosCount);
    }

    freeIdentifierList(&identifierListe);
    fclose(output_file);
    return;
}