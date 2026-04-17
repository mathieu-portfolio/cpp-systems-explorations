#include "vector.hpp"
#include <iostream>

int main() {
    Vector<int> v;

    // insert elements
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    // iterate (range-based)
    std::cout << "values: ";
    for (const auto& x : v) {
        std::cout << x << " ";
    }
    std::cout << "\n";

    // mutate via operator[]
    v[1] = 42;

    // iterate again (iterator style)
    std::cout << "after update: ";
    for (auto it = v.begin(); it != v.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // reserve capacity
    v.reserve(10);
    std::cout << "size=" << v.size()
              << " capacity=" << v.capacity() << "\n";

    return 0;
}