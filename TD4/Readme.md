# TD4 – Encapsulation d’une condition Posix

## 1. Exécution

Le projet est déjà compilé avec CMake.

Donc le plus simple :

cd build
./semaphore_test 2 2 100

(2 consumers, 2 producers, 100 tokens chacun)

---

Si besoin de recompiler :

rm -rf build
mkdir build
cd build
cmake ..
make

---

Il y a aussi des petits tests :

./test_monitor
./wait

---

## 2. a) Classe Monitor

Le but est d’encapsuler une condition POSIX (pthread_cond_t) avec un mutex.

La classe Monitor contient :

- un pthread_cond_t
- une référence vers un Mutex

Le cond est initialisé avec pthread_cond_init et détruit à la fin.

---

### notify / notifyAll

- notify → pthread_cond_signal
- notifyAll → pthread_cond_broadcast

---

### classe interne Lock

La classe Monitor::Lock hérite de Mutex::Lock.

Donc :

- le mutex est automatiquement verrouillé
- pas besoin de le gérer à la main

---

### wait()

Utilise pthread_cond_wait :

- libère le mutex
- attend le signal
- reprend le mutex au réveil

---

### wait(timeout)

Utilise pthread_cond_timedwait.

On calcule une deadline avec :

timespec_now() + timespec_from_ms(timeout)

Résultat :

- true si réveillé
- false si timeout

---

### remarque

wait est dans Lock car le mutex doit être déjà verrouillé  
notify est dans Monitor

---

## 3. b) Classe Semaphore

On implémente un sémaphore classique.

Attributs :

- counter → nombre de jetons
- maxCount → capacité max
- mutex
- monitor (notEmpty)

---

### constructeur

Vérifie :

- maxValue >= 1
- initValue <= maxValue

sinon exception.

---

### give()

Si counter < maxCount :

- on incrémente
- on fait notify()

Sinon rien (saturation)

---

### take()

Cas standard :

while (counter == 0)
{
    wait();
}

--counter;

Important :

on utilise while et pas if pour éviter les problèmes de réveil.

---

### take(timeout)

Même logique mais avec timeout.

On calcule :

deadline = now + timeout

Puis boucle :

- si timeout → false
- sinon wait(remaining)

Quand un token arrive :

- counter--
- return true

---

## 4. Producteurs / Consommateurs

Dans le main :

- les producers appellent give()
- les consumers appellent take(500)

Les consumers s’arrêtent quand il n’y a plus de tokens (timeout).

---

### fin du programme

On affiche :

- nombre de tokens produits
- nombre de tokens consommés

et on vérifie :

total produced == total consumed

---

## 5. Tests Monitor

test_monitor :

- test simple de notify / notifyAll

test_wait :

- un thread attend
- le main envoie un notify
- le thread se réveille

---

## 6. Remarques

Ce TD sert surtout à comprendre :

- le lien entre mutex et condition
- pourquoi wait libère le mutex
- l’utilisation de while (et pas if)
- la gestion du timeout

Les points un peu sensibles :

- éviter les deadlocks
- bien calculer les temps restants
- gérer correctement les réveils