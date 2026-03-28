# CSC_5RO05_TA_TD02

## Mini-rapport

Ce projet regroupe les exercices du TD 02 autour de la mesure du temps, des temporisateurs POSIX et de l'etallonage d'une boucle CPU. L'objectif general est de passer d'une utilisation directe des timers systeme a une abstraction orientee objet, puis d'utiliser cette abstraction pour executer une charge CPU pendant une duree demandee.

Le code est organise de maniere progressive :

- `td2a.cpp` : manipulation directe d'un timer POSIX.
- `td2b.cpp` : encapsulation du timer dans une classe `Timer`, puis specialisation avec `CountDown`.
- `td2c.cpp` : creation d'une boucle CPU simple (`Looper`) et mesure de son temps d'execution.
- `td2d.cpp` : calibration de la boucle CPU par echantillonnage periodique et regression lineaire.
- `td2e.cpp` : execution d'une boucle CPU pendant une duree cible grace a la calibration precedente.

## Structure du projet

Le repertoire `src/` contient :

- `Timer.*` : classe abstraite encapsulant `timer_create`, `timer_settime` et la reception du signal.
- `CountDown.*` : exemple simple de classe derivee de `Timer`.
- `Looper.*` : boucle active incrementant un compteur jusqu'a un nombre d'iterations donne.
- `Chrono.*` : chronometre base sur `CLOCK_MONOTONIC`.
- `Calibrator.*` : estimation du nombre d'iterations necessaires pour une duree donnee.
- `CpuLoop.*` : interface de haut niveau pour executer la boucle pendant un temps vise.
- `timespec.*` : fonctions utilitaires pour convertir, additionner, soustraire et attendre sur des `timespec`.
- `makefile` : cibles de compilation locale et cross-compilation Raspberry Pi.

## Reponses et explications par exercice

### TD2A - Timer POSIX minimal

`td2a.cpp` montre le mecanisme de base :

- installation d'un gestionnaire de signal avec `sigaction`,
- creation d'un timer via `timer_create`,
- declenchement periodique toutes les 500 ms,
- incrementation d'un compteur transmis dans `sigev_value`.

Cet exercice valide la comprehension du modele evenementiel : le travail n'est pas fait dans la boucle principale, mais dans le handler appele par le noyau a chaque expiration.

### TD2B - Abstraction objet avec `Timer` et `CountDown`

L'etape suivante consiste a encapsuler le timer dans une classe abstraite `Timer`. Cette classe :

- initialise le handler commun,
- cree le timer dans le constructeur,
- propose `start`, `start_ms` et `stop`,
- delegue le comportement applicatif a la methode virtuelle `callback()`.

`CountDown` herite de `Timer` et redefinit `callback()` pour afficher puis decrementer le compteur. Cette solution est plus propre que `td2a` car elle separe l'infrastructure de temporisation du comportement metier.

### TD2C - Boucle CPU et mesure de duree

`Looper` implemente une boucle active tres simple qui incremente `iLoop` jusqu'a atteindre `nLoops` ou jusqu'a l'appel de `stopLoop()`. `td2c.cpp` mesure ensuite le temps d'execution de cette boucle avec `clock_gettime(CLOCK_MONOTONIC, ...)`.

L'idee importante ici est qu'une boucle vide peut servir de charge CPU reproductible. En pratique, sa duree depend de la machine, de la frequence CPU et de la charge systeme ; il faut donc la calibrer avant de viser une duree precise.

### TD2D - Calibration par echantillonnage

`Calibrator` combine `Timer` et `Looper` :

- un timer periodique est lance avec une periode d'echantillonnage fixe,
- pendant ce temps, `Looper::runLoop(__DBL_MAX__)` tourne en continu,
- a chaque callback, on memorise la valeur courante du compteur,
- apres `nSamples` mesures, on arrete la boucle et le timer,
- on calcule ensuite une regression lineaire `y = a x + b`.

Ici, `x` represente le temps ecoule en millisecondes et `y` le nombre d'iterations observees. La methode `nLoops(duration_ms)` retourne alors l'estimation du nombre de tours necessaires pour occuper le CPU pendant `duration_ms`.

Ce choix est pertinent car, pour une boucle aussi simple, la relation entre temps et nombre d'iterations est approximativement lineaire sur une machine stable.

### TD2E - Execution pendant une duree cible

`CpuLoop` reutilise l'objet `Calibrator` pour transformer une duree demandee en nombre d'iterations. La methode `runTime(duration_ms)` :

- calcule `nLoops` a partir du modele lineaire,
- execute la boucle CPU,
- mesure la duree reelle avec `Chrono`,
- retourne l'erreur relative.

`td2e.cpp` balaie plusieurs durees entre 0.5 s et 10 s, puis affiche pour chaque cas la duree demandee, la duree mesuree et l'erreur relative. L'objectif est de verifier que la calibration reste raisonnablement precise sur une plage de temps assez large.

## Choix de conception

Les principaux choix de conception sont les suivants :

- utiliser `CLOCK_MONOTONIC` dans `Chrono` pour mesurer des durees sans etre perturbe par un changement de l'heure systeme ;
- factoriser les operations sur `timespec` pour eviter les conversions dispersees dans le code ;
- garder `Looper` volontairement simple afin que la calibration mesure surtout le cout de la boucle elle-meme ;
- separer clairement les responsabilites : temporisation (`Timer`), charge CPU (`Looper`), mesure (`Chrono`), estimation (`Calibrator`) et execution pilotee par une duree (`CpuLoop`).

Cette decomposition rend le code plus lisible et facilite les tests module par module.

## Compilation

Depuis `src/` :

```bash
make all
```

Pour une cible specifique :

```bash
make td2a
make td2b
make td2c
make td2d
make td2e
```

Le `makefile` prevoit aussi une cross-compilation via `arm-linux-gnueabihf-g++` pour Raspberry Pi.

Attention : ce projet utilise des fonctionnalites POSIX (`sigaction`, `SIGRTMIN`, `timer_create`, `timer_settime`). Une compilation sous Windows avec MinGW echoue donc normalement. La compilation doit etre faite sur Linux, ou avec une toolchain fournissant ces API POSIX.

## Execution

Exemples d'utilisation :

```bash
./td2a
./td2b
./td2c 100000000
./td2d
./td2e
```

## Remarques

- Le projet s'appuie sur les timers POSIX et vise donc principalement un environnement Linux.
- La precision de `Calibrator` et `CpuLoop` depend de la charge machine au moment de l'execution.
- La boucle utilise une attente active : c'est volontaire pour mesurer une consommation CPU, mais ce n'est pas une solution adaptee a une attente economique en energie.
