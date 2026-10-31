// bench_mintas_scale.cpp
// Benchmark augmentatif : insert / top / pop / deleteKey sur MinHeap (vector)
// Balaye N = 100, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8 (configurable).
//
// Compilation :
//   g++ -std=c++17 -O2 -Wall -Wextra -o bench_mintas_scale bench_mintas_scale.cpp
// Exécution :
//   ./bench_mintas_scale              # tailles par défaut
//   ./bench_mintas_scale 1000 100000  # tailles custom
//
// Conseils de lecture :
//   - ns/op permet de comparer les tailles entre elles (complexité).
//   - insert ≈ O(log N) → ns/op doit croître lentement avec N.
//   - top     ≈ O(1)     → ns/op quasi constant.
//   - pop     ≈ O(log N) → ns/op croît comme insert.
//   - deleteKey ≈ O(N)   → ns/op croît LINÉAIREMENT avec N (recherche linéaire).

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <string>
#include <cstdint>

// ---------- MinHeap minimal ----------
template <typename T>
class MinHeap {
    std::vector<T> arr;
    void heapify(int i) {
        int n = (int)arr.size();
        while (true) {
            int l = 2*i+1, r = 2*i+2, smallest = i;
            if (l < n && arr[l] < arr[smallest]) smallest = l;
            if (r < n && arr[r] < arr[smallest]) smallest = r;
            if (smallest == i) break;
            std::swap(arr[i], arr[smallest]);
            i = smallest;
        }
    }
public:
    void reserve(std::size_t n) { arr.reserve(n); }
    void insert(const T& v) {
        arr.push_back(v);
        int i = (int)arr.size() - 1;
        while (i != 0 && arr[i] < arr[(i-1)/2]) {
            std::swap(arr[i], arr[(i-1)/2]);
            i = (i-1)/2;
        }
    }
    T pop() {
        T root = arr[0];
        arr[0] = arr.back(); arr.pop_back();
        if (!arr.empty()) heapify(0);
        return root;
    }
    const T& top() const { return arr[0]; }
    int size() const { return (int)arr.size(); }
    bool empty() const { return arr.empty(); }
    bool deleteKey(const T& k) {
        int n = (int)arr.size(), idx = -1;
        for (int i = 0; i < n; ++i) if (arr[i] == k) { idx = i; break; }
        if (idx == -1) return false;
        if (idx == n-1) { arr.pop_back(); return true; }
        arr[idx] = arr.back(); arr.pop_back();
        int i = idx;
        while (i != 0 && arr[i] < arr[(i-1)/2]) {
            std::swap(arr[i], arr[(i-1)/2]); i = (i-1)/2;
        }
        heapify(i);
        return true;
    }
};

using Clock = std::chrono::high_resolution_clock;
static double ms(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

struct Row {
    std::size_t N;
    double ins_ms, ins_ns;
    double top_ms, top_ns;     // par op
    double pop_ms, pop_ns;     // par op
    double del_ms, del_ns;     // par op (deleteKey sur min(N, 10k) éléments)
    std::size_t top_iters;
    std::size_t pop_count;
    std::size_t del_count;
};

// Formate les grands nombres avec séparateurs (1'000'000 -> "1 000 000")
static std::string fmt_u(std::uint64_t v) {
    std::string s = std::to_string(v);
    std::string out;
    int c = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        out.push_back(s[i]);
        if (++c == 3 && i != 0) { out.push_back(' '); c = 0; }
    }
    std::reverse(out.begin(), out.end());
    return out;
}

