/* TODO:
Pouvoir déclarer des listes/tableaux.
-> Création de la fonction print() pour pouvoir debugger
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "headers/tokenizer.h"
#include "headers/structs.h"
#include "headers/generator.h"
#include "headers/parser.h"

void afficherTokens(struct TokenListe *liste)
{
    struct Token *current = liste->head;
    while (current != NULL)
    {
        printf("Type: %s, Value: %s\n", current->type, current->value);
        current = current->next;
    }
}

void freeTokenList(struct TokenListe *liste) {
    struct Token *current = liste->head;
    while (current != NULL) {
        struct Token *next = current->next;
        free(current);
        current = next;
    }
    liste->head = NULL;
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

    struct TokenListe tokenList = tokenizer(input_file);

    // Display the tokens
    //afficherTokens(&tokenList);

    generator(input_file, tokenList);

    freeTokenList(&tokenList);

    system("nasm -f elf64 ./src/output.asm -o ./src/output.o");
    system("ld ./src/output.o -o ./src/output -no-pie");

    // Close the input file
    fclose(input_file);
    return 0;
}