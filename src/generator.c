#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/tokenizer.h"
#include "headers/structs.h"
#include "headers/parser.h"

struct nativeFunction
{
    char name[1024];
    char assembly[4096];
    int argCount;
};

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

void ajouterIdentifier(struct identifierListe *liste, char name[1024], int *stackPos, char type[1024])
{
    struct identifier *current = liste->head;
    struct identifier *newIdentifier = (struct identifier *)malloc(sizeof(struct identifier));
    newIdentifier->stackPos = *stackPos;
    strcpy(newIdentifier->name, name);
    strcpy(newIdentifier->type, type);
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

int isNativeUsed(char *name, struct TokenListe tokenList)
{
    struct Token *current = tokenList.head;
    while (current != NULL)
    {
        if (strcmp(current->type, "NATIVE_FUNCTION") == 0 && strcmp(current->value, name) == 0)
        {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

char *getIdentifierType(char *name, struct identifierListe *liste)
{
    struct identifier *current = liste->head;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current->type;
        }
        current = current->next;
    }
    return "INT"; // type par défaut
}

struct Token *tokenTreater(struct Token *current, struct identifierListe *identifierListe, FILE *output_file, FILE *input_file, int *boucleCount, int *stackPosCount)
{
    int position;
    char type[1024];
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
                        strcpy(type, "int");
                        ajouterIdentifier(identifierListe, current->value, stackPosCount, type);
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
                    if (strcmp(current->next->next->type, "STRING") == 0)
                    {
                        fprintf(output_file, "   lea rax, str%d\n", current->next->next->stringId);
                        fprintf(output_file, "   mov [rbp - %d], rax\n", position * 8);
                    }
                    else
                    {
                        current = parser(current->next->next, identifierListe, output_file, boucleCount);
                        fprintf(output_file, "   mov [rbp - %d], rax\n", position * 8);
                    }
                }
                else
                {
                    if (strcmp(current->next->next->type, "STRING") == 0)
                    {
                        strcpy(type, "string");
                        ajouterIdentifier(identifierListe, current->value, stackPosCount, type);
                        current = current->next->next;
                        fprintf(output_file, "   lea rax, str%d\n", current->stringId);
                        fprintf(output_file, "   push rax\n");
                        (*stackPosCount)++;
                    }
                    else
                    {
                        strcpy(type, "int");
                        ajouterIdentifier(identifierListe, current->value, stackPosCount, type);
                        current = parser(current->next->next, identifierListe, output_file, boucleCount);
                        fprintf(output_file, "   push rax\n");
                        (*stackPosCount)++;
                    }
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
                strcpy(type, "int*");
                ajouterIdentifier(identifierListe, current->value, stackPosCount, type);
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
    else if (strcmp(current->type, "NATIVE_FUNCTION") == 0)
    {
        if (strcmp(current->value, "print") == 0)
        {
            current = current->next;
            if (current != NULL && strcmp(current->value, "(") != 0)
            {
                fprintf(stderr, "Error: Expected '(' after print\n");
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
            if (strcmp(current->next->type, "STRING") == 0)
            {
                fprintf(output_file, "   lea rdi, [buffer]\n");
                for (int i = 0; current->next->value[i] != '\0'; i++)
                {
                    fprintf(output_file, "   mov byte [rdi + %d], '%c'\n", i, current->next->value[i]);
                }
                fprintf(output_file, "   mov byte [rdi + %ld], 0\n", strlen(current->next->value));
                fprintf(output_file, "   call _print_string\n");
                return current->next->next;
            }
            else if (strcmp(current->next->type, "IDENTIFIER") == 0)
            {
                if (strcmp(getIdentifierType(current->next->value, identifierListe), "string") == 0)
                {
                    int position = stackPos(current->next->value, identifierListe);
                    fprintf(output_file, "   mov rdi, [rbp - %d]\n", position * 8);
                    fprintf(output_file, "   call _print_string\n");
                }
                else if (strcmp(getIdentifierType(current->next->value, identifierListe), "int*") == 0)
                {
                    int position = stackPos(current->next->value, identifierListe);
                    fprintf(output_file, "   mov rdi, rbp\n");
                    fprintf(output_file, "   sub rdi, %d\n", position * 8);
                    fprintf(output_file, "   call _print_hex\n");
                }
                else
                {
                    current = parser(current, identifierListe, output_file, boucleCount);
                    fprintf(output_file, "   mov rdi, rax\n");
                    fprintf(output_file, "   call _print_number\n");
                }
            }
            else
            {
                current = parser(current, identifierListe, output_file, boucleCount);
                fprintf(output_file, "   mov rdi, rax\n");
                fprintf(output_file, "   call _print_number\n");
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

    struct nativeFunction nativeFunctions[] = {
        {"print",
         "_print_number:\n"
         "   mov rax, rdi\n"
         "   lea rdi, [buffer + 18]\n"
         "   mov byte [buffer + 19], 10\n"
         "   mov rcx, 1\n"
         "   mov rbx, 10\n"
         "loop1:\n"
         "   cqo\n"
         "   idiv rbx\n"
         "   add rdx, 0x30\n"
         "   mov [rdi], dl\n"
         "   dec rdi\n"
         "   inc rcx\n"
         "   test rax, rax\n"
         "   jnz loop1\n"
         "   inc rdi\n"
         "   mov rax, 1\n"
         "   mov rsi, rdi\n"
         "   mov rdx, rcx\n"
         "   mov rdi, 1\n"
         "   syscall\n"
         "   ret\n"
         "_print_string:\n"
         "   mov rcx, 0\n"
         "_strlen_loop:\n"
         "   cmp byte [rdi + rcx], 0\n"
         "   je _strlen_done\n"
         "   inc rcx\n"
         "   jmp _strlen_loop\n"
         "_strlen_done:\n"
         "   mov byte [rdi + rcx], 10\n"
         "   inc rcx\n"
         "   mov rsi, rdi\n"
         "   mov rdx, rcx\n"
         "   mov rax, 1\n"
         "   mov rdi, 1\n"
         "   syscall\n"
         "   ret\n"
         "_print_hex:\n"
         "   mov rax, rdi\n"
         "   mov byte [buffer], '0'\n"
         "   mov byte [buffer + 1], 'x'\n"
         "   lea rsi, [buffer + 17]\n"
         "   mov rcx, 16\n"
         "_print_hex_loop:\n"
         "   mov rdx, rax\n"
         "   and rdx, 0xF\n"
         "   cmp rdx, 9\n"
         "   jle _print_hex_digit\n"
         "   add rdx, 'a' - 10\n"
         "   jmp _print_hex_store\n"
         "_print_hex_digit:\n"
         "   add rdx, '0'\n"
         "_print_hex_store:\n"
         "   mov [rsi], dl\n"
         "   dec rsi\n"
         "   shr rax, 4\n"
         "   dec rcx\n"
         "   jnz _print_hex_loop\n"
         "   mov byte [buffer + 18], 10\n"
         "   mov rax, 1\n"
         "   mov rdi, 1\n"
         "   lea rsi, [buffer]\n"
         "   mov rdx, 19\n"
         "   syscall\n"
         "   ret\n",
         1},
        {"", "", 0}};

    FILE *output_file = fopen("./src/output.asm", "w");

    if (isNativeUsed("print", tokenList) == 1)
    {
        fprintf(output_file, "section .bss\n");
        fprintf(output_file, "   buffer resb 20\n");
    }

    int stringCount = 0;
    struct Token *tmp = tokenList.head;
    
    int hasStrings = 0;
    while (tmp != NULL) {
        if (strcmp(tmp->type, "STRING") == 0) {
            hasStrings = 1;
            break;
        }
        tmp = tmp->next;
    }

    if (hasStrings) {
        fprintf(output_file, "section .data\n");
        tmp = tokenList.head;
        while (tmp != NULL)
        {
            if (strcmp(tmp->type, "STRING") == 0)
            {
                fprintf(output_file, "   str%d db \"%s\", 0, 0\n", stringCount, tmp->value);
                tmp->stringId = stringCount;  // ← stocker l'id dans le token
                stringCount++;
            }
            tmp = tmp->next;
        }
    }

    fprintf(output_file, "section .text\n");
    fprintf(output_file, "global _start\n");
    for (int i = 0; nativeFunctions[i].argCount != 0; i++)
    {
        if (isNativeUsed(nativeFunctions[i].name, tokenList) == 1)
        {
            fprintf(output_file, "%s", nativeFunctions[i].assembly);
        }
    }
    fprintf(output_file, "_start:\n");
    fprintf(output_file, "   push rbp\n");
    fprintf(output_file, "   mov rbp, rsp\n");

    while (current != NULL)
    {
        current = tokenTreater(current, &identifierListe, output_file, input_file, &boucleCount, &stackPosCount);
    }

    fprintf(output_file, "   mov rax, 60\n   mov rdi, 0\n   syscall\n");

    freeIdentifierList(&identifierListe);
    fclose(output_file);
    return;
}