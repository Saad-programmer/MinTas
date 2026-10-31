// heap_list.cpp — Min Heap avec std::list
// ATTENTION : std::list n'offre PAS d'accès aléatoire.
// On doit utiliser std::next(lst.begin(), i) → O(i).
// Résultat : insert/pop en O(n) au lieu de O(log n). Version pédagogique seulement.
// Compilation : g++ -std=c++17 -O2 -Wall -Wextra -o heap_list heap_list.cpp

#include <iostream>
#include <list>
#include <iterator>
#include <stdexcept>

template <typename T>
class MinHeapList {
    std::list<T> lst;

    // Accès par indice : O(i) car std::list n'est pas contigu
    typename std::list<T>::iterator at(int i) {
        return std::next(lst.begin(), i);
    }

    void heapify(int i) {
        int n = (int)lst.size();
        int l = 2*i + 1, r = 2*i + 2, smallest = i;

        auto itSmallest = at(smallest);
        if (l < n) {
            auto itL = at(l);
            if (*itL < *itSmallest) { smallest = l; itSmallest = itL; }
        }
        if (r < n) {
            auto itR = at(r);
            if (*itR < *itSmallest) { smallest = r; itSmallest = itR; }
        }
        if (smallest != i) {
            std::swap(*at(i), *at(smallest));
            heapify(smallest);
        }
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
        // remplacer la racine par le dernier
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
        // remonter
        while (i != 0) {
            int p = (i-1)/2;
            auto itI = at(i), itP = at(p);
            if (*itP <= *itI) break;
            std::swap(*itI, *itP);
            i = p;
        }
        // descendre
        heapify(i);
        return true;
    }

    void print(std::ostream& os = std::cout) const {
        os << "MinHeapList: [ ";
        for (const auto& v : lst) os << v << ' ';
        os << "]\n";
    }
};

int main() {
    MinHeapList<int> h;
    for (int v : {88, 1, 10, 2, 99, 100, 102, 7}) h.insert(v);
    h.print();
    std::cout << "top = " << h.top() << "\n";
    std::cout << "pop = " << h.pop() << "\n";
    h.print();
    h.deleteKey(99);
    h.print();
    return 0;
}

/*
resultat terminal:
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ ./heap_list
MinHeapList: [ 1 2 10 7 99 100 102 88 ]
top = 1
pop = 1
MinHeapList: [ 2 7 10 88 99 100 102 ]
MinHeapList: [ 2 7 10 88 102 100 ]
*/