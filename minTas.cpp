// minTas.cpp
// Min Heap simple et lisible (version étudiant 2e année ingénierie)
// Compilation : g++ -std=c++17 -O2 -Wall -Wextra -o minTas minTas.cpp

#include <iostream>
#include <vector>
#include <stdexcept>

template <typename T>
class MinHeap {
private:
    std::vector<T> arr;   // stockage interne

    // Restaure la propriété du min heap à partir de l'indice i (descente)
    void heapify(int i) {
        int n = static_cast<int>(arr.size());
        int smallest = i;
        int left  = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n && arr[left] < arr[smallest])  smallest = left;
        if (right < n && arr[right] < arr[smallest]) smallest = right;

        if (smallest != i) {
            std::swap(arr[i], arr[smallest]);
            heapify(smallest);
        }
    }

public:
    MinHeap() = default;

    explicit MinHeap(const std::vector<T>& input) {
        arr = input;
        // Construit le heap en O(n)
        for (int i = static_cast<int>(arr.size()) / 2 - 1; i >= 0; --i) {
            heapify(i);
        }
    }

    bool empty() const { return arr.empty(); }
    int  size()  const { return static_cast<int>(arr.size()); }

    // Insertion : on ajoute à la fin puis on remonte
    void insert(const T& value) {
        arr.push_back(value);
        int i = static_cast<int>(arr.size()) - 1;
        while (i != 0 && arr[i] < arr[(i - 1) / 2]) {
            std::swap(arr[i], arr[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    // Retourne le minimum sans le retirer
    const T& top() const {
        if (arr.empty()) throw std::runtime_error("MinHeap::top() : heap vide");
        return arr[0];
    }

    // Retire et retourne le minimum
    T pop() {
        if (arr.empty()) throw std::runtime_error("MinHeap::pop() : heap vide");
        T root = arr[0];
        arr[0] = arr.back();
        arr.pop_back();
        if (!arr.empty()) heapify(0);
        return root;
    }

    // Supprime une clé arbitraire (première occurrence)
    bool deleteKey(const T& key) {
        int n = static_cast<int>(arr.size());
        int index = -1;
        for (int i = 0; i < n; ++i) {
            if (arr[i] == key) { index = i; break; }
        }
        if (index == -1) return false;

        if (index == n - 1) { arr.pop_back(); return true; }

        arr[index] = arr.back();
        arr.pop_back();

        // Selon la valeur, il faut remonter ou descendre
        int i = index;
        // Remonter si nécessaire
        while (i != 0 && arr[i] < arr[(i - 1) / 2]) {
            std::swap(arr[i], arr[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
        // Sinon descendre
        heapify(i);
        return true;
    }

    void print(std::ostream& os = std::cout) const {
        os << "Min Heap: [ ";
        for (const auto& v : arr) os << v << ' ';
        os << "]\n";
    }

    const std::vector<T>& data() const { return arr; }
};

// -------- Démonstration --------
int main() {
    MinHeap<int> h;
    std::cout << "Insertion successive de 88, 1, 10 :\n";
    for (int v : {88, 1, 10}) { h.insert(v); h.print(); }

    std::cout << "\nInsertion successive de 2, 99, 100, 102, 7 :\n";
    for (int v : {2, 99, 100, 102, 7}) { h.insert(v); h.print(); }

    std::cout << "\nTop = " << h.top() << "\n";
    std::cout << "Pop = " << h.pop() << "\n";
    h.print();

    h.deleteKey(99);
    std::cout << "Après deleteKey(99) :\n";
    h.print();

    return 0;
}
/*
Resultat Terminal:
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ ./minTas
Insertion successive de 88, 1, 10 :
Min Heap: [ 88 ]
Min Heap: [ 1 88 ]
Min Heap: [ 1 88 10 ]

Insertion successive de 2, 99, 100, 102, 7 :
Min Heap: [ 1 2 10 88 ]
Min Heap: [ 1 2 10 88 99 ]
Min Heap: [ 1 2 10 88 99 100 ]
Min Heap: [ 1 2 10 88 99 100 102 ]
Min Heap: [ 1 2 10 7 99 100 102 88 ]

Top = 1
Pop = 1
Min Heap: [ 2 7 10 88 99 100 102 ]
Après deleteKey(99) :
Min Heap: [ 2 7 10 88 102 100 ]
*/