// Choisit un nombre d'itérations "top" pour rester raisonnable en temps
// (au moins 1e4, au plus 1e7, et ~ 3e8 opérations au total).
static std::size_t pick_top_iters(std::size_t N) {
    // top est O(1) ; viser ~ 50M d'appels pour une mesure stable
    const std::size_t TARGET = 50'000'000;
    std::size_t it = TARGET;
    if (it < 10'000) it = 10'000;
    if (it > 20'000'000) it = 20'000'000;
    (void)N;
    return it;
}

static Row run_one(std::size_t N, std::mt19937_64& rng) {
    Row r{};
    r.N = N;

    std::uniform_int_distribution<int> dist(0, 10'000'000);

    // Pré-génération des données (hors mesure)
    std::vector<int> data(N);
    for (auto& x : data) x = dist(rng);

    // ---- 1) INSERT ----
    MinHeap<int> h;
    h.reserve(N);
    auto t0 = Clock::now();
    for (int v : data) h.insert(v);
    auto t1 = Clock::now();
    r.ins_ms = ms(t0, t1);
    r.ins_ns = r.ins_ms * 1e6 / (double)N;

    // ---- 2) TOP ----
    r.top_iters = pick_top_iters(N);
    volatile long long sink = 0;
    t0 = Clock::now();
    for (std::size_t i = 0; i < r.top_iters; ++i) sink += h.top();
    t1 = Clock::now();
    r.top_ms = ms(t0, t1);
    r.top_ns = r.top_ms * 1e6 / (double)r.top_iters;

    // ---- 3) POP (moitié du heap) ----
    r.pop_count = h.size() / 2;
    if (r.pop_count > 0) {
        t0 = Clock::now();
        for (std::size_t i = 0; i < r.pop_count; ++i) sink += h.pop();
        t1 = Clock::now();
        r.pop_ms = ms(t0, t1);
        r.pop_ns = r.pop_ms * 1e6 / (double)r.pop_count;
    } else {
        r.pop_ms = r.pop_ns = 0.0;
    }

    // ---- 4) DELETE KEY ----
    // On construit un heap dédié de taille del_count (capped), on y supprime
    // toutes les clés dans un ordre aléatoire. La recherche étant linéaire,
    // le coût par op est représentatif de O(N).
    r.del_count = std::min<std::size_t>(N, 10'000);
    if (r.del_count > 0) {
        MinHeap<int> hd;
        hd.reserve(r.del_count);
        std::vector<int> keys(r.del_count);
        for (std::size_t i = 0; i < r.del_count; ++i) { int v = (int)i; hd.insert(v); keys[i] = v; }
        std::shuffle(keys.begin(), keys.end(), rng);

        t0 = Clock::now();
        for (int k : keys) hd.deleteKey(k);
        t1 = Clock::now();
        r.del_ms = ms(t0, t1);
        r.del_ns = r.del_ms * 1e6 / (double)r.del_count;
    } else {
        r.del_ms = r.del_ns = 0.0;
    }

    // Empêche l'optimisation
    sink += h.size();
    return r;
}

int main(int argc, char** argv) {
    // Tailles par défaut : 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8
    std::vector<std::size_t> sizes;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) sizes.push_back(std::stoull(argv[i]));
    } else {
        sizes = {100ULL, 1'000ULL, 10'000ULL, 100'000ULL,
                 1'000'000ULL, 10'000'000ULL, 100'000'000ULL};
    }

    std::mt19937_64 rng(42); // graine fixe -> résultats reproductibles

    std::cout << "============================================================\n";
    std::cout << "  Benchmark MinHeap — balayage de N\n";
    std::cout << "  (N = nombre d'éléments ; temps total en ms, par op en ns)\n";
    std::cout << "============================================================\n\n";

    std::vector<Row> rows;
    rows.reserve(sizes.size());

    for (std::size_t N : sizes) {
        std::cout << ">>> Exécution N = " << fmt_u(N) << " ... " << std::flush;
        Row r = run_one(N, rng);
        rows.push_back(r);
        std::cout << "OK\n";
    }

    std::cout << "\n";
    std::cout << "RÉSULTATS DÉTAILLÉS\n";
    std::cout << "-------------------\n\n";

    for (const auto& r : rows) {
        std::cout << "N = " << fmt_u(r.N) << "\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Insert   : " << std::setw(12) << r.ins_ms << " ms total  | "
                  << std::setw(10) << r.ins_ns << " ns/op\n";
        std::cout << "  Top      : " << std::setw(12) << r.top_ms << " ms total  | "
                  << std::setw(10) << r.top_ns << " ns/op  ("
                  << fmt_u(r.top_iters) << " iters)\n";
        std::cout << "  Pop      : " << std::setw(12) << r.pop_ms << " ms total  | "
                  << std::setw(10) << r.pop_ns << " ns/op  ("
                  << fmt_u(r.pop_count) << " pops)\n";
        std::cout << "  DeleteKey: " << std::setw(12) << r.del_ms << " ms total  | "
                  << std::setw(10) << r.del_ns << " ns/op  ("
                  << fmt_u(r.del_count) << " ops)\n\n";
    }

    // ---- Tableau récapitulatif ----
    std::cout << "TABLEAU RÉCAPITULATIF (ns/op)\n";
    std::cout << "-----------------------------\n";
    std::cout << std::left  << std::setw(12) << "N"
              << std::right << std::setw(14) << "insert"
              << std::setw(14) << "top"
              << std::setw(14) << "pop"
              << std::setw(14) << "deleteKey" << "\n";
    std::cout << std::string(68, '-') << "\n";
    for (const auto& r : rows) {
        std::cout << std::left  << std::setw(12) << fmt_u(r.N)
                  << std::right << std::setw(14) << std::setprecision(2) << r.ins_ns
                  << std::setw(14) << r.top_ns
                  << std::setw(14) << r.pop_ns
                  << std::setw(14) << r.del_ns << "\n";
    }

    // ---- Analyse de complexité (ratios entre tailles successives) ----
    std::cout << "\nANALYSE DE COMPLEXITÉ (ratio ns/op entre N et N/10)\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "Attendu : top ~ 1 | insert/pop ~ log(N) (≈ 1.1-1.3) | deleteKey ~ 10 (linéaire)\n\n";
    std::cout << std::left  << std::setw(12) << "N"
              << std::right << std::setw(14) << "insert"
              << std::setw(14) << "top"
              << std::setw(14) << "pop"
              << std::setw(14) << "deleteKey" << "\n";
    std::cout << std::string(68, '-') << "\n";
    for (std::size_t i = 1; i < rows.size(); ++i) {
        const auto& a = rows[i-1];
        const auto& b = rows[i];
        auto ratio = [](double x, double y) -> double {
            return (x > 0.0 && y > 0.0) ? y / x : 0.0;
        };
        std::cout << std::left  << std::setw(12) << (fmt_u(b.N) + " / " + fmt_u(a.N))
                  << std::right << std::setw(14) << std::setprecision(2) << ratio(a.ins_ns, b.ins_ns)
                  << std::setw(14) << ratio(a.top_ns, b.top_ns)
                  << std::setw(14) << ratio(a.pop_ns, b.pop_ns)
                  << std::setw(14) << ratio(a.del_ns, b.del_ns) << "\n";
    }

    std::cout << "\nTerminé.\n";
    return 0;
}

