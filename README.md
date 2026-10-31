Min Heap (Tas Min) — Implémentations & Benchmarks
Projet C++17 — Quatre implémentations d'un tas min (Min Heap) comparées sur trois plans : lisibilité, complexité algorithmique et performance réelle mesurée.

Table des matières
Présentation

Structure du projet

Les quatre implémentations

Complexités théoriques

Problématique métier

Résultats des benchmarks

Vue d'ensemble

MinHeapVector / MinHeapRaw — scalabilité

MinHeapMap — scalabilité

MinHeapList — scalabilité

Étudiants boursiers (N = 1 000 000)

Comparaison finale

Interprétation des résultats

Compilation & exécution

Points pédagogiques

Limites & pistes d'amélioration

Présentation
Un tas min (min heap) est un arbre binaire quasi-complet stocké dans un tableau, tel que chaque nœud soit inférieur ou égal à ses deux enfants. Il permet en O(1) d'accéder au minimum et en O(log N) de l'insérer ou de l'extraire.

Ce projet explore le même algorithme implémenté avec quatre structures de stockage différentes — du tableau brut new/delete au std::map — pour illustrer concrètement que le choix de la structure de données conditionne la complexité effective, pas seulement le code.

Fonctionnalités communes
Opération	Description
insert(v)	Ajoute v au tas
top()	Retourne la valeur minimale sans la retirer
pop()	Retire et retourne la valeur minimale (extract-min)
deleteKey(k)	Supprime une clé arbitraire k (première occurrence)
empty() / size()	État du tas
Structure du projet
text
TAS/MinTas/
├── minTas.cpp                      # Version pédagogique (référence lisible)
├── heap_vector.cpp                 # MinHeapVector  (std::vector)
├── heap_raw.cpp                    # MinHeapRaw     (tableau brut new/delete)
├── heap_map.cpp                    # MinHeapMap     (std::map<int,T>)
├── heap_list.cpp                   # MinHeapList    (std::list<T>)
│
├── bench_mintas.cpp                # Bench simple N = 1e6
├── bench_mintas_scale.cpp          # Bench scalabilité (100 → 1e8)
├── bench_heap_vector_scale.cpp     # Bench scalabilité MinHeapVector
├── bench_heap_raw_scale.cpp        # Bench scalabilité MinHeapRaw
├── bench_heap_map_scale.cpp        # Bench scalabilité MinHeapMap
├── bench_heap_list_scale.cpp       # Bench scalabilité MinHeapList
│
└── Problematique_Exemple/
    ├── bench_students_vector.cpp   # Cas métier : étudiants boursiers (vector)
    ├── bench_students_raw.cpp      # Cas métier (raw)
    └── bench_students_map.cpp      # Cas métier (map)
Les quatre implémentations
1. MinHeapVector — référence moderne
Stockage : std::vector<T>. Accès indexé O(1), push_back amorti O(1).

cpp
template <typename T>
class MinHeapVector {
    std::vector<T> arr;
    void heapify(int i) { /* descente O(log n) */ }
public:
    void insert(const T& v);   // O(log n)
    const T& top() const;      // O(1)
    T pop();                   // O(log n)
    bool deleteKey(const T& k);// O(n) recherche + O(log n)
};
2. MinHeapRaw — tableau brut new/delete
Croissance manuelle (×2), copie des éléments, destructeur explicite. Reproduit exactement le comportement de std::vector mais avec une gestion mémoire manuelle — utile pour comprendre l'allocation dynamique et les règles des 3/5.

3. MinHeapMap — std::map<int,T> indexée
La clé entière est l'indice dans le tableau virtuel. Chaque accès est O(log n) → insert/pop deviennent O(log² n). Pédagogique : montre le coût caché d'un accès non contigu.

4. MinHeapList — std::list<T>
Aucun accès aléatoire. Chaque at(i) coûte std::next(begin(), i) en O(i). Conséquence : insert/pop/deleteKey passent en O(n). Utilisé uniquement pour illustrer pourquoi std::list n'est pas adapté aux tas.

Complexités théoriques
Implémentation	insert	top	pop	deleteKey	Mémoire
MinHeapVector	O(log N) amorti	O(1)	O(log N)	O(N)	~N·sizeof(T) + slack
MinHeapRaw	O(log N) amorti	O(1)	O(log N)	O(N)	~N·sizeof(T) (cap ×2)
MinHeapMap	O(log² N)	O(log N)	O(log² N)	O(N)	~N·(sizeof(T)+overhead)
MinHeapList	O(N)	O(1)	O(N)	O(N)	N·(sizeof(T)+2·8)
Note : deleteKey est O(N) dans tous les cas à cause de la recherche linéaire de la clé. Pour la rendre O(log N), il faudrait un index unordered_map<T, index> maintenu en parallèle (hors périmètre).

