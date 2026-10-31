// heap_vector.cpp — Min Heap avec std::vector (implémentation de référence)
// Compilation : g++ -std=c++17 -O2 -Wall -Wextra -o heap_vector heap_vector.cpp

#include <iostream>
#include <vector>
#include <stdexcept>

template <typename T>
class MinHeapVector {
    std::vector<T> arr;

    void heapify(int i) {
        int n = (int)arr.size();
        int l = 2*i + 1, r = 2*i + 2, smallest = i;
        if (l < n && arr[l] < arr[smallest]) smallest = l;
        if (r < n && arr[r] < arr[smallest]) smallest = r;
        if (smallest != i) {
            std::swap(arr[i], arr[smallest]);
            heapify(smallest);
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

    void print(std::ostream& os = std::cout) const {
        os << "MinHeapVector: [ ";
        for (const auto& v : arr) os << v << ' ';
        os << "]\n";
    }
};

int main() {
    MinHeapVector<int> h;
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
Resultat terminal:
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/TAS/MinTas$ ./heap_vector
MinHeapVector: [ 1 2 10 7 99 100 102 88 ]
top = 1
pop = 1
MinHeapVector: [ 2 7 10 88 99 100 102 ]
MinHeapVector: [ 2 7 10 88 102 100 ]
*/