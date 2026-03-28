# TD1 – Mesure du temps Posix

## 1. Compilation / exécution

Le projet a déjà été compilé avec CMake, donc le plus simple :

cd build  
./td1  

---

Si on veut recompiler proprement :

rm -rf build  
mkdir build  
cd build  
cmake ..  
make  
./td1  

---

Sans CMake ça marche aussi :

g++ -std=c++17 main.cpp timespec_utils.cpp chrono.cpp -o td1  
./td1  

---

## 2. a) Partie timespec

### Conversion ms → timespec

Fonction :

timespec_from_ms(double time_ms)

Idée :

- 1000 ms = 1 seconde
- le reste → en nanosecondes

Donc :

tv_sec = partie entière  
tv_nsec = reste × 1e6  

Le point important ici c’est la normalisation :

- tv_nsec doit toujours être entre 0 et 1e9
- même si le temps est négatif

Exemples :

2700 ms → 2 s + 700000000 ns  
-2700 ms → -3 s + 300000000 ns  

---

### Conversion timespec → ms

timespec_to_ms(const timespec& ts)

Formule directe :

tv_sec * 1000 + tv_nsec / 1e6  

---

### Temps courant

timespec_now()

Utilisation de :

clock_gettime(CLOCK_MONOTONIC, ...)

J’ai choisi MONOTONIC parce que :

- indépendant de l’horloge système
- plus adapté pour mesurer des durées

---

### Opposé

timespec_negate()

On change juste le signe, puis on normalise.

---

### Addition / soustraction

timespec_add  
timespec_subtract  

Addition directe.

Soustraction :

t1 - t2 = t1 + (-t2)

Puis normalisation.

---

### nanosleep

timespec_wait()

Attention ici :

nanosleep peut être interrompu (EINTR)

Donc :

- si interrompu
- on reprend avec le temps restant

---

### Opérateurs

Implémentés :

- -t
- +
- -
- +=
- -=
- ==, !=
- <, >

Comparaison :

- d’abord tv_sec
- puis tv_nsec

---

### Tests dans main

J’ai testé :

- conversions (positif / négatif)
- addition / soustraction
- comparaisons
- sleep
- chrono

---

## 3. b) Classe Chrono

C’est un chrono assez simple.

Variables :

- m_startTime
- m_stopTime

---

### restart()

- initialise startTime avec le temps courant
- stopTime pareil

---

### stop()

- enregistre le temps courant
- retourne la durée depuis restart()

---

### isActive()

- vrai si pas encore stoppé

---

### lap()

Deux cas :

- si actif → maintenant - start
- sinon → stop - start

---

### lap_ms()

Même chose que lap mais en millisecondes.

---

## 4. Tests

Dans le main :

- conversion correcte
- gestion du négatif
- addition / soustraction
- nanosleep
- chrono (lap / stop)

Les résultats sont cohérents.

---

## 5. Remarque

Le plus important dans ce TD c’est surtout :

- bien gérer la normalisation
- faire attention aux temps négatifs
- gérer correctement nanosleep

Le reste est assez direct.