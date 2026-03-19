/*TODO: Differencier identifiant et mot-clé (a faire quand on se penchera sur la création de variable)
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

    int i = 0;
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
            strcpy(tokenType, "IDENTIFIER");
            strcpy(tokenValue, stringify(buffer));
        }
        else if (isdigit(buffer[0]))
        {
            strcpy(tokenType, "NUMBER");
            strcpy(tokenValue, stringify(buffer));
        }else if (c == '\n')
        {
            strcpy(tokenType, "NEWLINE");
            strcpy(tokenValue, "\0");
        }
        // else
        // {
        //     strcpy(tokenType, "SYMBOL");
        //     strcpy(tokenValue, stringify(buffer));
        // }
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

    FILE *output_file = fopen("./src/output.asm", "w");
    fprintf(output_file, "section .text\nglobal _start\n_start:\n");

    while (current != NULL)
    {
        if (strcmp(current->type, "IDENTIFIER") == 0)
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
                else
                {
                    fprintf(stderr, "Error: Expected a expression after return\n");
                    fclose(input_file);
                    return EXIT_FAILURE;
                }
            }
        }
        // else
        // {
        //     fprintf(stderr, "Error: Expected an identifier\n");
        //     fclose(input_file);
        //     return EXIT_FAILURE;
        // }
        current = current->next;
    }

    fclose(output_file);
    system("nasm -f elf64 ./src/output.asm -o ./src/output.o");
    system("ld ./src/output.o -o ./src/output -no-pie");

    // Close the input file
    fclose(input_file);
    return 0;
}