/*
resultat terminal:
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ ./bench_mintas_scale
============================================================
  Benchmark MinHeap — balayage de N
  (N = nombre d'éléments ; temps total en ms, par op en ns)
============================================================

>>> Exécution N = 100 ... OK
>>> Exécution N = 1 000 ... OK
>>> Exécution N = 10 000 ... OK
>>> Exécution N = 100 000 ... OK
>>> Exécution N = 1 000 000 ... OK
>>> Exécution N = 10 000 000 ... OK
>>> Exécution N = 100 000 000 ... OK

RÉSULTATS DÉTAILLÉS
-------------------

N = 100
  Insert   :         0.00 ms total  |      16.51 ns/op
  Top      :         9.54 ms total  |       0.48 ns/op  (20 000 000 iters)
  Pop      :         0.00 ms total  |      59.72 ns/op  (50 pops)
  DeleteKey:         0.01 ms total  |      50.63 ns/op  (100 ops)

N = 1 000
  Insert   :         0.03 ms total  |      27.65 ns/op
  Top      :        12.19 ms total  |       0.61 ns/op  (20 000 000 iters)
  Pop      :         0.04 ms total  |      72.27 ns/op  (500 pops)
  DeleteKey:         0.39 ms total  |     390.81 ns/op  (1 000 ops)

N = 10 000
  Insert   :         0.21 ms total  |      20.85 ns/op
  Top      :         9.77 ms total  |       0.49 ns/op  (20 000 000 iters)
  Pop      :         0.57 ms total  |     113.18 ns/op  (5 000 pops)
  DeleteKey:        11.14 ms total  |    1114.12 ns/op  (10 000 ops)

N = 100 000
  Insert   :         1.99 ms total  |      19.91 ns/op
  Top      :        10.07 ms total  |       0.50 ns/op  (20 000 000 iters)
  Pop      :         6.79 ms total  |     135.70 ns/op  (50 000 pops)
  DeleteKey:        11.43 ms total  |    1143.21 ns/op  (10 000 ops)

N = 1 000 000
  Insert   :        23.30 ms total  |      23.30 ns/op
  Top      :         8.10 ms total  |       0.41 ns/op  (20 000 000 iters)
  Pop      :        95.03 ms total  |     190.06 ns/op  (500 000 pops)
  DeleteKey:        13.05 ms total  |    1304.66 ns/op  (10 000 ops)

N = 10 000 000
  Insert   :       189.23 ms total  |      18.92 ns/op
  Top      :         7.60 ms total  |       0.38 ns/op  (20 000 000 iters)
  Pop      :      1986.50 ms total  |     397.30 ns/op  (5 000 000 pops)
  DeleteKey:         9.98 ms total  |     998.26 ns/op  (10 000 ops)

N = 100 000 000
  Insert   :      2305.24 ms total  |      23.05 ns/op
  Top      :        10.72 ms total  |       0.54 ns/op  (20 000 000 iters)
  Pop      :     27424.15 ms total  |     548.48 ns/op  (50 000 000 pops)
  DeleteKey:         9.09 ms total  |     909.33 ns/op  (10 000 ops)

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

Terminé.
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ 
*/