Problématique métier
Énoncé : gérer N = 1 000 000 d'étudiants boursiers et toujours pouvoir extraire le moins boursier rapidement.

cpp
struct Student {
    int         id;
    std::string name;
    double      scholarship;   // montant en €
    bool operator<(const Student& o) const {
        if (scholarship != o.scholarship) return scholarship < o.scholarship;
        return id < o.id;      // départage stable
    }
    bool operator==(const Student& o) const { return id == o.id; }
};
Le tas est un min-heap sur scholarship (départage par id). Le tas sur Student étant plus lourd (conteneur std::string), il révèle l'impact mémoire + copies en plus de la complexité.

Résultats des benchmarks
Machine : HP EliteBook 850 G8, compilation g++ -std=c++17 -O2. Graine fixe SEED = 42 → reproductibilité.
Mesures en ns/op (nanosecondes par opération) — plus petit = meilleur.

Vue d'ensemble
Opération	Vector	Raw	Map	List
insert	O(log N)	O(log N)	O(log² N)	O(N)
top	O(1)	O(1)	O(log N)	O(1)
pop	O(log N)	O(log N)	O(log² N)	O(N)
deleteKey	O(N)	O(N)	O(N)	O(N)
MinHeapVector / MinHeapRaw — scalabilité
TOP_ITERS = 50 000 000, DEL_CAP = 10 000, budget = 30 s/phase.

MinHeapVector (ns/op)
N	insert	top	pop	deleteKey
100	24.90	0.66	93.08	68.75
1 000	37.45	0.96	87.72	181.02
10 000	25.76	1.11	110.56	1 338.80
100 000	18.42	0.73	149.28	1 233.65
1 000 000	24.89	0.61	189.90	921.76
10 000 000	20.42	0.46	465.25	1 137.78
100 000 000	24.24	0.62	700.13	1 176.53
MinHeapRaw (ns/op)
N	insert	top	pop	deleteKey
100	20.99	0.48	78.50	51.36
1 000	25.78	0.52	84.99	189.39
10 000	26.42	0.51	122.16	1 144.05
100 000	22.86	0.56	157.32	1 151.20
1 000 000	20.97	0.48	155.55	894.16
10 000 000	17.18	0.64	307.03	934.59
100 000 000	20.04	1.22	627.24	1 082.72
Observation graphique — insert reste plat (~20 ns), top constant (~0.5 ns, dominé par le cache L1), pop croît lentement (log N → de ~80 à ~700 ns sur 6 ordres de grandeur), deleteKey explose dès N ≥ 10⁴ (recherche linéaire → ~1 µs/op, plafonné par DEL_CAP = 10 000). Le run N = 1e8 a été abandonné (pop ABORT après 30 s) : ~42–47 millions de pops sur 50 millions visés.

text
insert  ▁▁▁▁▁▁▁  (~20 ns, log N amorti)
top     ▁▁▁▁▁▁▁  (~0.5 ns, O(1))
pop     ▂▂▃▃▄▄▅  (croissance log)
deleteKey ▁▂████  (saut à N=10⁴ → O(N))
MinHeapMap — scalabilité
TOP_ITERS = 5 000 000, DEL_CAP = 5 000, budget = 15 s/phase.

N	insert	top	pop	deleteKey
100	384.46	5.44	1 128.98	800.68
1 000	474.20	10.12	2 276.72	1 939.95
10 000	666.85	12.47	3 913.64	9 647.39
100 000	1 100.79	27.47	10 016.81	9 267.05
1 000 000	1 680.47	45.06	17 772.47	8 508.00
Observation : top est ~90× plus lent que sur vector (45 vs 0.5 ns à N=10⁶) — le tree.at(0) traverse un arbre rouge-noir. pop atteint 17.8 µs/op à N=10⁶ (contre 156–190 ns en vector, soit ×100 à ×110). La recherche deleteKey reste O(N) mais avec un surcoût de parcours std::map (~9 µs/op).

MinHeapList — scalabilité
TOP_ITERS = 5 000 000, DEL_CAP = 2 000, budget = 5 s/phase, tailles plafonnées à 1e5.

N	insert	top	pop	deleteKey	Statut
100	181.35	0.49	516.62	259.62	OK
300	630.95	0.60	1 689.33	835.13	OK
1 000	1 964.44	0.50	8 331.83	3 799.61	OK
3 000	9 058.12	0.95	27 775.36	7 773.94	OK
10 000	26 660.87	0.48	115 251.55	8 801.36	OK
30 000	93 920.02	0.45	391 240.59	8 727.00	pop ABORT
100 000	123 505.75	2.21	643 574.40	9 346.90	insert+pop ABORT
Observation : le coût explose linéairement (×10 par décade → confirmé par les ratios ~2.7–4.9 sur les petites tailles). À N = 10⁵, un seul insert prend 124 µs — un vector fait la même chose en ~20 ns, soit ×6 000 plus rapide. Les ABORT prouvent que la structure ne passe pas à l'échelle.

