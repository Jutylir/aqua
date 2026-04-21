struct identifier
{
    char name[1024];
    int stackPos;
    char type[1024];
    struct identifier *next;
};

struct identifierListe
{
    struct identifier *head;
};

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