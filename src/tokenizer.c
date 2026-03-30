#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "headers/structs.h"

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

int isSymbol(char *value, char **symbols)
{
    for (int i = 0; symbols[i] != NULL; i++)
    {
        if (strcmp(symbols[i], value) == 0)
        {
            return 1;
        }
    }
    return 0;
}

struct TokenListe tokenizer(FILE *input_file)
{
    struct TokenListe tokenList;
    tokenList.head = NULL;
    int buffer[1024];
    int i = 0;

    int c = fgetc(input_file);
    if (c == EOF)
    {
        fprintf(stderr, "Error: Empty file");
        fclose(input_file);
        exit(EXIT_FAILURE);
    }

    char *statements[] = {"return", NULL};
    char *symbols[] = {"=", "+", "-", "/", "\\", "'", "''", "\"", "\"\"", "&", "{", "}", "{}", "(", ")", "*", "()", "**", "%", ".", "!", ">", ">=", "<", "<=", "++", "--", "+=", "-=", "[", "]", "[]", NULL};

    char tokenType[1024];
    char tokenValue[1024] = {0};
    do
    {
        // Sauter espaces et tabulations
        while (c == ' ' || c == '\t')
        {
            c = fgetc(input_file);
        }

        // Newline
        if (c == '\n')
        {
            strcpy(tokenType, "NEWLINE");
            strcpy(tokenValue, "\0");
            ajouterToken(&tokenList, tokenType, tokenValue);
            c = fgetc(input_file);
            continue;
        }

        if (c == EOF)
        {
            strcpy(tokenType, "EOL");
            strcpy(tokenValue, "\0");
            ajouterToken(&tokenList, tokenType, tokenValue);
            break;
        }

        // Lire le token selon son type de départ
        if (isalpha(c))
        {
            // IDENTIFIER : lettres uniquement, stop au chiffre ou symbole
            while (isalpha(c) || isdigit(c))
            {
                buffer[i++] = c;
                c = fgetc(input_file);
            }
        }
        else if (isdigit(c))
        {
            // NUMBER : chiffres uniquement, stop à la lettre ou symbole
            while (isdigit(c))
            {
                buffer[i++] = c;
                c = fgetc(input_file);
            }
            // Si le char suivant est une lettre → erreur lexicale "65er"
            if (isalpha(c))
            {
                fprintf(stderr, "Erreur lexicale : nombre invalide\n");
            }
        }
        else if (ispunct(c))
        {
            // SYMBOL : consomme les symboles consécutifs (pour ==, !=, ->)
            while (ispunct(c) && !isspace(c) && c != EOF)
            {
                buffer[i++] = c;
                c = fgetc(input_file);
            }
        }

        buffer[i] = '\0';
        i = 0;

        if (buffer[0] == '\0')
            continue;

        if (isalpha(buffer[0]))
        {
            if (isStatement(stringify(buffer), statements) == 1)
            {
                strcpy(tokenType, "STATEMENT");
                strcpy(tokenValue, stringify(buffer));
                ajouterToken(&tokenList, tokenType, tokenValue);
            }
            else
            {
                strcpy(tokenType, "IDENTIFIER");
                strcpy(tokenValue, stringify(buffer));
                ajouterToken(&tokenList, tokenType, tokenValue);
            }
        }
        else if (isdigit(buffer[0]))
        {
            strcpy(tokenType, "NUMBER");
            strcpy(tokenValue, stringify(buffer));
            ajouterToken(&tokenList, tokenType, tokenValue);
        }
        else if (ispunct(buffer[0]))
        {
            if (isSymbol(stringify(buffer), symbols) == 1)
            {
                strcpy(tokenType, "SYMBOL");
                strcpy(tokenValue, stringify(buffer));
                ajouterToken(&tokenList, tokenType, tokenValue);
            }
            else
            {
                fprintf(stderr, "Error: Unknown token\n");
                fclose(input_file);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "Error: Unknown token\n");
            fclose(input_file);
            exit(EXIT_FAILURE);
        }
    } while (c != EOF);
    strcpy(tokenType, "EOL");
    strcpy(tokenValue, "\0");
    ajouterToken(&tokenList, tokenType, tokenValue);
    return tokenList;
}