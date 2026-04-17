#pragma once

#include <utility>

struct Tracked {
    static inline int default_ctor = 0;
    static inline int value_ctor = 0;
    static inline int copy_ctor = 0;
    static inline int move_ctor = 0;
    static inline int copy_assign = 0;
    static inline int move_assign = 0;
    static inline int dtor = 0;
    static inline int alive = 0;

    int value = 0;

    static void reset() {
        default_ctor = 0;
        value_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_assign = 0;
        move_assign = 0;
        dtor = 0;
        alive = 0;
    }

    Tracked() : value(0) {
        ++default_ctor;
        ++alive;
    }

    explicit Tracked(int v) : value(v) {
        ++value_ctor;
        ++alive;
    }

    Tracked(const Tracked& other) : value(other.value) {
        ++copy_ctor;
        ++alive;
    }

    Tracked(Tracked&& other) noexcept : value(other.value) {
        other.value = -9999;
        ++move_ctor;
        ++alive;
    }

    Tracked& operator=(const Tracked& other) {
        value = other.value;
        ++copy_assign;
        return *this;
    }

    Tracked& operator=(Tracked&& other) noexcept {
        value = other.value;
        other.value = -9999;
        ++move_assign;
        return *this;
    }

    ~Tracked() {
        ++dtor;
        --alive;
    }
};

struct PairLike {
    int a;
    int b;

    PairLike(int x, int y) : a(x), b(y) {}
};