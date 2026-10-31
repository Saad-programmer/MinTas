// heap_map.cpp — Min Heap avec std::map<int, T> (clé = index dans le tableau virtuel)
// Justification : std::map fournit un accès par clé en O(log n).
// On stocke le heap comme un tableau indexé 0..n-1 dans une map.
// Résultat : insert/pop en O(log² n) (O(log n) par accès × O(log n) niveaux).
// Moins rapide que vector mais conserve une complexité logarithmique.
// Compilation : g++ -std=c++17 -O2 -Wall -Wextra -o heap_map heap_map.cpp

#include <iostream>
#include <map>
#include <stdexcept>

template <typename T>
class MinHeapMap {
    std::map<int, T> tree;   // index -> valeur

    T&       ref(int i)       { return tree.at(i); }
    const T& ref(int i) const { return tree.at(i); }

    void heapify(int i) {
        int n = (int)tree.size();
        int l = 2*i+1, r = 2*i+2, smallest = i;
        if (l < n && ref(l) < ref(smallest)) smallest = l;
        if (r < n && ref(r) < ref(smallest)) smallest = r;
        if (smallest != i) {
            std::swap(ref(i), ref(smallest));
            heapify(smallest);
        }
    }

public:
    bool empty() const { return tree.empty(); }
    int  size()  const { return (int)tree.size(); }

    void insert(const T& v) {
        int idx = (int)tree.size();
        tree[idx] = v;
        int i = idx;
        while (i != 0) {
            int p = (i-1)/2;
            if (ref(p) <= ref(i)) break;
            std::swap(ref(i), ref(p));
            i = p;
        }
    }

    const T& top() const {
        if (tree.empty()) throw std::runtime_error("top() sur heap vide");
        return tree.at(0);
    }

    T pop() {
        if (tree.empty()) throw std::runtime_error("pop() sur heap vide");
        T root = tree.at(0);
        int last = (int)tree.size() - 1;
        if (last == 0) { tree.clear(); return root; }
        tree[0] = tree.at(last);
        tree.erase(last);
        heapify(0);
        return root;
    }

    bool deleteKey(const T& key) {
        int idx = -1;
        for (const auto& kv : tree) {
            if (kv.second == key) { idx = kv.first; break; }
        }
        if (idx == -1) return false;
        int last = (int)tree.size() - 1;
        if (idx == last) { tree.erase(last); return true; }

        tree[idx] = tree.at(last);
        tree.erase(last);
        int i = idx;
        while (i != 0) {
            int p = (i-1)/2;
            if (ref(p) <= ref(i)) break;
            std::swap(ref(i), ref(p));
            i = p;
        }
        heapify(i);
        return true;
    }

    void print(std::ostream& os = std::cout) const {
        os << "MinHeapMap: [ ";
        for (const auto& kv : tree) os << kv.second << ' ';
        os << "]\n";
    }
};

int main() {
    MinHeapMap<int> h;
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
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ ./heap_map
MinHeapMap: [ 1 2 10 7 99 100 102 88 ]
top = 1
pop = 1
MinHeapMap: [ 2 7 10 88 99 100 102 ]
MinHeapMap: [ 2 7 10 88 102 100 ]
*/
