# Aqua Language Documentation

> **Version** : 0.3.0 — Work in progress

---

## Introduction

Aqua est un langage compilé minimaliste dont le compilateur génère de l'assembleur x86-64 (Linux). Il vise la simplicité et l'apprentissage de la compilation basse niveau.

---

## Compilation et exécution

```bash
# Compiler le fichier source
./build/aqua <fichier.aq>

# Exécuter le binaire généré
./src/output

# Valeur de retour
echo $?
```

La sortie assembleur est dans `./src/output.asm` et le binaire final est `./src/output`.

---

## Syntaxe

### Variables et assignation

- Identifiant : lettre initiale + lettres/chiffres (`[a-zA-Z][a-zA-Z0-9]*`).
- Assignation : `identifiant = expression`.
- Une variable est déclarée à la première affectation.

```
x = 10
y = x + 5
```

### Expressions

- Nombres : `0`, `1`, `42`, ...
- Identifiants : `x`, `result`, ...
- Parenthèses : `( ... )`
- Arithmétique : `+`, `-`, `*`, `/`, `%`, `**`
- Comparaisons : `==`, `!=`, `<`, `<=`, `>`, `>=`
- Postfixe : `x++`, `x--`

```
x = (2 + 3) * 4
x = x ** 3
x = x % 5
flag = x > 10
```

### Contrôle de flux

- `if (cond) { ... } [else { ... }]`
- `while (cond) { ... }`
- `for (i = init; cond; incr) { ... }`

Exemple :

```
i = 0
sum = 0
while (i < 5) {
    sum = sum + i
    i++
}
if (sum >= 10) {
    sum = sum + 1
} else {
    sum = sum + 2
}
return sum
```

### Retour

```
return expression
```

- Le programme termine via syscall `exit` (n° 60).
- Valeur de retour 0–255 (shell `echo $?`).

---

## Gestion de la pile

- `rbp` et `rsp` sont configurés au début (`push rbp`, `mov rbp,rsp`).
- Les assignations sauvegardent les valeurs sur la pile (`push rax`) et lisent avec des offsets (`[rbp - N]`).
- Les blocs `{}` libèrent les variables locales (`pop rax`).

---

## Limitations actuelles

- Pas de fonctions nommées (programme mono-bloc / entrée unique).
- Pas de types autres qu'entier (ato sur 64 bits signés).
- Gestion simple des erreurs : expression invalide, identifiant non déclaré, bloc mal fermés.

---

## Exemple simple

```
x = 42
return x
```

---

## Exemple avancé

```
result = 0
for (i = 0; i < 5; i++) {
    result = result + i
}
return result
```

---

## Génération d'assembleur

Le compilateur produit `src/output.asm` en x86-64 NASM, puis invoque :

```bash
nasm -f elf64 ./src/output.asm -o ./src/output.o
ld ./src/output.o -o ./src/output -no-pie
```

---

*Documentation mise à jour au fur et à mesure du développement du langage.*
