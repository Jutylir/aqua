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
        if (strcmp(current->value, "if") == 0)
        {
            int currentBoucleCount = *boucleCount;
            (*boucleCount)++;
            current = current->next;
            current = parser(current, identifierListe, output_file, boucleCount);
            fprintf(output_file, "   cmp rax, 1\n");
            fprintf(output_file, "   jne .else%d\n", currentBoucleCount);
            if (current != NULL && strcmp(current->value, ")") == 0)
            {
                current = current->next;
            }
            if (current != NULL && strcmp(current->type, "NEWLINE") == 0)
            {
                current = current->next;
            }
            if (current != NULL && strcmp(current->type, "SYMBOL") == 0 && strcmp(current->value, "{") == 0)
            {
                current = tokenTreater(current, identifierListe, output_file, input_file, boucleCount, stackPosCount);
            }
            else
            {
                fprintf(stderr, "Error: Expected '{' after if statement\n");
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
            fprintf(output_file, "   jmp .endif%d\n", currentBoucleCount);
            fprintf(output_file, ".else%d:\n", currentBoucleCount);
            if (current != NULL && strcmp(current->type, "STATEMENT") == 0 && strcmp(current->value, "else") == 0)
            {
                current = current->next;
                if (current != NULL && strcmp(current->type, "SYMBOL") == 0 && strcmp(current->value, "{") == 0)
                {
                    current = tokenTreater(current, identifierListe, output_file, input_file, boucleCount, stackPosCount);
                }
            }
            fprintf(output_file, ".endif%d:\n", currentBoucleCount);
        }
        if (strcmp(current->value, "while") == 0)
        {
            int currentBoucleCount = *boucleCount;
            (*boucleCount)++;
            fprintf(output_file, ".while%d:\n", currentBoucleCount);
            current = current->next;
            current = parser(current, identifierListe, output_file, boucleCount);
            fprintf(output_file, "   cmp rax, 1\n");
            fprintf(output_file, "   jne .endwhile%d\n", currentBoucleCount);
            if (current != NULL && strcmp(current->value, ")") == 0)
            {
                current = current->next;
            }
            if (current != NULL && strcmp(current->type, "NEWLINE") == 0)
            {
                current = current->next;
            }
            if (current != NULL && strcmp(current->type, "SYMBOL") == 0 && strcmp(current->value, "{") == 0)
            {
                current = tokenTreater(current, identifierListe, output_file, input_file, boucleCount, stackPosCount);
            }
            else
            {
                fprintf(stderr, "Error: Expected '{' after while statement\n");
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
            fprintf(output_file, "   jmp .while%d\n", currentBoucleCount);
            fprintf(output_file, ".endwhile%d:\n", currentBoucleCount);
        }
        if (strcmp(current->value, "for") == 0)
        {
            int currentBoucleCount = *boucleCount;
            (*boucleCount)++;
            current = current->next; // sauter "for"
            if (current != NULL && strcmp(current->value, "(") == 0)
            {
                current = current->next; // sauter "("
                if (current != NULL && strcmp(current->type, "IDENTIFIER") == 0)
                {
                    int forVarPos = *stackPosCount;
                    if (current != NULL && strcmp(current->next->value, "=") == 0)
                    {
                        ajouterIdentifier(identifierListe, current->value, stackPosCount);
                        (*stackPosCount)++;
                        current = current->next->next; // sauter "IDENTIFIER" et "="
                        current = parser(current, identifierListe, output_file, boucleCount);
                        fprintf(output_file, "   push rax\n");
                        if (current != NULL && strcmp(current->value, ";") == 0)
                        {
                            fprintf(output_file, ".for%d:\n", currentBoucleCount);
                            current = parser(current->next, identifierListe, output_file, boucleCount);
                            fprintf(output_file, "   cmp rax, 1\n");
                            fprintf(output_file, "   jne .endfor%d\n", currentBoucleCount);
                            if (current != NULL && strcmp(current->value, ";") == 0)
                            {
                                current = parser(current->next, identifierListe, output_file, boucleCount);
                                if (current != NULL && strcmp(current->value, ")") == 0)
                                {
                                    current = current->next; // sauter ")"
                                }
                                if (current != NULL && strcmp(current->type, "NEWLINE") == 0)
                                {
                                    current = current->next;
                                }
                                if (current != NULL && strcmp(current->type, "SYMBOL") == 0 && strcmp(current->value, "{") == 0)
                                {
                                    current = tokenTreater(current, identifierListe, output_file, input_file, boucleCount, stackPosCount);
                                    fprintf(output_file, "   jmp .for%d\n", currentBoucleCount);
                                    fprintf(output_file, ".endfor%d:\n", currentBoucleCount);
                                }
                                else
                                {
                                    fprintf(stderr, "Error: Expected '{' after for statement\n");
                                    fclose(input_file);
                                    exit(EXIT_FAILURE);
                                }
                            }
                            else
                            {
                                fprintf(stderr, "Error: Expected ';' in for statement\n");
                                fclose(input_file);
                                exit(EXIT_FAILURE);
                            }
                        }
                        else
                        {
                            fprintf(stderr, "Error: Expected ';' in for statement\n");
                            fclose(input_file);
                            exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Error: Expected '=' in for statement\n");
                        fclose(input_file);
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    fprintf(stderr, "Error: Expected identifier in for statement\n");
                    fclose(input_file);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                fprintf(stderr, "Error: Expected '(' after for statement\n");
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
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
                current = parser(current, identifierListe, output_file, boucleCount);
            }
        }
    }
    else if (strcmp(current->type, "SYMBOL") == 0)
    {
        if (strcmp(current->value, "{") == 0)
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
        else if (strcmp(current->value, "*") == 0)
        {
            if (strcmp(current->next->type, "IDENTIFIER") != 0)
            {
                fprintf(stderr, "Error: Expected identifier after '*'\n");
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
            current = current->next; // sauter "*"
            if (isDeclared(current->value, identifierListe) == 1)
            {
                position = stackPos(current->value, identifierListe);
                current = parser(current->next->next, identifierListe, output_file, boucleCount);
                fprintf(output_file, "   mov rbx, [rbp - %d]\n", position * 8);
                fprintf(output_file, "   mov qword [rbx], rax\n");
            }
            else
            {
                ajouterIdentifier(identifierListe, current->value, stackPosCount);
                current = parser(current->next->next, identifierListe, output_file, boucleCount);
                fprintf(output_file, "   mov rdx, rax\n");
                fprintf(output_file, "   mov rax, 12\n");
                fprintf(output_file, "   syscall\n");
                fprintf(output_file, "   push rax\n");
                fprintf(output_file, "   add rax, 8\n");
                fprintf(output_file, "   mov rdi, rax\n");
                fprintf(output_file, "   mov rax, 12\n");
                fprintf(output_file, "   syscall\n");
                fprintf(output_file, "   mov rax, [rbp - %d]\n", (*stackPosCount) * 8);
                fprintf(output_file, "   mov qword [rax], rdx\n");
                (*stackPosCount)++;
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