// bench_heap_vector_scale.cpp
// Benchmark MinHeapVector (std::vector) — self-contained, AUCUN argument CLI.
//
// Compilation :
//   g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_vector_scale bench_heap_vector_scale.cpp
// Exécution :
//   ./bench_heap_vector_scale
//
// Règlages : constantes SIZES / BUDGET_MS / DEL_CAP / TOP_ITERS ci-dessous.
//
// Complexités :
//   insert    : O(log N) amorti (push_back + remontée)
//   pop       : O(log N)
//   top       : O(1)
//   deleteKey : O(N) (recherche linéaire) + O(log N) remontée/descente
//   => on peut monter à 1e8 (prévoir ~1 Go de RAM : data + buffer).

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <string>
#include <cstdint>
#include <stdexcept>

// ============================================================
//  CONFIGURATION — modifie ici, rien à passer en ligne de commande
// ============================================================
static const std::vector<std::size_t> SIZES = {
    100ULL, 1'000ULL, 10'000ULL, 100'000ULL,
    1'000'000ULL, 10'000'000ULL, 100'000'000ULL
};
static const double BUDGET_MS = 30000.0;   // budget max par phase (ms)
static const std::size_t DEL_CAP = 10'000; // deleteKey : cap du nombre d'ops
static const std::size_t TOP_ITERS = 50'000'000; // itérations pour top()
static const unsigned SEED = 42;
// ============================================================

// ---------- MinHeapVector (copie minimale de heap_vector.cpp) ----------
template <typename T>
class MinHeapVector {
    std::vector<T> arr;

    void heapify(int i) {
        // Version itérative pour éviter la récursion profonde (sécurité à 1e8)
        while (true) {
            int n = (int)arr.size();
            int l = 2*i + 1, r = 2*i + 2, smallest = i;
            if (l < n && arr[l] < arr[smallest]) smallest = l;
            if (r < n && arr[r] < arr[smallest]) smallest = r;
            if (smallest == i) break;
            std::swap(arr[i], arr[smallest]);
            i = smallest;
        }
    }
public:
    void reserve(std::size_t n) { arr.reserve(n); }
    bool empty() const { return arr.empty(); }
    int  size()  const { return (int)arr.size(); }

