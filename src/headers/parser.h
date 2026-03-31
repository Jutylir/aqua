#ifndef PARSER_H
#define PARSER_H

struct Token *parser(struct Token *current, struct identifierListe *identifierListe, FILE *output_file, int *boucleCount);

#endif