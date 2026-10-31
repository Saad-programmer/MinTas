# 🔺 Min Heap (Tas Min) — Implémentations & Benchmarks

<div align="center">

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![g++](https://img.shields.io/badge/g++-O2-A22846?style=for-the-badge&logo=gnu&logoColor=white)
![Status](https://img.shields.io/badge/status-complete-success?style=for-the-badge)
![License](https://img.shields.io/badge/license-educational-blue?style=for-the-badge)

**Quatre implémentations d'un tas min comparées sur trois plans : *lisibilité*, *complexité algorithmique* et *performance réelle* mesurée.**

[📖 Présentation](#-présentation) · [🏗️ Structure](#-structure-du-projet) · [⚙️ Implémentations](#-les-quatre-implémentations) · [📊 Résultats](#-résultats-des-benchmarks) · [🧠 Analyse](#-interprétation-des-résultats) · [🚀 Build](#-compilation--exécution)

</div>

---

## 📌 Table des matières

<details open>
<summary><b>Cliquez pour déplier / replier</b></summary>

1. [Présentation](#-présentation)
2. [Structure du projet](#-structure-du-projet)
3. [Les quatre implémentations](#-les-quatre-implémentations)
4. [Complexités théoriques](#-complexités-théoriques)
5. [Problématique métier](#-problématique-métier)
6. [Résultats des benchmarks](#-résultats-des-benchmarks)
7. [Interprétation des résultats](#-interprétation-des-résultats)
8. [Compilation & exécution](#-compilation--exécution)
9. [Points pédagogiques](#-points-pédagogiques)
10. [Limites & pistes](#-limites--pistes-damélioration)

</details>

---

## 🎯 Présentation

> Un **tas min** (*min heap*) est un arbre binaire quasi-complet stocké dans un tableau, tel que chaque nœud soit **inférieur ou égal** à ses deux enfants.

| Propriété | Coût |
|:---|:---:|
| Accès au minimum | `O(1)` |
| Insertion | `O(log N)` |
| Extraction | `O(log N)` |

Ce projet implémente **le même algorithme** avec **quatre structures de stockage différentes** — du tableau brut `new/delete` au `std::map` — pour illustrer que **le choix de la structure conditionne la complexité effective**, pas seulement le code.

### 🧩 API commune

```cpp
insert(const T& v);    // ajoute v
top() const;           // retourne le min (sans retirer)
pop();                 // retire et retourne le min (extract-min)
deleteKey(const T& k); // supprime une clé (1ère occurrence)
empty() / size();
```

---

## 🗂️ Structure du projet

```text
TAS/MinTas/
│
├── 🎓 minTas.cpp                    ← version pédagogique (référence)
│
├── 📦 Implémentations
│   ├── heap_vector.cpp              ← MinHeapVector  (std::vector)
│   ├── heap_raw.cpp                 ← MinHeapRaw     (new/delete)
│   ├── heap_map.cpp                 ← MinHeapMap     (std::map<int,T>)
│   └── heap_list.cpp                ← MinHeapList    (std::list<T>)
│
├── 📈 Benchmarks
│   ├── bench_mintas.cpp             ← N = 1e6
│   ├── bench_mintas_scale.cpp       ← 100 → 1e8
│   ├── bench_heap_vector_scale.cpp
│   ├── bench_heap_raw_scale.cpp
│   ├── bench_heap_map_scale.cpp
│   └── bench_heap_list_scale.cpp
│
└── 🎯 Problematique_Exemple/
    ├── bench_students_vector.cpp    ← cas métier
    ├── bench_students_raw.cpp
    └── bench_students_map.cpp
```

---

## ⚙️ Les quatre implémentations

<table>
<tr>
<td width="50%">

### 1️⃣ `MinHeapVector` — *référence moderne*
> Stockage : `std::vector<T>`

```cpp
template <typename T>
class MinHeapVector {
    std::vector<T> arr;
    void heapify(int i); // descente
public:
    void insert(const T& v);
    const T& top() const;
    T pop();
    bool deleteKey(const T& k);
};
```

✅ **Contigu** · ✅ **Cache-friendly** · ✅ **Amorti O(1) sur push_back**

</td>
<td width="50%">

### 2️⃣ `MinHeapRaw` — *tableau brut*
> Stockage : `T* buf` + `new[]` / `delete[]`

```cpp
template <typename T>
class MinHeapRaw {
    T* buf; int sz, cap;
    void grow();      // ×2
    void heapify(int i);
public:
    ~MinHeapRaw();    // delete[] buf
    // ...
};
```

✅ **Contrôle mémoire** · ⚠️ **Règle des 3/5** · 🎓 **Pédagogique**

</td>
</tr>
<tr>
<td width="50%">

### 3️⃣ `MinHeapMap` — *accès O(log N)*
> Stockage : `std::map<int, T>` (clé = index)

```cpp
std::map<int, T> tree;
T& ref(int i) { return tree.at(i); }
```

🔴 **Insert/pop = O(log² N)**  
🔴 **Top = O(log N) au lieu de O(1)**  
🎓 **Montre le coût d'un accès non contigu**

</td>
<td width="50%">

### 4️⃣ `MinHeapList` — *anti-pattern*
> Stockage : `std::list<T>`

```cpp
typename std::list<T>::iterator at(int i) {
    return std::next(lst.begin(), i); // O(i) !!
}
```

🔴 **Insert/pop = O(N)**  
🔴 **Inadapté aux tas**  
🎓 **Preuve empirique du mur**

</td>
</tr>
</table>

---

## 📐 Complexités théoriques

<table>
<thead>
<tr>
<th align="left">Implémentation</th>
<th><code>insert</code></th>
<th><code>top</code></th>
<th><code>pop</code></th>
<th><code>deleteKey</code></th>
<th><code>Mémoire</code></th>
</tr>
</thead>
<tbody>
<tr>
<td align="left">🟢 <b>MinHeapVector</b></td>
<td><code>O(log N)</code> amorti</td>
<td><code>O(1)</code></td>
<td><code>O(log N)</code></td>
<td><code>O(N)</code></td>
<td><code>~N·sizeof(T)</code> + slack</td>
</tr>
<tr>
<td align="left">🟢 <b>MinHeapRaw</b></td>
<td><code>O(log N)</code> amorti</td>
<td><code>O(1)</code></td>
<td><code>O(log N)</code></td>
<td><code>O(N)</code></td>
<td><code>~N·sizeof(T)</code> (cap ×2)</td>
</tr>
<tr>
<td align="left">🟠 <b>MinHeapMap</b></td>
<td><code>O(log² N)</code></td>
<td><code>O(log N)</code></td>
<td><code>O(log² N)</code></td>
<td><code>O(N)</code></td>
<td><code>N·(sizeof(T)+overhead)</code></td>
</tr>
<tr>
<td align="left">🔴 <b>MinHeapList</b></td>
<td><code>O(N)</code></td>
<td><code>O(1)</code></td>
<td><code>O(N)</code></td>
<td><code>O(N)</code></td>
<td><code>N·(sizeof(T)+16)</code></td>
</tr>
</tbody>
</table>

> [!NOTE]
> `deleteKey` est **`O(N)` dans tous les cas** à cause de la recherche linéaire de la clé. Pour la rendre `O(log N)`, il faudrait un index `unordered_map<T, index>` maintenu en parallèle *(hors périmètre)*.

---

## 🎯 Problématique métier

> **Énoncé** : gérer `N = 1 000 000` d'étudiants boursiers et toujours pouvoir extraire **le moins boursier** rapidement.

```cpp
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
```

Le tas est un **min-heap** sur `scholarship` (départage par `id`). Le tas sur `Student` étant plus lourd (`std::string`), il révèle l'impact **mémoire + copies** en plus de la complexité.

---

## 📊 Résultats des benchmarks

> [!IMPORTANT]
> **Machine** : HP EliteBook 850 G8 · **Compilateur** : `g++ -std=c++17 -O2`
> **Graine** : `SEED = 42` *(reproductibilité)* · **Unité** : `ns/op` *(plus petit = meilleur)*

### 🎨 Vue d'ensemble

| Opération | 🟢 Vector | 🟢 Raw | 🟠 Map | 🔴 List |
|:---|:---:|:---:|:---:|:---:|
| `insert` | `O(log N)` | `O(log N)` | `O(log² N)` | `O(N)` |
| `top` | `O(1)` | `O(1)` | `O(log N)` | `O(1)` |
| `pop` | `O(log N)` | `O(log N)` | `O(log² N)` | `O(N)` |
| `deleteKey` | `O(N)` | `O(N)` | `O(N)` | `O(N)` |

---

### 🟢 MinHeapVector / MinHeapRaw — scalabilité

> `TOP_ITERS = 50 000 000` · `DEL_CAP = 10 000` · Budget = 30 s/phase

<table>
<thead>
<tr>
<th colspan="5" align="center">🟢 MinHeapVector (ns/op)</th>
</tr>
<tr>
<th>N</th><th>insert</th><th>top</th><th>pop</th><th>deleteKey</th>
</tr>
</thead>
<tbody>
<tr><td>100</td><td>24.90</td><td>0.66</td><td>93.08</td><td>68.75</td></tr>
<tr><td>1 000</td><td>37.45</td><td>0.96</td><td>87.72</td><td>181.02</td></tr>
<tr><td>10 000</td><td>25.76</td><td>1.11</td><td>110.56</td><td>1 338.80</td></tr>
<tr><td>100 000</td><td>18.42</td><td>0.73</td><td>149.28</td><td>1 233.65</td></tr>
<tr><td>1 000 000</td><td>24.89</td><td>0.61</td><td>189.90</td><td>921.76</td></tr>
<tr><td>10 000 000</td><td>20.42</td><td>0.46</td><td>465.25</td><td>1 137.78</td></tr>
<tr><td>100 000 000</td><td>24.24</td><td>0.62</td><td>700.13</td><td>1 176.53</td></tr>
</tbody>
</table>

<table>
<thead>
<tr>
<th colspan="5" align="center">🟢 MinHeapRaw (ns/op)</th>
</tr>
<tr>
<th>N</th><th>insert</th><th>top</th><th>pop</th><th>deleteKey</th>
</tr>
</thead>
<tbody>
<tr><td>100</td><td>20.99</td><td>0.48</td><td>78.50</td><td>51.36</td></tr>
<tr><td>1 000</td><td>25.78</td><td>0.52</td><td>84.99</td><td>189.39</td></tr>
<tr><td>10 000</td><td>26.42</td><td>0.51</td><td>122.16</td><td>1 144.05</td></tr>
<tr><td>100 000</td><td>22.86</td><td>0.56</td><td>157.32</td><td>1 151.20</td></tr>
<tr><td>1 000 000</td><td>20.97</td><td>0.48</td><td>155.55</td><td>894.16</td></tr>
<tr><td>10 000 000</td><td>17.18</td><td>0.64</td><td>307.03</td><td>934.59</td></tr>
<tr><td>100 000 000</td><td>20.04</td><td>1.22</td><td>627.24</td><td>1 082.72</td></tr>
</tbody>
</table>

**Visualisation** *(échelle logarithmique, tendances)*

```text
insert  ▁▁▁▁▁▁▁   ~20 ns constants   → O(log N) amorti
top     ▁▁▁▁▁▁▁   ~0.5 ns (cache L1) → O(1)
pop     ▂▂▃▃▄▄▅   80 → 700 ns         → O(log N)
deleteKey ▁▂████   saut à N=10⁴ → O(N)
```

> [!WARNING]
> À `N = 1e8`, le `pop` a été **abandonné** (`pop ABORT` après 30 s) : ~42–47 M de pops sur 50 M visés.

---

### 🟠 MinHeapMap — scalabilité

> `TOP_ITERS = 5 000 000` · `DEL_CAP = 5 000` · Budget = 15 s/phase

<table>
<thead>
<tr>
<th colspan="5" align="center">🟠 MinHeapMap (ns/op)</th>
</tr>
<tr>
<th>N</th><th>insert</th><th>top</th><th>pop</th><th>deleteKey</th>
</tr>
</thead>
<tbody>
<tr><td>100</td><td>384.46</td><td>5.44</td><td>1 128.98</td><td>800.68</td></tr>
<tr><td>1 000</td><td>474.20</td><td>10.12</td><td>2 276.72</td><td>1 939.95</td></tr>
<tr><td>10 000</td><td>666.85</td><td>12.47</td><td>3 913.64</td><td>9 647.39</td></tr>
<tr><td>100 000</td><td>1 100.79</td><td>27.47</td><td>10 016.81</td><td>9 267.05</td></tr>
<tr><td>1 000 000</td><td>1 680.47</td><td>45.06</td><td>17 772.47</td><td>8 508.00</td></tr>
</tbody>
</table>

> [!TIP]
> `top` est **~90× plus lent** que sur vector (45 vs 0.5 ns à `N=10⁶`).  
> `pop` atteint **17.8 µs/op** (contre 156–190 ns en vector, soit **×100 à ×110**).

---

### 🔴 MinHeapList — scalabilité

> `TOP_ITERS = 5 000 000` · `DEL_CAP = 2 000` · Budget = 5 s/phase · Tailles plafonnées à `1e5`

<table>
<thead>
<tr>
<th colspan="6" align="center">🔴 MinHeapList (ns/op)</th>
</tr>
<tr>
<th>N</th><th>insert</th><th>top</th><th>pop</th><th>deleteKey</th><th>Statut</th>
</tr>
</thead>
<tbody>
<tr><td>100</td><td>181.35</td><td>0.49</td><td>516.62</td><td>259.62</td><td>✅ OK</td></tr>
<tr><td>300</td><td>630.95</td><td>0.60</td><td>1 689.33</td><td>835.13</td><td>✅ OK</td></tr>
<tr><td>1 000</td><td>1 964.44</td><td>0.50</td><td>8 331.83</td><td>3 799.61</td><td>✅ OK</td></tr>
<tr><td>3 000</td><td>9 058.12</td><td>0.95</td><td>27 775.36</td><td>7 773.94</td><td>✅ OK</td></tr>
<tr><td>10 000</td><td>26 660.87</td><td>0.48</td><td>115 251.55</td><td>8 801.36</td><td>✅ OK</td></tr>
<tr><td>30 000</td><td>93 920.02</td><td>0.45</td><td>391 240.59</td><td>8 727.00</td><td>⚠️ <b>pop ABORT</b></td></tr>
<tr><td>100 000</td><td>123 505.75</td><td>2.21</td><td>643 574.40</td><td>9 346.90</td><td>🚫 <b>insert+pop ABORT</b></td></tr>
</tbody>
</table>

> [!CAUTION]
> À `N = 10⁵`, un seul `insert` prend **124 µs** — un `vector` fait la même chose en **~20 ns**, soit **×6 000 plus rapide**. Les `ABORT` prouvent que la structure **ne passe pas à l'échelle**.

---

### 👨‍🎓 Étudiants boursiers (`N = 1 000 000`)

<table>
<thead>
<tr>
<th align="left">Implémentation</th>
<th><code>insert</code> total</th>
<th><code>top</code> ×iters</th>
<th><code>pop</code> 500k</th>
<th><code>deleteKey</code></th>
</tr>
</thead>
<tbody>
<tr>
<td align="left">🟢 <b>Vector</b></td>
<td>66.57 ms <i>(67 ns/op)</i></td>
<td>25.13 ms / 50M <i>(0.50 ns)</i></td>
<td>364.95 ms <i>(730 ns/op)</i></td>
<td>11 406 ms <i>(1.14 ms/op, 4 984)</i></td>
</tr>
<tr>
<td align="left">🟢 <b>Raw</b></td>
<td>71.89 ms <i>(72 ns/op)</i></td>
<td>24.46 ms / 50M <i>(0.49 ns)</i></td>
<td>591.67 ms <i>(1 183 ns/op)</i></td>
<td>13 135 ms <i>(1.31 ms/op, 4 984)</i></td>
</tr>
<tr>
<td align="left">🟠 <b>Map</b></td>
<td>1 462.34 ms <i>(1.46 µs/op)</i></td>
<td>234.29 ms / 5M <i>(46.86 ns)</i></td>
<td>11 722.99 ms <i>(23.4 µs/op)</i></td>
<td>38 538 ms <i>(7.71 ms/op, 2 508)</i></td>
</tr>
</tbody>
</table>

**Interprétation métier** :

| Constat | Détail |
|:---|:---|
| ✅ Vector ≈ Raw | Écart < 10 % sur `insert`/`top` ; le choix `new/delete` n'apporte **rien** en perf |
| 🔴 Map ×22 plus lent | À l'insertion (1.46 µs vs 67 ns) |
| 🔴 Map ×32 plus lent | Au `pop` (23.4 µs vs 730 ns) |
| 🔴 Map ×94 plus lent | Au `top` (46.86 ns vs 0.50 ns) |
| 🔴 Map ×7 plus lent | Sur `deleteKey` (7.71 ms vs 1.14 ms) |
| ✅ Résultat identique | Somme bourses extraites = **1 249 507 804 €** dans les 3 cas → validation croisée |

---

### 🏆 Comparaison finale

```text
N = 1 000 000 — ns/op (plus petit = meilleur)
─────────────────────────────────────────────────────────
                🟢 Vector   🟢 Raw      🟠 Map       🔴 List
insert            24.89       20.97     1 680.47    123 506*   (aborted)
top                0.61        0.48        45.06        0.62
pop              189.90      155.55    17 772.47    618 292*   (aborted)
deleteKey        921.76      894.16     8 508.00      8 727
─────────────────────────────────────────────────────────
* valeurs non comparables : budget temps dépassé
```

<table>
<thead>
<tr><th align="left">Métrique</th><th align="left">Gagnant</th><th align="left">Facteur vs Vector</th></tr>
</thead>
<tbody>
<tr><td><code>insert</code></td><td>🟢 Vector ≈ Raw</td><td>—</td></tr>
<tr><td><code>top</code></td><td>🟢 Vector ≈ Raw ≈ 🔴 List</td><td>— <i>(toutes O(1))</i></td></tr>
<tr><td><code>pop</code></td><td>🟢 Vector ≈ Raw</td><td>—</td></tr>
<tr><td><code>deleteKey</code></td><td>🟢 Vector ≈ Raw</td><td>—</td></tr>
<tr><td><code>std::list</code> à N=10⁵</td><td>—</td><td>🔴 <b>×6 000 plus lent</b></td></tr>
<tr><td><code>std::map</code> à N=10⁶</td><td>—</td><td>🔴 <b>×22 à ×110 plus lent</b></td></tr>
</tbody>
</table>

---

## 🧠 Interprétation des résultats

<details open>
<summary><b>1️⃣ <code>top()</code> est vraiment <code>O(1)</code> — et c'est visible</b></summary>

Sur `vector`, `top` reste **entre 0.4 et 1.1 ns/op** sur 6 ordres de grandeur. Le cache L1 gagne : `arr[0]` est toujours chaud.

En `std::map`, `tree.at(0)` traverse un arbre rouge-noir → **~45 ns/op** à `N=10⁶` (**×75 à ×100**).
</details>

<details open>
<summary><b>2️⃣ <code>insert</code> en <code>O(log N)</code> amorti : quasi-plat</b></summary>

Entre `N=100` (≈7 niveaux) et `N=10⁸` (≈27 niveaux), on attend un facteur ~4. Mesuré : **16 → 24 ns** (~1.5).

Le coût est **dominé par le cache** et le `push_back`, pas par le nombre de comparaisons (des `swap` de 4 octets quasi-gratuits).
</details>

<details open>
<summary><b>3️⃣ <code>deleteKey</code> est le vrai goulot</b></summary>

Le passage de `~50 ns` (petits `N`) à `~1 µs` (grands `N`) illustre `O(N)` : à `N=10⁴`, 10 000 comparaisons ; à `N=10⁶`, 1 000 000.

Le **plafond `DEL_CAP`** évite des runs de plusieurs minutes. **Leçon** : si `deleteKey` est fréquent, il faut un index secondaire.
</details>

<details open>
<summary><b>4️⃣ <code>std::list</code> est disqualifié pour les tas</b></summary>

Avec `std::next(begin(), i)`, un accès au niveau `k` coûte `O(2^k)` → insertion en `O(N)` en moyenne.

Les **abandons** (`ABORT`) à `N=30 000` et `100 000` sont la preuve empirique du mur.
</details>

<details open>
<summary><b>5️⃣ <code>std::map</code> : <code>O(log² N)</code> coûte cher en pratique</b></summary>

Le facteur **×100** sur `pop` à `N=10⁶` vient de `log²(10⁶) ≈ 400` opérations de pointeurs par `pop`, contre `log₂(10⁶) ≈ 20` en vector.

La théorie se vérifie **brutalement**.
</details>

<details open>
<summary><b>6️⃣ Vector vs Raw : match nul</b></summary>

Performances **statistiquement proches** (écarts souvent < 20 %, sensibles au bruit et aux copies de `T`).

`std::vector` apporte la sécurité (copie, move, exceptions) **sans surcoût** significatif à `-O2`. `Raw` n'est utile qu'en exercice.
</details>

<details open>
<summary><b>7️⃣ Min Heap ↔ Max Heap : symétrie parfaite</b></summary>

Les chiffres sont **quasi identiques** à ceux du max heap (mêmes ordres, mêmes courbes).

Attendu : la seule différence est le sens de la comparaison (`<` au lieu de `>`), qui n'affecte **ni le nombre d'opérations ni la localité mémoire**.
</details>

---

## 🚀 Compilation & exécution

### 📋 Prérequis

- `g++` ≥ 9 *(C++17)* ou `clang++` ≥ 10
- ~1 Go de RAM pour les runs à `N = 1e8`

### 🎓 Implémentations (démonstration)

```bash
g++ -std=c++17 -O2 -Wall -Wextra -o minTas       minTas.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_vector  heap_vector.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_raw     heap_raw.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_map     heap_map.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o heap_list    heap_list.cpp

./minTas
./heap_vector
./heap_raw
./heap_map
./heap_list
```

### 📈 Benchmarks

```bash
# Bench simple (N = 1e6)
g++ -std=c++17 -O2 -Wall -Wextra -o bench_mintas bench_mintas.cpp
./bench_mintas

# Bench scalabilité (100 → 1e8)
g++ -std=c++17 -O2 -Wall -Wextra -o bench_mintas_scale      bench_mintas_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_vector_scale bench_heap_vector_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_raw_scale    bench_heap_raw_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_map_scale    bench_heap_map_scale.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_list_scale   bench_heap_list_scale.cpp

./bench_mintas_scale
./bench_heap_vector_scale
./bench_heap_raw_scale
./bench_heap_map_scale
./bench_heap_list_scale
```

### 🎯 Cas métier (étudiants)

```bash
cd Problematique_Exemple
g++ -std=c++17 -O2 -Wall -Wextra -o bench_students_vector bench_students_vector.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_students_raw    bench_students_raw.cpp
g++ -std=c++17 -O2 -Wall -Wextra -o bench_students_map    bench_students_map.cpp

./bench_students_vector
./bench_students_raw
./bench_students_map
```

### 🔧 Configuration

Tout est dans les **constantes en tête de fichier** *(pas d'arguments CLI, sauf `bench_mintas.cpp` et `bench_mintas_scale.cpp`)* :

```cpp
static const std::vector<std::size_t> SIZES = {
    100, 1'000, 10'000, 100'000,
    1'000'000, 10'000'000, 100'000'000
};
static const double      BUDGET_MS = 30000.0;   // budget max par phase
static const std::size_t DEL_CAP   = 10'000;    // cap deleteKey
static const std::size_t TOP_ITERS = 50'000'000;
static const unsigned    SEED      = 42;
```

---

## 🎓 Points pédagogiques

<table>
<thead>
<tr><th width="5%">#</th><th align="left">Leçon</th></tr>
</thead>
<tbody>
<tr>
<td align="center">1️⃣</td>
<td><b>La complexité asymptotique est un guide, pas une garantie.</b><br>
<code>O(log N)</code> sur <code>std::map</code> devient <code>O(log² N)</code> à cause de l'accès non contigu — la théorie le prédit, les chiffres le confirment (×100 sur <code>pop</code>).</td>
</tr>
<tr>
<td align="center">2️⃣</td>
<td><b>Le cache est roi.</b><br>
<code>vector</code> et <code>raw</code> restent à ~20 ns/insert sur 6 ordres de grandeur parce que la mémoire est contiguë. <code>map</code> paie chaque saut de pointeur.</td>
</tr>
<tr>
<td align="center">3️⃣</td>
<td><b><code>deleteKey</code> en <code>O(N)</code> est un piège classique.</b><br>
À <code>N=10⁶</code>, une seule suppression coûte <b>~1 ms</b> — invisible sur un petit N, rédhibitoire en production.</td>
</tr>
<tr>
<td align="center">4️⃣</td>
<td><b><code>std::list</code> n'est pas un "vector avec insertion rapide".</b><br>
Sans accès aléatoire, il est <b>structurellement inadapté</b> aux tas.</td>
</tr>
<tr>
<td align="center">5️⃣</td>
<td><b><code>std::vector</code> bat <code>new/delete</code> en sécurité, à égalité en perf.</b><br>
Aucune raison technique de préférer le tableau brut, sauf exercice.</td>
</tr>
<tr>
<td align="center">6️⃣</td>
<td><b>Les benchmarks doivent avoir des garde-fous.</b><br>
<code>BUDGET_MS</code> et <code>DEL_CAP</code> évitent qu'un run pédagogique prenne 3 heures — et les <code>ABORT</code> documentent honnêtement les limites.</td>
</tr>
<tr>
<td align="center">7️⃣</td>
<td><b>Min Heap ↔ Max Heap : même coût, code miroir.</b><br>
Remplacer <code>&lt;</code> par <code>&gt;</code> ne change <b>ni la complexité, ni les performances</b>. Un tas min est un tas max « retourné » — le reste est identique.</td>
</tr>
</tbody>
</table>

---

## 🛠️ Limites & pistes d'amélioration

<table>
<thead>
<tr><th align="left">Limite</th><th align="left">Piste</th></tr>
</thead>
<tbody>
<tr><td><code>deleteKey</code> en <code>O(N)</code></td><td>Index <code>std::unordered_map&lt;T, size_t&gt;</code> maintenu → <code>O(log N)</code></td></tr>
<tr><td><code>Student</code> recopié à chaque <code>swap</code></td><td><code>std::vector&lt;std::unique_ptr&lt;Student&gt;&gt;</code> ou heap sur <code>id</code> + table annexe</td></tr>
<tr><td>Pas de <code>buildHeap</code> en <code>O(N)</code> mesuré</td><td>Ajouter <code>MinHeap::MinHeap(vector&lt;T&gt;)</code> + bench dédié (déjà présent dans <code>minTas.cpp</code>, non benchmarké)</td></tr>
<tr><td>Pas de <code>decreaseKey</code> / <code>increaseKey</code></td><td>Utile pour <b>Dijkstra</b> / <b>Prim</b> — à implémenter</td></tr>
<tr><td>Pas de test unitaire</td><td>Ajouter <code>Catch2</code> ou <code>GoogleTest</code> avec cas aléatoires</td></tr>
<tr><td>Bench monothread</td><td><code>std::execution::par</code> sur la génération, ou bench concurrent</td></tr>
<tr><td>Pas de profiling cache</td><td><code>perf stat -e cache-misses</code> ou <code>valgrind --tool=cachegrind</code></td></tr>
<tr><td>Pas de mesure mémoire</td><td><code>valgrind --tool=massif</code> ou <code>/usr/bin/time -v</code></td></tr>
<tr><td><code>pop</code> sur <code>N = 1e8</code> non terminé dans le budget</td><td>Augmenter <code>BUDGET_MS</code>, paralléliser, ou réduire <code>half</code></td></tr>
</tbody>
</table>

---

## 📎 Annexe — Résultats bruts (extraits terminaux)

<details>
<summary><b>🟢 <code>./bench_heap_vector_scale</code> — sortie complète</b></summary>

```text
================================================================
  Benchmark MinHeapVector (std::vector) — self-contained
  Budget/phase = 30000 ms | DEL_CAP = 10000 | TOP_ITERS = 50 000 000
  /!\ insert/pop = O(log N), top = O(1), deleteKey = O(N)
  /!\ N = 1e8 : ~1 Go de RAM (data + buffer)
================================================================

>>> Exécution N = 100 ... OK
>>> Exécution N = 1 000 ... OK
>>> Exécution N = 10 000 ... OK
>>> Exécution N = 100 000 ... OK
>>> Exécution N = 1 000 000 ... OK
>>> Exécution N = 10 000 000 ... OK
>>> Exécution N = 100 000 000 ... OK [pop ABORT]

RÉSULTATS DÉTAILLÉS
-------------------

N = 100
  Insert   :         0.00 ms |          24.90 ns/op (100 / 100)
  Top      :        32.91 ms |           0.66 ns/op (50 000 000 iters)
  Pop      :         0.00 ms |          93.08 ns/op (50)
  DeleteKey:         0.01 ms |          68.75 ns/op (100)

N = 1 000 000
  Insert   :        24.89 ms |          24.89 ns/op (1 000 000 / 1 000 000)
  Top      :        30.68 ms |           0.61 ns/op (50 000 000 iters)
  Pop      :        94.95 ms |         189.90 ns/op (500 000)
  DeleteKey:         9.22 ms |         921.76 ns/op (10 000)

N = 100 000 000
  Insert   :      2423.78 ms |          24.24 ns/op (100 000 000 / 100 000 000)
  Top      :        31.10 ms |           0.62 ns/op (50 000 000 iters)
  Pop      :     30000.01 ms |         700.13 ns/op (42 849 281, ABORT)
  DeleteKey:        11.77 ms |        1176.53 ns/op (10 000)
```

</details>

<details>
<summary><b>🟢 <code>./bench_heap_raw_scale</code> — sortie complète</b></summary>

```text
================================================================
  Benchmark MinHeapRaw (tableau brut new/delete) — self-contained
  Budget/phase = 30000 ms | DEL_CAP = 10000 | TOP_ITERS = 50 000 000
================================================================

N = 1 000 000
  Insert   :        20.97 ms |          20.97 ns/op (1 000 000 / 1 000 000)
  Top      :        24.18 ms |           0.48 ns/op (50 000 000 iters)
  Pop      :        77.77 ms |         155.55 ns/op (500 000)
  DeleteKey:         8.94 ms |         894.16 ns/op (10 000)

N = 100 000 000
  Insert   :      2003.56 ms |          20.04 ns/op (100 000 000 / 100 000 000)
  Top      :        61.13 ms |           1.22 ns/op (50 000 000 iters)
  Pop      :     30000.03 ms |         627.24 ns/op (47 828 673, ABORT)
  DeleteKey:        10.83 ms |        1082.72 ns/op (10 000)
```

</details>

<details>
<summary><b>🟠 <code>./bench_heap_map_scale</code> — sortie complète</b></summary>

```text
================================================================
  Benchmark MinHeapMap (std::map) — self-contained
  Budget/phase = 15000 ms | DEL_CAP = 5000 | TOP_ITERS = 5 000 000
  /!\ insert/pop = O(log^2 N), top = O(log N), deleteKey = O(N)
================================================================

>>> Exécution N = 100 ... OK
>>> Exécution N = 1 000 ... OK
>>> Exécution N = 10 000 ... OK
>>> Exécution N = 100 000 ... OK
>>> Exécution N = 1 000 000 ... OK

RATIOS ns/op entre N et N/10
-----------------------------
Attendu MinHeapMap : top ~log(N) (≈1.2-1.5 entre N et N/10)
                     insert/pop ~log^2 => ratio ≈ 1.5-2
                     deleteKey ~O(N) => ratio ≈ 10

N                         insert           top           pop     deleteKey
--------------------------------------------------------------------------
1 000 / 100                 1.23          1.86          2.02          2.42
10 000 / 1 000              1.41          1.23          1.72          4.97
100 000 / 10 000            1.65          2.20          2.56          0.96
1 000 000 / 100 000          1.53          1.64          1.77          0.92
```

</details>

<details>
<summary><b>🔴 <code>./bench_heap_list_scale</code> — sortie complète</b></summary>

```text
================================================================
  Benchmark MinHeapList (std::list) — self-contained
  Budget/phase = 5000 ms | DEL_CAP = 2000 | TOP_ITERS = 5 000 000
  /!\ insert/pop/deleteKey = O(N) par op, top = O(1)
================================================================

>>> Exécution N = 100 ... OK
>>> Exécution N = 300 ... OK
>>> Exécution N = 1 000 ... OK
>>> Exécution N = 3 000 ... OK
>>> Exécution N = 10 000 ... OK
>>> Exécution N = 30 000 ... OK [pop ABORT]
>>> Exécution N = 100 000 ... OK [insert ABORT] [pop ABORT]

RATIOS ns/op entre N et N/10
-----------------------------
Attendu MinHeapList : top ~1 | insert/pop/delete ~10 (~O(N) par op)

N                         insert           top           pop     deleteKey
--------------------------------------------------------------------------
300 / 100                   3.48          1.24          3.27          3.22
1 000 / 300                 3.11          0.83          4.93          4.55
3 000 / 1 000               4.61          1.90          3.33          2.05
10 000 / 3 000              2.94          0.51          4.15          1.13
30 000 / 10 000             3.52          0.93          3.39          0.99
100 000 / 30 000            1.32          4.95          1.64          1.07
```

</details>

<details>
<summary><b>🎓 <code>./bench_mintas</code> — démo N = 1e6</b></summary>

```text
=== Benchmark MinHeap (N = 1000000) ===
Insert total : 23.46 ms (23.46 ns/op)
Top x10000000 : 10.29 ms (1.03 ns/op)
Pop 500000 éléments : 114.01 ms (228.02 ns/op)
DeleteKey x1000 : 0.16 ms
Sink = 1252859639994 (empêche l'optimisation)
```

</details>

<details>
<summary><b>🎓 <code>./bench_mintas_scale</code> — sortie complète</b></summary>

```text
============================================================
  Benchmark MinHeap — balayage de N
============================================================

TABLEAU RÉCAPITULATIF (ns/op)
-----------------------------
N                   insert           top           pop     deleteKey
--------------------------------------------------------------------
100                  16.51          0.48         59.72         50.63
1 000                27.65          0.61         72.27        390.81
10 000               20.85          0.49        113.18       1114.12
100 000              19.91          0.50        135.70       1143.21
1 000 000            23.30          0.41        190.06       1304.66
10 000 000           18.92          0.38        397.30        998.26
100 000 000          23.05          0.54        548.48        909.33

ANALYSE DE COMPLEXITÉ (ratio ns/op entre N et N/10)
----------------------------------------------------
Attendu : top ~ 1 | insert/pop ~ log(N) (≈ 1.1-1.3) | deleteKey ~ 10 (linéaire)

N                   insert           top           pop     deleteKey
--------------------------------------------------------------------
1 000 / 100           1.67          1.28          1.21          7.72
10 000 / 1 000          0.75          0.80          1.57          2.85
100 000 / 10 000          0.95          1.03          1.20          1.03
1 000 000 / 100 000          1.17          0.80          1.40          1.14
10 000 000 / 1 000 000          0.81          0.94          2.09          0.77
100 000 000 / 10 000 000          1.22          1.41          1.38          0.91
```

</details>

<details>
<summary><b>🎯 <code>./bench_students_vector</code> — cas métier</b></summary>

```text
===============================================================
  [VECTOR]   Étudiants boursiers — N = 1 000 000
===============================================================

Insert   :        66.57 ms | 66.57 ns/op
Top x50 000 000 :        25.13 ms | 0.50 ns/op
Pop 500 000 :       364.95 ms | 729.90 ns/op   (somme = 1249507804 €)
DeleteKey x10 000 :        11406 ms | 1140632 ns/op   (4984 supprimés)

----------------------------- RÉCAP -----------------------------
Opération          total (ms)         ns/op         complexité
----------------------------------------------------------------
insert                      67            67            O(log N)
top                         25             1                O(1)
pop (extract)              365           730            O(log N)
deleteKey                11406       1140632                O(N)
----------------------------------------------------------------
moins boursier restant : 5000 € (id 789309)
```

</details>

<details>
<summary><b>🎯 <code>./bench_students_raw</code> — cas métier</b></summary>

```text
===============================================================
  [RAW]   Étudiants boursiers — N = 1 000 000
  insert / top (le + boursier) / pop (= extract-min) / deleteKey
===============================================================

Insert   :        71.89 ms | 71.89 ns/op
Top x50 000 000 :        24.46 ms | 0.49 ns/op
Pop 500 000 :       591.67 ms | 1183.34 ns/op   (somme bourses extraites = 1249507804 €)
DeleteKey x10 000 :        13135 ms | 1313469 ns/op   (4984 supprimés)

moins boursier restant : Student{id=789309, name="etu6045", scholarship=5000€}
```

</details>

<details>
<summary><b>🎯 <code>./bench_students_map</code> — cas métier</b></summary>

```text
===============================================================
  [MAP]   Étudiants boursiers — N = 1 000 000
  /!\ O(log^2 N) par insert/pop — plus lent
===============================================================

Insert   :      1462.34 ms | 1462.34 ns/op
Top x5 000 000 :       234.29 ms | 46.86 ns/op
Pop 500 000 :     11722.99 ms | 23445.97 ns/op   (somme = 1249507804 €)
DeleteKey x5 000 :        38538 ms | 7707627 ns/op   (2508 supprimés)

moins boursier restant : 5000 € (id 789309)
```

</details>

---

<div align="center">

### 🏁 TL;DR

```
┌──────────────────────────────────────────────────────────────┐
│  ✅  Utilisez std::vector pour un tas.                        │
│  ❌  N'utilisez PAS std::map ni std::list.                    │
│  ⚠️  deleteKey est O(N) — indexez si c'est fréquent.          │
│  🎓  new/delete et std::vector : mêmes perfs, plus de risques.│
└──────────────────────────────────────────────────────────────┘
```

**Min Heap ≡ Max Heap** — même complexité, même perf, seul `<` change en `>`.

*Projet pédagogique · ILISI2 · Virtualisation · C++ · Seed = 42 · Reproducible*

</div>

---

## 📜 Licence

Projet pédagogique — **ILISI2, Virtualisation, C++** — libre d'utilisation à des fins d'apprentissage.

<p align="center">
  <sub>📦 Généré à partir du dépôt <code>TAS/MinTas/</code> — benchmarks reproductibles avec <code>SEED = 42</code></sub>
</p>