    void insert(const T& v) {
        arr.push_back(v);
        int i = (int)arr.size() - 1;
        while (i != 0 && arr[i] < arr[(i-1)/2]) {
            std::swap(arr[i], arr[(i-1)/2]);
            i = (i-1)/2;
        }
    }
    const T& top() const {
        if (arr.empty()) throw std::runtime_error("top() sur heap vide");
        return arr[0];
    }
    T pop() {
        if (arr.empty()) throw std::runtime_error("pop() sur heap vide");
        T root = arr[0];
        arr[0] = arr.back();
        arr.pop_back();
        if (!arr.empty()) heapify(0);
        return root;
    }
    bool deleteKey(const T& key) {
        int n = (int)arr.size(), idx = -1;
        for (int i = 0; i < n; ++i) if (arr[i] == key) { idx = i; break; }
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

static std::string fmt_u(std::uint64_t v) {
    std::string s = std::to_string(v), out;
    int c = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        out.push_back(s[i]);
        if (++c == 3 && i != 0) { out.push_back(' '); c = 0; }
    }
    std::reverse(out.begin(), out.end());
    return out;
}

struct Row {
    std::size_t N = 0;
    double ins_ms = 0; std::size_t ins_done = 0; bool ins_abort = false;
    double top_ms = 0; std::size_t top_iters = 0;
    double pop_ms = 0; std::size_t pop_done = 0; bool pop_abort = false;
    double del_ms = 0; std::size_t del_done = 0; bool del_abort = false;
};

static Row run_one(std::size_t N, std::mt19937_64& rng) {
    Row r; r.N = N;
    std::uniform_int_distribution<int> dist(0, 10'000'000);

    std::vector<int> data(N);
    for (auto& x : data) x = dist(rng);

    // ---- INSERT (budget) ----
    MinHeapVector<int> h;
    h.reserve(N); // évite les realloc : on mesure le vrai insert, pas la croissance
    {
        auto t0 = Clock::now();
        for (std::size_t i = 0; i < N; ++i) {
            h.insert(data[i]);
            ++r.ins_done;
            if ((i & 0x3F) == 0 && ms(t0, Clock::now()) > BUDGET_MS) {
                r.ins_abort = true; break;
            }
        }
        r.ins_ms = ms(t0, Clock::now());
    }

    // ---- TOP ----
    if (!h.empty()) {
        r.top_iters = TOP_ITERS;
        volatile long long sink = 0;
        auto t0 = Clock::now();
        for (std::size_t i = 0; i < r.top_iters; ++i) sink += h.top();
        r.top_ms = ms(t0, Clock::now());
        (void)sink;
    }

    // ---- POP (moitié, budget) ----
    {
        std::size_t half = h.size() / 2;
        volatile long long sink = 0;
        auto t0 = Clock::now();
        for (std::size_t i = 0; i < half; ++i) {
            sink += h.pop();
            ++r.pop_done;
            if ((i & 0x3F) == 0 && ms(t0, Clock::now()) > BUDGET_MS) {
                r.pop_abort = true; break;
            }
        }
        r.pop_ms = ms(t0, Clock::now());
        (void)sink;
    }

    // ---- DELETE KEY (cap + budget) ----
    std::size_t del_count = std::min<std::size_t>(N, DEL_CAP);
    if (del_count > 0) {
        MinHeapVector<int> hd;
        hd.reserve(del_count);
        std::vector<int> keys(del_count);
        for (std::size_t i = 0; i < del_count; ++i) { int v = (int)i; hd.insert(v); keys[i] = v; }
        std::shuffle(keys.begin(), keys.end(), rng);

        auto t0 = Clock::now();
        for (std::size_t i = 0; i < del_count; ++i) {
            hd.deleteKey(keys[i]);
            ++r.del_done;
            if ((i & 0x0F) == 0 && ms(t0, Clock::now()) > BUDGET_MS) {
                r.del_abort = true; break;
            }
        }
        r.del_ms = ms(t0, Clock::now());
    }
    return r;
}

int main() {
    std::mt19937_64 rng(SEED);

    std::cout << "================================================================\n";
    std::cout << "  Benchmark MinHeapVector (std::vector) — self-contained\n";
    std::cout << "  Budget/phase = " << BUDGET_MS << " ms | DEL_CAP = " << DEL_CAP
              << " | TOP_ITERS = " << fmt_u(TOP_ITERS) << "\n";
    std::cout << "  /!\\ insert/pop = O(log N), top = O(1), deleteKey = O(N)\n";
    std::cout << "  /!\\ N = 1e8 : ~1 Go de RAM (data + buffer)\n";
    std::cout << "================================================================\n\n";

    std::vector<Row> rows;
    rows.reserve(SIZES.size());

    for (std::size_t N : SIZES) {
        std::cout << ">>> Exécution N = " << fmt_u(N) << " ... " << std::flush;
        Row r = run_one(N, rng);
        rows.push_back(r);
        std::cout << "OK";
        if (r.ins_abort) std::cout << " [insert ABORT]";
        if (r.pop_abort) std::cout << " [pop ABORT]";
        if (r.del_abort) std::cout << " [deleteKey ABORT]";
        std::cout << "\n";
    }

    std::cout << "\nRÉSULTATS DÉTAILLÉS\n-------------------\n\n";
    std::cout << std::fixed << std::setprecision(2);

    for (const auto& r : rows) {
        double ins_ns = r.ins_done ? r.ins_ms * 1e6 / (double)r.ins_done : 0.0;
        double top_ns = r.top_iters ? r.top_ms * 1e6 / (double)r.top_iters : 0.0;
        double pop_ns = r.pop_done ? r.pop_ms * 1e6 / (double)r.pop_done : 0.0;
        double del_ns = r.del_done ? r.del_ms * 1e6 / (double)r.del_done : 0.0;

        std::cout << "N = " << fmt_u(r.N) << "\n";
        std::cout << "  Insert   : " << std::setw(12) << r.ins_ms << " ms | "
                  << std::setw(14) << ins_ns << " ns/op ("
                  << fmt_u(r.ins_done) << " / " << fmt_u(r.N)
                  << (r.ins_abort ? ", ABORT" : "") << ")\n";
        std::cout << "  Top      : " << std::setw(12) << r.top_ms << " ms | "
                  << std::setw(14) << top_ns << " ns/op ("
                  << fmt_u(r.top_iters) << " iters)\n";
        std::cout << "  Pop      : " << std::setw(12) << r.pop_ms << " ms | "
                  << std::setw(14) << pop_ns << " ns/op ("
                  << fmt_u(r.pop_done) << (r.pop_abort ? ", ABORT" : "") << ")\n";
        std::cout << "  DeleteKey: " << std::setw(12) << r.del_ms << " ms | "
                  << std::setw(14) << del_ns << " ns/op ("
                  << fmt_u(r.del_done) << (r.del_abort ? ", ABORT" : "") << ")\n\n";
    }

    // Tableau récap
    std::cout << "TABLEAU RÉCAPITULATIF (ns/op)\n-----------------------------\n";
    std::cout << std::left  << std::setw(12) << "N"
              << std::right << std::setw(16) << "insert"
              << std::setw(14) << "top"
              << std::setw(16) << "pop"
              << std::setw(16) << "deleteKey" << "\n";
    std::cout << std::string(74, '-') << "\n";
    auto nso = [](const Row& r, int kind) -> double {
        switch (kind) {
            case 0: return r.ins_done ? r.ins_ms * 1e6 / (double)r.ins_done : 0.0;
            case 1: return r.top_iters ? r.top_ms * 1e6 / (double)r.top_iters : 0.0;
            case 2: return r.pop_done ? r.pop_ms * 1e6 / (double)r.pop_done : 0.0;
            default:return r.del_done ? r.del_ms * 1e6 / (double)r.del_done : 0.0;
        }
    };
    for (const auto& r : rows) {
        std::cout << std::left  << std::setw(12) << fmt_u(r.N)
                  << std::right << std::setw(16) << nso(r,0)
                  << std::setw(14) << nso(r,1)
                  << std::setw(16) << nso(r,2)
                  << std::setw(16) << nso(r,3) << "\n";
    }

    // Ratios
    std::cout << "\nRATIOS ns/op entre N et N/10\n-----------------------------\n";
    std::cout << "Attendu MinHeapVector : top ~1 (O(1)) | insert/pop ~1.1-1.3 (log N) |\n";
    std::cout << "                        deleteKey ~10 (O(N))\n\n";
    std::cout << std::left  << std::setw(18) << "N"
              << std::right << std::setw(14) << "insert"
              << std::setw(14) << "top"
              << std::setw(14) << "pop"
              << std::setw(14) << "deleteKey" << "\n";
    std::cout << std::string(74, '-') << "\n";
    for (std::size_t i = 1; i < rows.size(); ++i) {
        const auto& a = rows[i-1];
        const auto& b = rows[i];
        auto ratio = [](double x, double y) -> double { return (x > 0.0 && y > 0.0) ? y / x : 0.0; };
        std::cout << std::left  << std::setw(18) << (fmt_u(b.N) + " / " + fmt_u(a.N))
                  << std::right << std::setw(14) << ratio(nso(a,0), nso(b,0))
                  << std::setw(14) << ratio(nso(a,1), nso(b,1))
                  << std::setw(14) << ratio(nso(a,2), nso(b,2))
                  << std::setw(14) << ratio(nso(a,3), nso(b,3)) << "\n";
    }

    std::cout << "\nTerminé.\n";
    return 0;
}

/*
resultat terminal:
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ ./bench_heap_vector_scale
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

N = 1 000
  Insert   :         0.04 ms |          37.45 ns/op (1 000 / 1 000)
  Top      :        47.90 ms |           0.96 ns/op (50 000 000 iters)
  Pop      :         0.04 ms |          87.72 ns/op (500)
  DeleteKey:         0.18 ms |         181.02 ns/op (1 000)

N = 10 000
  Insert   :         0.26 ms |          25.76 ns/op (10 000 / 10 000)
  Top      :        55.39 ms |           1.11 ns/op (50 000 000 iters)
  Pop      :         0.55 ms |         110.56 ns/op (5 000)
  DeleteKey:        13.39 ms |        1338.80 ns/op (10 000)

N = 100 000
  Insert   :         1.84 ms |          18.42 ns/op (100 000 / 100 000)
  Top      :        36.46 ms |           0.73 ns/op (50 000 000 iters)
  Pop      :         7.46 ms |         149.28 ns/op (50 000)
  DeleteKey:        12.34 ms |        1233.65 ns/op (10 000)

N = 1 000 000
  Insert   :        24.89 ms |          24.89 ns/op (1 000 000 / 1 000 000)
  Top      :        30.68 ms |           0.61 ns/op (50 000 000 iters)
  Pop      :        94.95 ms |         189.90 ns/op (500 000)
  DeleteKey:         9.22 ms |         921.76 ns/op (10 000)

N = 10 000 000
  Insert   :       204.15 ms |          20.42 ns/op (10 000 000 / 10 000 000)
  Top      :        22.92 ms |           0.46 ns/op (50 000 000 iters)
  Pop      :      2326.25 ms |         465.25 ns/op (5 000 000)
  DeleteKey:        11.38 ms |        1137.78 ns/op (10 000)

N = 100 000 000
  Insert   :      2423.78 ms |          24.24 ns/op (100 000 000 / 100 000 000)
  Top      :        31.10 ms |           0.62 ns/op (50 000 000 iters)
  Pop      :     30000.01 ms |         700.13 ns/op (42 849 281, ABORT)
  DeleteKey:        11.77 ms |        1176.53 ns/op (10 000)

TABLEAU RÉCAPITULATIF (ns/op)
-----------------------------
N                     insert           top             pop       deleteKey
--------------------------------------------------------------------------
100                    24.90          0.66           93.08           68.75
1 000                  37.45          0.96           87.72          181.02
10 000                 25.76          1.11          110.56         1338.80
100 000                18.42          0.73          149.28         1233.65
1 000 000              24.89          0.61          189.90          921.76
10 000 000             20.42          0.46          465.25         1137.78
100 000 000            24.24          0.62          700.13         1176.53

RATIOS ns/op entre N et N/10
-----------------------------
Attendu MinHeapVector : top ~1 (O(1)) | insert/pop ~1.1-1.3 (log N) |
                        deleteKey ~10 (O(N))

N                         insert           top           pop     deleteKey
--------------------------------------------------------------------------
1 000 / 100                 1.50          1.46          0.94          2.63
10 000 / 1 000              0.69          1.16          1.26          7.40
100 000 / 10 000            0.72          0.66          1.35          0.92
1 000 000 / 100 000          1.35          0.84          1.27          0.75
10 000 000 / 1 000 000          0.82          0.75          2.45          1.23
100 000 000 / 10 000 000          1.19          1.36          1.50          1.03

Terminé.
*/