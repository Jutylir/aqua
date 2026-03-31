# Aqua Language Documentation

> **Version** : 0.2.0 — Work in progress


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

- L'identifiant doit commencer par une lettre et peut contenir des lettres et des chiffres.
- La valeur peut être un entier littéral (0–254), un identifiant existant ou une expression.
- Chaque instruction doit être sur sa propre ligne.
- Opérateurs arithmétiques supportés : `+`, `-`, `*`, `/`, `%`, `**`, parenthèses.
- Opérateurs de comparaison supportés : `==`, `!=`, `<`, `<=`, `>`, `>=` (résultat 0/1).
- Opérateurs unaires supportés en suffixe : `++` et `--`.

---

### Expressions

```
x = 5
x = x + 10
x = x * 2
x = x - 3
x = (x + 1) * 2
x = 8 > 7          ; valeur 1
y = x == 1         ; valeur 1 ou 0
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


- Pas de conditions (`if`, `else`) au niveau du flux de contrôle.
- Pas de fonctions multi-lignes (une seule entrée/exécution).
- Les valeurs attendues pour `return` restent 0–254 pour correspondre à `echo $?`.
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
    mov rax, 5
    push rax              ; x = 5 (stack variable)
    mov rax, 60
    mov rdi, [rbp - 8]   ; return x
    syscall
```

---

*Documentation mise à jour au fur et à mesure du développement du langage.*