Étudiants boursiers (N = 1 000 000)
Implémentation	insert total	top ×iters	pop 500k	deleteKey
Vector	66.57 ms (67 ns/op)	25.13 ms / 50M (0.50 ns)	364.95 ms (730 ns/op)	11 406 ms (1.14 ms/op, 10k, 4 984)
Raw	71.89 ms (72 ns/op)	24.46 ms / 50M (0.49 ns)	591.67 ms (1 183 ns/op)	13 135 ms (1.31 ms/op, 10k, 4 984)
Map	1 462.34 ms (1.46 µs/op)	234.29 ms / 5M (46.86 ns)	11 722.99 ms (23.4 µs/op)	38 538 ms (7.71 ms/op, 5k, 2 508)
Interprétation métier :

vector et raw : proches (écart < 10 % sur insert/top, ~60 % sur pop — ce dernier est plus sensible au bruit et aux copies de Student). Le choix new/delete vs std::vector n'apporte rien en perf à ce niveau, seulement de la complexité de code.

map : 22× plus lent à l'insertion, ~32× plus lent au pop, ~94× plus lent au top — pour un résultat fonctionnellement identique. La recherche linéaire de deleteKey dans une std::map coûte ~7.7 ms/op (contre ~1.1 ms en vector), car elle parcourt N nœuds non contigus → cache misses massifs.

Somme des bourses extraites identique (1 249 507 804 €) dans les trois cas → validation croisée de la correction.

Comparaison finale
text
N = 1 000 000 — ns/op (plus petit = meilleur)
─────────────────────────────────────────────────────────
                Vector      Raw         Map         List
insert          24.89      20.97     1 680.47    123 506*   (aborted)
top              0.61       0.48        45.06        0.62
pop            189.90     155.55    17 772.47    618 292*   (aborted)
deleteKey      921.76     894.16     8 508.00      8 727
─────────────────────────────────────────────────────────
* valeurs non comparables : budget temps dépassé avant N complet
Métrique	Gagnant	Facteur vs Vector
insert	Vector ≈ Raw	—
top	Vector ≈ Raw ≈ List	— (toutes O(1))
pop	Vector ≈ Raw	—
deleteKey	Vector ≈ Raw	—
std::list à N=10⁵	—	×6 000 plus lent
std::map à N=10⁶	—	×22 à ×110 plus lent
Interprétation des résultats
1. top() est vraiment O(1) — et c'est visible
Sur vector, top reste entre 0.4 et 1.1 ns/op sur 6 ordres de grandeur. C'est le cache L1 qui gagne : arr[0] est toujours en cache chaud. En std::map, tree.at(0) traverse un arbre rouge-noir → ~45 ns/op à N=10⁶ (×75 à ×100).

2. insert en O(log N) amorti : quasi-plat
Théoriquement, insert fait log₂(N) comparaisons. Entre N=100 (≈7 niveaux) et N=10⁸ (≈27 niveaux), on attend un facteur ~4. Mesuré : 16 → 24 ns (~1.5). Le coût est dominé par le cache et le push_back, pas par le nombre de comparaisons (qui sont des swap de 4 octets gratuits).

3. deleteKey est le vrai goulot
Le passage de ~50 ns (petits N) à ~1 µs (grands N) illustre O(N) : à N=10⁴, 10 000 comparaisons ; à N=10⁶, 1 000 000. Le plafond de DEL_CAP dans les benchmarks est précisément là pour éviter des runs de plusieurs minutes. Leçon : si deleteKey est fréquent, il faut un index secondaire.

4. std::list est disqualifié pour les tas
Avec std::next(begin(), i), un accès au niveau k coûte O(2^k) → l'insertion coûte O(N) en moyenne. Les abandons (ABORT) à N=30 000 et 100 000 sont la preuve empirique du mur.

5. std::map : O(log² N) coûte cher en pratique
Le facteur ×100 sur pop à N=10⁶ vient de log²(10⁶) ≈ 400 opérations de pointeurs par pop, contre log₂(10⁶) ≈ 20 en vector. La théorie se vérifie brutalement.

6. Vector vs Raw : match nul
Performances statistiquement proches (écarts souvent < 20 %, sensibles au bruit et aux copies de T). std::vector apporte la sécurité (copie, move, exceptions) sans surcoût significatif à ce niveau d'optimisation (-O2 inline tout). Raw n'est utile que comme exercice pédagogique.

7. Min Heap vs Max Heap : symétrie parfaite
Les chiffres obtenus sur le min heap sont quasi identiques à ceux du max heap (mêmes ordres de grandeur, mêmes formes de courbes). C'est attendu : la seule différence est le sens de la comparaison (< au lieu de >), qui n'affecte ni le nombre d'opérations ni la localité mémoire.

