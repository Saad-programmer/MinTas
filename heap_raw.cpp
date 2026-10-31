// heap_raw.cpp — Min Heap sans STL : tableau dynamique géré à la main (new/delete)
// Compilation : g++ -std=c++17 -O2 -Wall -Wextra -o heap_raw heap_raw.cpp

#include <iostream>
#include <stdexcept>

template <typename T>
class MinHeapRaw {
    T*   buf;        // tableau dynamique brut
    int  sz;         // taille actuelle
    int  cap;        // capacité allouée

    void grow() {
        int newCap = (cap == 0) ? 8 : cap * 2;
        T* newBuf = new T[newCap];
        for (int i = 0; i < sz; ++i) newBuf[i] = buf[i];
        delete[] buf;
        buf = newBuf;
        cap = newCap;
    }

    void heapify(int i) {
        int l = 2*i+1, r = 2*i+2, smallest = i;
        if (l < sz && buf[l] < buf[smallest]) smallest = l;
        if (r < sz && buf[r] < buf[smallest]) smallest = r;
        if (smallest != i) {
            T tmp = buf[i]; buf[i] = buf[smallest]; buf[smallest] = tmp;
            heapify(smallest);
        }
    }

public:
    MinHeapRaw() : buf(nullptr), sz(0), cap(0) {}

    // Interdiction de la copie par simplicité (règle des 3/5 non traitée ici)
    MinHeapRaw(const MinHeapRaw&) = delete;
    MinHeapRaw& operator=(const MinHeapRaw&) = delete;

    ~MinHeapRaw() { delete[] buf; }

    bool empty() const { return sz == 0; }
    int  size()  const { return sz; }

    void insert(const T& v) {
        if (sz == cap) grow();
        buf[sz++] = v;
        int i = sz - 1;
        while (i != 0 && buf[i] < buf[(i-1)/2]) {
            T tmp = buf[i]; buf[i] = buf[(i-1)/2]; buf[(i-1)/2] = tmp;
            i = (i-1)/2;
        }
    }

    const T& top() const {
        if (sz == 0) throw std::runtime_error("top() sur heap vide");
        return buf[0];
    }

    T pop() {
        if (sz == 0) throw std::runtime_error("pop() sur heap vide");
        T root = buf[0];
        buf[0] = buf[--sz];
        if (sz > 0) heapify(0);
        return root;
    }

    bool deleteKey(const T& key) {
        int idx = -1;
        for (int i = 0; i < sz; ++i) if (buf[i] == key) { idx = i; break; }
        if (idx == -1) return false;
        if (idx == sz-1) { --sz; return true; }

        buf[idx] = buf[--sz];
        int i = idx;
        while (i != 0 && buf[i] < buf[(i-1)/2]) {
            T tmp = buf[i]; buf[i] = buf[(i-1)/2]; buf[(i-1)/2] = tmp;
            i = (i-1)/2;
        }
        heapify(i);
        return true;
    }

    void print(std::ostream& os = std::cout) const {
        os << "MinHeapRaw: [ ";
        for (int i = 0; i < sz; ++i) os << buf[i] << ' ';
        os << "]\n";
    }
};

int main() {
    MinHeapRaw<int> h;
    for (int v : {88, 1, 10, 2, 99, 100, 102, 7}) h.insert(v);
    h.print();
    std::cout << "top = " << h.top() << "\n";
    std::cout << "pop = " << h.pop() << "\n";
    h.print();
    h.deleteKey(99);
    h.print();
    return 0;
}

