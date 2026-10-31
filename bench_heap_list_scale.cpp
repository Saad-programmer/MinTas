// bench_heap_list_scale.cpp
// Benchmark MinHeapList (std::list) — 100% self-contained, AUCUN argument CLI.
//
// Compilation :
//   g++ -std=c++17 -O2 -Wall -Wextra -o bench_heap_list_scale bench_heap_list_scale.cpp
// Exécution :
//   ./bench_heap_list_scale
//
// Toutes les tailles et le budget temps se règlent dans les constantes
// ci-dessous (SIZES, BUDGET_MS, DEL_CAP, TOP_ITERS).
//
// !!! COMPLEXITÉ !!!
//   insert    : O(N) par op (std::next dans la remontée)
//   pop       : O(N) par op (std::next dans heapify)
//   deleteKey : O(N) par op (recherche + std::next + heapify)
//   top       : O(1)
//   => inutile de monter à 1e8 : ça ne terminerait pas.

#include <iostream>
#include <list>
#include <iterator>
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
    100ULL, 300ULL, 1'000ULL, 3'000ULL, 10'000ULL, 30'000ULL, 100'000ULL
};
static const double BUDGET_MS = 5000.0;   // budget max par phase (ms)
static const std::size_t DEL_CAP = 2'000; // deleteKey : cap du nombre d'ops
static const std::size_t TOP_ITERS = 5'000'000; // itérations pour top()
static const unsigned SEED = 42;
// ============================================================

// ---------- MinHeapList (copie minimale de heap_list.cpp) ----------
template <typename T>
class MinHeapList {
    std::list<T> lst;

    typename std::list<T>::iterator at(int i) {
        return std::next(lst.begin(), i);
    }
    void heapify(int i) {
        int n = (int)lst.size();
        int l = 2*i + 1, r = 2*i + 2, smallest = i;
        auto itSmallest = at(smallest);
        if (l < n) { auto itL = at(l); if (*itL < *itSmallest) { smallest = l; itSmallest = itL; } }
        if (r < n) { auto itR = at(r); if (*itR < *itSmallest) { smallest = r; itSmallest = itR; } }
        if (smallest != i) { std::swap(*at(i), *at(smallest)); heapify(smallest); }
    }
public:
    bool empty() const { return lst.empty(); }
    int  size()  const { return (int)lst.size(); }

    void insert(const T& v) {
        lst.push_back(v);
        int i = (int)lst.size() - 1;
        while (i != 0) {
            int p = (i - 1) / 2;
            auto itI = at(i), itP = at(p);
            if (*itP <= *itI) break;
            std::swap(*itI, *itP);
            i = p;
        }
    }
    const T& top() const {
        if (lst.empty()) throw std::runtime_error("top() sur heap vide");
        return lst.front();
    }
    T pop() {
        if (lst.empty()) throw std::runtime_error("pop() sur heap vide");
        T root = lst.front();
        *lst.begin() = lst.back();
        lst.pop_back();
        if (!lst.empty()) heapify(0);
        return root;
    }
    bool deleteKey(const T& key) {
        int n = (int)lst.size(), idx = -1, k = 0;
        for (auto it = lst.begin(); it != lst.end(); ++it, ++k) {
            if (*it == key) { idx = k; break; }
        }
        if (idx == -1) return false;
        if (idx == n-1) { lst.pop_back(); return true; }
        *at(idx) = lst.back();
        lst.pop_back();
        int i = idx;
        while (i != 0) {
            int p = (i-1)/2;
            auto itI = at(i), itP = at(p);
            if (*itP <= *itI) break;
            std::swap(*itI, *itP);
            i = p;
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
    MinHeapList<int> h;
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
        MinHeapList<int> hd;
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
    std::cout << "  Benchmark MinHeapList (std::list) — self-contained\n";
    std::cout << "  Budget/phase = " << BUDGET_MS << " ms | DEL_CAP = " << DEL_CAP
              << " | TOP_ITERS = " << fmt_u(TOP_ITERS) << "\n";
    std::cout << "  /!\\ insert/pop/deleteKey = O(N) par op, top = O(1)\n";
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
    std::cout << "Attendu MinHeapList : top ~1 | insert/pop/delete ~10 (~O(N) par op)\n\n";
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
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ ./bench_heap_list_scale
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

RÉSULTATS DÉTAILLÉS
-------------------

N = 100
  Insert   :         0.02 ms |         181.35 ns/op (100 / 100)
  Top      :         2.44 ms |           0.49 ns/op (5 000 000 iters)
  Pop      :         0.03 ms |         516.62 ns/op (50)
  DeleteKey:         0.03 ms |         259.62 ns/op (100)

N = 300
  Insert   :         0.19 ms |         630.95 ns/op (300 / 300)
  Top      :         3.02 ms |           0.60 ns/op (5 000 000 iters)
  Pop      :         0.25 ms |        1689.33 ns/op (150)
  DeleteKey:         0.25 ms |         835.13 ns/op (300)

N = 1 000
  Insert   :         1.96 ms |        1964.44 ns/op (1 000 / 1 000)
  Top      :         2.50 ms |           0.50 ns/op (5 000 000 iters)
  Pop      :         4.17 ms |        8331.83 ns/op (500)
  DeleteKey:         3.80 ms |        3799.61 ns/op (1 000)

N = 3 000
  Insert   :        27.17 ms |        9058.12 ns/op (3 000 / 3 000)
  Top      :         4.73 ms |           0.95 ns/op (5 000 000 iters)
  Pop      :        41.66 ms |       27775.36 ns/op (1 500)
  DeleteKey:        15.55 ms |        7773.94 ns/op (2 000)

N = 10 000
  Insert   :       266.61 ms |       26660.87 ns/op (10 000 / 10 000)
  Top      :         2.40 ms |           0.48 ns/op (5 000 000 iters)
  Pop      :       576.26 ms |      115251.55 ns/op (5 000)
  DeleteKey:        17.60 ms |        8801.36 ns/op (2 000)

N = 30 000
  Insert   :      2817.60 ms |       93920.02 ns/op (30 000 / 30 000)
  Top      :         2.23 ms |           0.45 ns/op (5 000 000 iters)
  Pop      :      5008.27 ms |      391240.59 ns/op (12 801, ABORT)
  DeleteKey:        17.45 ms |        8727.00 ns/op (2 000)

N = 100 000
  Insert   :      5019.40 ms |      123505.75 ns/op (40 641 / 100 000, ABORT)
  Top      :        11.06 ms |           2.21 ns/op (5 000 000 iters)
  Pop      :      5025.67 ms |      643574.40 ns/op (7 809, ABORT)
  DeleteKey:        18.69 ms |        9346.90 ns/op (2 000)

TABLEAU RÉCAPITULATIF (ns/op)
-----------------------------
N                     insert           top             pop       deleteKey
--------------------------------------------------------------------------
100                   181.35          0.49          516.62          259.62
300                   630.95          0.60         1689.33          835.13
1 000                1964.44          0.50         8331.83         3799.61
3 000                9058.12          0.95        27775.36         7773.94
10 000              26660.87          0.48       115251.55         8801.36
30 000              93920.02          0.45       391240.59         8727.00
100 000            123505.75          2.21       643574.40         9346.90

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

Terminé.
*/