Compilation & exécution
Prérequis
g++ ≥ 9 (C++17) ou clang++ ≥ 10

~1 Go de RAM disponible pour les runs à N = 1e8

Implémentations (démonstration)
bash
g++ -std=c++17 -O2 -Wall -Wextra -o minTas        minTas.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_vector   heap_vector.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_raw      heap_raw.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_map      heap_map.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_list     heap_list.cpp

./minTas
./heap_vector
./heap_raw
./heap_map
./heap_list
Benchmarks
bash
# Bench simple (N = 1e6)
g++ -std=c++17 -O2 -Wall -Wextra -o bench_mintas bench_mintas.cpp
./bench_mintas

# Bench scalabilité (100 → 1e8)
g++ -std=c++17 -O2 -Wall -Wextra -o bench_mintas_scale       bench_mintas_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_vector_scale  bench_heap_vector_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_raw_scale     bench_heap_raw_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_map_scale     bench_heap_map_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_list_scale    bench_heap_list_scale.cpp

./bench_mintas_scale
./bench_heap_vector_scale
./bench_heap_raw_scale
./bench_heap_map_scale
./bench_heap_list_scale
Cas métier (étudiants)
bash
cd Problematique_Exemple
g++ -std=c++17 -O2 -Wall -Wextra -o bench_students_vector bench_students_vector.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_students_raw    bench_students_raw.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_students_map    bench_students_map.cpp

./bench_students_vector
./bench_students_raw
./bench_students_map
Configuration
Toutes les tailles/paramètres sont des constantes en tête de fichier (pas d'arguments CLI, sauf bench_mintas.cpp et bench_mintas_scale.cpp qui acceptent des tailles en argv) :

cpp
static const std::vector<std::size_t> SIZES = { 100, 1'000, 10'000, 100'000,
                                                1'000'000, 10'000'000, 100'000'000 };
static const double BUDGET_MS   = 30000.0;   // budget max par phase
static const std::size_t DEL_CAP = 10'000;   // cap deleteKey
static const std::size_t TOP_ITERS = 50'000'000;
static const unsigned SEED = 42;
Points pédagogiques
La complexité asymptotique est un guide, pas une garantie.
O(log N) sur std::map devient O(log² N) à cause de l'accès non contigu — la théorie le prédit, les chiffres le confirment (×100 sur pop).

Le cache est roi.
vector et raw restent à ~20 ns/insert sur 6 ordres de grandeur parce que la mémoire est contiguë. map paie chaque saut de pointeur.

deleteKey en O(N) est un piège classique.
À N=10⁶, une seule suppression coûte ~1 ms — invisible sur un N de test, rédhibitoire en production.

std::list n'est pas un "vector avec insertion rapide".
Sans accès aléatoire, il est structurellement inadapté aux tas.

std::vector bat new/delete en sécurité, à égalité en perf.
Aucune raison technique de préférer le tableau brut, sauf exercice.

Les benchmarks doivent avoir des garde-fous.
Les BUDGET_MS et DEL_CAP évitent qu'un run pédagogique ne prenne 3 heures — et les ABORT documentent honnêtement les limites.

Min Heap ↔ Max Heap : même coût, code miroir.
Remplacer < par > ne change ni la complexité, ni les performances. Un tas min est un tas max « retourné » — le reste est identique (structure, cache, localité).

Limites & pistes d'amélioration
Limite	Piste
deleteKey en O(N)	Index std::unordered_map<T, size_t> maintenu → O(log N)
Student recopié à chaque swap	std::vector<std::unique_ptr<Student>> ou heap sur id + table annexe
Pas de buildHeap en O(N) mesuré	Ajouter MinHeap::MinHeap(vector<T>) + bench dédié (déjà présent dans minTas.cpp, non benchmarké)
Pas de decreaseKey / increaseKey	Utile pour Dijkstra / Prim — à implémenter
Pas de test unitaire	Ajouter Catch2 ou GoogleTest avec cas aléatoires
Bench monothread	std::execution::par sur la génération, ou bench concurrent
Pas de profiling cache	Utiliser perf stat -e cache-misses ou valgrind --tool=cachegrind
Pas de mesure mémoire	valgrind --tool=massif ou /usr/bin/time -v
pop sur N = 1e8 non terminé dans le budget	Augmenter BUDGET_MS ou paralléliser, ou réduire half
Licence
Projet pédagogique — ILISI2, Virtualisation, C++ — libre d'utilisation à des fins d'apprentissage.

<p align="center"> <sub>Généré à partir du dépôt <code>TAS/MinTas/</code> — benchmarks reproductibles avec <code>SEED = 42</code></sub> </p>
