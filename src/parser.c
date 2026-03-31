#include "headers/structs.h"
#include "headers/generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Token *parsePrimary(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount);
struct Token *parseTerm(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount);
struct Token *parseExpression(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount);
struct Token *parser(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount);
struct Token *parseComparison(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount);

/*
Ordre de priorité des opérations :
1. Parenthèses -> parsePrimary
2. Opérateurs prioritaires -> parseTerm
3. Opérateurs d'addition et de soustraction -> parseExpression
4. Nombres et identifiants -> parsePrimary
5. Opérateurs de comparaison -> parseComparison

parser appelle toujours l'ordre de priorité le plus bas, qui lui-même appelle les ordre de priorité plus élevé

Par la suite, les opérateurs and et or seront ajoutés, avec une priorité plus basse que les comparaisons
*/

struct Token *parsePrimary(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount)
{
    if (strcmp(current->type, "NUMBER") == 0)
    {
        fprintf(output_file, "   mov rax, %s\n", current->value);
        return current->next;
    }
    else if (strcmp(current->type, "IDENTIFIER") == 0)
    {
        if (isDeclared(current->value, &identifierListe))
        {
            int position = stackPos(current->value, &identifierListe);
            fprintf(output_file, "   mov rax, [rbp - %d]\n", position * 8);

            // Vérifier si c'est ++ ou -- après la variable
            if (current != NULL && strcmp(current->type, "SYMBOL") == 0)
            {
                if (strcmp(current->value, "++") == 0)
                {
                    fprintf(output_file, "   inc rax\n");
                    fprintf(output_file, "   mov [rbp - %d], rax\n", position * 8);
                    current = current->next;
                }
                else if (strcmp(current->value, "--") == 0)
                {
                    fprintf(output_file, "   dec rax\n");
                    fprintf(output_file, "   mov [rbp - %d], rax\n", position * 8);
                    current = current->next;
                }
            }

            return current->next;
        }
        else
        {
            fprintf(stderr, "Error: Undeclared identifier %s\n", current->value);
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(current->type, "SYMBOL") == 0 && strcmp(current->value, "(") == 0)
    {
        current = current->next; // sauter "("
        current = parseExpression(current, identifierListe, output_file, boucleCount);
        if (current != NULL && strcmp(current->value, ")") == 0)
        {
            return current->next; // sauter ")"
        }
        fprintf(stderr, "Error: Expected closing parenthesis\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Error: Unexpected token %s\n", current->value);
    exit(EXIT_FAILURE);
}

struct Token *parseTerm(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount)
{
    current = parsePrimary(current, identifierListe, output_file, boucleCount);

    while (current != NULL && strcmp(current->type, "SYMBOL") == 0 && (strcmp(current->value, "*") == 0 || strcmp(current->value, "/") == 0 || strcmp(current->value, "%") == 0 || strcmp(current->value, "**") == 0))
    {
        char op[8];
        strcpy(op, current->value);
        current = current->next;

        fprintf(output_file, "   push rax\n"); // sauvegarder gauche
        current = parsePrimary(current, identifierListe, output_file, boucleCount);
        // rax = droite, stack = gauche

        if (strcmp(op, "*") == 0)
        {
            fprintf(output_file, "   pop rbx\n");
            fprintf(output_file, "   imul rax, rbx\n");
        }
        else if (strcmp(op, "/") == 0)
        {
            fprintf(output_file, "   pop rbx\n");
            fprintf(output_file, "   xchg rax, rbx\n");
            fprintf(output_file, "   cdq\n");
            fprintf(output_file, "   idiv ebx\n");
        }
        else if (strcmp(op, "%") == 0)
        {
            fprintf(output_file, "   pop rbx\n");
            fprintf(output_file, "   xchg rax, rbx\n");
            fprintf(output_file, "   cdq\n");
            fprintf(output_file, "   idiv ebx\n");
            fprintf(output_file, "   mov rax, rdx\n");
        }
        else if (strcmp(op, "**") == 0)
        {
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
    }
    return current;
}

struct Token *parseComparison(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount)
{
    current = parseExpression(current, identifierListe, output_file, boucleCount);

    while (current != NULL && strcmp(current->type, "SYMBOL") == 0 && (strcmp(current->value, "==") == 0 || strcmp(current->value, "!=") == 0 || strcmp(current->value, "<") == 0 || strcmp(current->value, "<=") == 0 || strcmp(current->value, ">") == 0 || strcmp(current->value, ">=") == 0))
    {
        char op[8];
        strcpy(op, current->value);
        current = current->next;

        fprintf(output_file, "   push rax\n");
        current = parseExpression(current, identifierListe, output_file, boucleCount);
        fprintf(output_file, "   pop rbx\n");
        fprintf(output_file, "   cmp rbx, rax\n"); // ← rbx = gauche, rax = droite

        if (strcmp(op, "==") == 0)
            fprintf(output_file, "   sete al\n");
        else if (strcmp(op, "!=") == 0)
            fprintf(output_file, "   setne al\n");
        else if (strcmp(op, "<") == 0)
            fprintf(output_file, "   setl al\n");
        else if (strcmp(op, "<=") == 0)
            fprintf(output_file, "   setle al\n");
        else if (strcmp(op, ">") == 0)
            fprintf(output_file, "   setg al\n");
        else if (strcmp(op, ">=") == 0)
            fprintf(output_file, "   setge al\n");

        fprintf(output_file, "   movzx rax, al\n");
    }
    return current;
}

struct Token *parseExpression(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount)
{
    current = parseTerm(current, identifierListe, output_file, boucleCount);

    while (current != NULL && strcmp(current->type, "SYMBOL") == 0 && (strcmp(current->value, "+") == 0 || strcmp(current->value, "-") == 0))
    {
        char op[8];
        strcpy(op, current->value);
        current = current->next;

        fprintf(output_file, "   push rax\n"); // sauvegarder gauche
        current = parseTerm(current, identifierListe, output_file, boucleCount);
        // rax = droite, stack = gauche

        if (strcmp(op, "+") == 0)
        {
            fprintf(output_file, "   pop rbx\n");
            fprintf(output_file, "   add rax, rbx\n");
        }
        else if (strcmp(op, "-") == 0)
        {
            fprintf(output_file, "   pop rbx\n");
            fprintf(output_file, "   sub rbx, rax\n");
            fprintf(output_file, "   mov rax, rbx\n");
        }
    }
    return current;
}

struct Token *parser(struct Token *current, struct identifierListe identifierListe, FILE *output_file, int *boucleCount)
{
    return parseComparison(current, identifierListe, output_file, boucleCount);
}