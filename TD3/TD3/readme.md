# Projet TD3 - Programmation Multithread & RTOS

## Instructions de compilation et d’exécution

### Compilation

Chaque exercice peut être compilé indépendamment avec `g++` :

```bash
g++ -std=c++17 -pthread a1.cpp Thread.cpp Mutex.cpp -o a1
g++ -std=c++17 -pthread a2.cpp Thread.cpp Mutex.cpp -o a2
g++ -std=c++17 -pthread a3.cpp Thread.cpp Mutex.cpp -o a3
g++ -std=c++17 -pthread b.cpp Thread.cpp Mutex.cpp -o b
g++ -std=c++17 -pthread c.cpp Thread.cpp Mutex.cpp -o c
g++ -std=c++17 -pthread d.cpp Thread.cpp Mutex.cpp Counter.cpp Incrementer.cpp -o d
g++ -std=c++17 -pthread e.cpp Thread.cpp Mutex.cpp -o e
```

---

### Exécution

Exemples :

```bash
./a1
./b
./c

./d 1000000 4
./d 1000000 4 protect SCHED_FIFO

./e
```

---

## Réponses aux questions des TD

### TD3 - Threads et synchronisation

---

### 1. Différence entre exécution séquentielle et parallèle

Les programmes `a1`, `a2`, `a3` illustrent différentes stratégies :

* **a1** : exécution concurrente sans synchronisation explicite
* **a2** : gestion explicite du temps avec attente active
* **a3** : meilleure structuration avec contrôle du temps

Conclusion :
La concurrence permet une meilleure utilisation du CPU, mais nécessite un contrôle précis du timing.

---

### 2. Importance de la synchronisation (Mutex)

Dans les exercices `b` et `c`, on observe :

* Sans mutex → **conditions de course (race condition)**
* Avec mutex → accès protégé → résultat correct

Conclusion :
Le mutex garantit l’exclusion mutuelle et la cohérence des données partagées.

---

### 3. Compteur partagé (TD d)

Le programme `d.cpp` met en œuvre :

* plusieurs threads (`Incrementer`)
* un compteur partagé (`Counter`)

#### Sans protection :

* résultat incorrect
* incréments perdus

#### Avec protection (`protect`) :

* utilisation de `Mutex`
* résultat correct et déterministe

Conclusion :
La synchronisation est indispensable pour les ressources partagées.

---

### 4. Influence de la politique d’ordonnancement

On peut tester :

* `SCHED_OTHER`
* `SCHED_RR`
* `SCHED_FIFO`

Observation :

* `SCHED_FIFO` donne plus de contrôle sur les priorités
* comportement plus prévisible (temps réel)

---

### 5. Problème d’inversion de priorité (TD e)

Le programme `e.cpp` démontre le phénomène de **priority inversion**.

#### Cas 1 : sans héritage de priorité

* Thread basse priorité détient le mutex
* Thread haute priorité attend
* Thread moyenne priorité monopolise le CPU

Résultat :

* blocage du thread prioritaire
* latence importante

---

#### Cas 2 : avec héritage de priorité (Priority Inheritance)

* Mutex avec `PTHREAD_PRIO_INHERIT`
* le thread basse priorité hérite temporairement d’une priorité élevée

Résultat :

* libération rapide du mutex
* réduction de la latence

---

### Conclusion générale

* Les systèmes concurrents nécessitent une synchronisation rigoureuse
* Les politiques d’ordonnancement influencent fortement les performances
* Le mécanisme de **priority inheritance** est essentiel en systèmes temps réel

---

## Choix de conception

### Encapsulation des primitives POSIX

* `Thread` encapsule `pthread`
* `Mutex` encapsule `pthread_mutex`

Avantage :

* code plus lisible
* abstraction claire

---

### Utilisation de RAII

Classe interne `Mutex::Lock` :

* verrouillage automatique à la construction
* déverrouillage automatique à la destruction

Avantage :

* évite les oublis de `unlock`
* sécurité du code

---

### Modularité

* séparation en classes (`Thread`, `Mutex`, `Counter`, `Incrementer`)
* réutilisation facile

---

### Simplicité

* pas de complexité inutile
* respect strict des énoncés

---

## Remarques finales

* Tous les programmes compilent sans erreurs ni warnings
* Les conventions de codage sont respectées
* Les résultats expérimentaux confirment les concepts théoriques

---

## Structure du projet

```
.
├── a1.cpp a2.cpp a3.cpp
├── b.cpp c.cpp d.cpp e.cpp
├── Thread.cpp / Thread.h
├── Mutex.cpp / Mutex.h
├── Counter.cpp / Counter.h
├── Incrementer.cpp / Incrementer.h
├── Chrono.h
├── TimespecUtils.h
└── README.md
```