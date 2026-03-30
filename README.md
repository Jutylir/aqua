# Aqua Language Documentation

> **Version** : 0.1.0 — Work in progress

---

## Introduction

Aqua est un langage compilé minimaliste dont le compilateur génère de l'assembleur x86-64 (Linux). Il est conçu pour être simple, explicite et bas niveau.

---

## Compilation

```bash
./aqua <fichier.aq>        # compile et génère ./src/output
./src/output               # exécuter le binaire
```

---

## Syntaxe

### Déclarer une variable

```
identifier = entierLittéral
```

**Exemples :**

```
x = 10
age = 25
result = 0
```

- L'identifiant doit commencer par une lettre.
- La valeur doit être un entier littéral (0–254) ou une expression arithmétique.
- Chaque instruction doit être sur sa propre ligne.
- Expressions supportées : `+`, `-`, `*`, `/`, `%`, `**`, `++`, `--`, parenthèses.

---

### Expressions

```
x = 5
x = x + 10
x = x * 2
return x
```


### Retourner une valeur

```
return entierLittéral
return identifier
```

**Exemples :**

```
return 69
```

```
x = 42
return x
```

- La valeur de retour est récupérable via `echo $?` dans le shell.
- La valeur doit être comprise entre **0 et 254**.
- `return` doit être la dernière instruction du programme.

---

## Exemple complet

```
x = 42
return x
```

```bash
./aqua ./src/mon_programme.aq
./src/output
echo $?   # → 42
```

---

## Limitations actuelles (v0.1.0)

- Opérations arithmétiques de base prises en charge (`+`, `-`, `*`, `/`, `%`, `**`).
- Opérateurs unaires `++`, `--` sur expressions.
- Pas de conditions (`if`, `else`).
- Pas de boucles.
- Pas de fonctions.
- Les valeurs sont limitées à des entiers entre 0 et 254.
- Un seul fichier source par compilation.

---

## Assembleur généré

Pour le programme suivant :

```
x = 5
return x
```

Aqua génère :

```nasm
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, 5
    mov [rbp - 8], rax   ; x = 5
    mov rax, 60
    mov rdi, [rbp - 8]   ; return x
    syscall
```

---

*Documentation mise à jour au fur et à mesure du développement du langage.*
