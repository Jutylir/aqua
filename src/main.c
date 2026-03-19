/*TODO: Trouver une manière de rendre le code plus propre, par exemple essayer de mettre dans une fonction le check de fin de ligne
Vérifier si il serait préférable de free() tous nos pointeurs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct Token
{
    char type[1024];
    char value[1024];
    struct Token *next;
};

struct TokenListe
{
    struct Token *head;
};

struct identifier
{
    char name[1024];
    int stackPos;
    struct identifier *next;
};

struct identifierListe
{
    struct identifier *head;
};

void ajouterToken(struct TokenListe *liste, char type[1024], char value[1024])
{
    struct Token *current = liste->head;
    struct Token *newToken = (struct Token *)malloc(sizeof(struct Token));
    strcpy(newToken->type, type);
    strcpy(newToken->value, value);
    newToken->next = NULL;

    if (current == NULL)
    {
        liste->head = newToken;
    }
    else
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newToken;
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

void afficherTokens(struct TokenListe *liste)
{
    struct Token *current = liste->head;
    while (current != NULL)
    {
        printf("Type: %s, Value: %s\n", current->type, current->value);
        current = current->next;
    }
}

char *stringify(int buffer[1024])
{
    static char str[1024];
    int i;
    for (i = 0; buffer[i] != '\0' && i < 1023; i++)
    {
        str[i] = (char)buffer[i];
    }
    str[i] = '\0'; // Null-terminate the string
    return str;
}

int isStatement(char *value, char **statements)
{
    for (int i = 0; statements[i] != NULL; i++)
    {
        if (strcmp(statements[i], value) == 0)
        {
            return 1;
        }
    }
    return 0;
}

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

int main(int argc, char *argv[])
{

    // Check if the input file is provided as a command-line argument
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open the input file for reading
    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL)
    {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int buffer[1024];
    int c = fgetc(input_file);
    if (c == EOF)
    {
        fprintf(stderr, "Error: Empty file %s\n", argv[1]);
        fclose(input_file);
        return EXIT_FAILURE;
    }

    char *statements[] = {"return", NULL};

    int i = 0;
    int stackPosCount = 1;
    struct TokenListe tokenList;
    tokenList.head = NULL;
    char tokenType[1024];
    char tokenValue[1024] = {0};
    do // Tokenization loop
    {
        while (!isspace(c) && c != '\n' && c != EOF)
        {
            buffer[i] = c;
            i++;
            c = fgetc(input_file);
        }
        buffer[i] = '\0'; // Null-terminate the string
        i = 0;            // Reset index for the next token
        if (isalpha(buffer[0]))
        {
            if (isStatement(stringify(buffer), statements) == 1)
            {
                strcpy(tokenType, "STATEMENT");
                strcpy(tokenValue, stringify(buffer));
            }
            else
            {
                strcpy(tokenType, "IDENTIFIER");
                strcpy(tokenValue, stringify(buffer));
            }
        }
        else if (isdigit(buffer[0]))
        {
            strcpy(tokenType, "NUMBER");
            strcpy(tokenValue, stringify(buffer));
        }
        else if (ispunct(buffer[0]))
        {
            strcpy(tokenType, "SYMBOL");
            strcpy(tokenValue, stringify(buffer));
        }
        else if (c == '\n')
        {
            strcpy(tokenType, "NEWLINE");
            strcpy(tokenValue, "\0");
        }
        ajouterToken(&tokenList, tokenType, tokenValue);
        c = fgetc(input_file);
    } while (c != EOF);

    // Display the tokens
    afficherTokens(&tokenList);

    struct Token *current = tokenList.head;
    if (current == NULL)
    {
        fprintf(stderr, "Error: Expected an identifier\n");
        fclose(input_file);
        return EXIT_FAILURE;
    }

    struct identifierListe identifierListe;
    identifierListe.head = NULL;
    int position;

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
                    if (current->next != NULL)
                    {
                        if (strcmp(current->next->type, "NEWLINE") == 0)
                        {
                            fprintf(output_file, "   mov rax, 60\n");
                            fprintf(output_file, "   mov rdi, %s\n", current->value);
                            fprintf(output_file, "   syscall\n");
                        }
                        else
                        {
                            fprintf(stderr, "Error: Unexpected token after return value\n");
                            fclose(input_file);
                            return EXIT_FAILURE;
                        }
                    }
                    else
                    {
                        fprintf(output_file, "   mov rax, 60\n");
                        fprintf(output_file, "   mov rdi, %s\n", current->value);
                        fprintf(output_file, "   syscall\n");
                    }
                }
                else if (current != NULL && strcmp(current->type, "IDENTIFIER") == 0 && isDeclared(current->value, &identifierListe) == 1)
                {
                    position = stackPos(current->value, &identifierListe);
                    if (current->next != NULL)
                    {
                        if (strcmp(current->next->type, "NEWLINE") == 0)
                        {
                            fprintf(output_file, "   mov rax, 60\n");
                            fprintf(output_file, "   mov rdi, [rbp - %d]\n", position * 8);
                            fprintf(output_file, "   syscall\n");
                        }
                        else
                        {
                            fprintf(stderr, "Error: Unexpected token after return value\n");
                            fclose(input_file);
                            return EXIT_FAILURE;
                        }
                    }
                    else
                    {
                        fprintf(output_file, "   mov rax, 60\n");
                        fprintf(output_file, "   mov rdi, [rbp - %d]\n", position * 8);
                        fprintf(output_file, "   syscall\n");
                    }
                }
                else
                {
                    fprintf(stderr, "Error: Expected a expression after return\n");
                    fclose(input_file);
                    return EXIT_FAILURE;
                }
            }
        }
        else if (strcmp(current->type, "IDENTIFIER") == 0)
        {
            if (current->next && current->next->next)
            {

                if (strcmp(current->next->value, "=") == 0 && strcmp(current->next->next->type, "NUMBER") == 0)
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
            }
        }
        current = current->next;
    }

    fclose(output_file);
    system("nasm -f elf64 ./src/output.asm -o ./src/output.o");
    system("ld ./src/output.o -o ./src/output -no-pie");

    // Close the input file
    fclose(input_file);
    return 0;
}