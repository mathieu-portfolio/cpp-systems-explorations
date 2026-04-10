#include "arena.hpp"
#include <iostream>
#include <new> // placement new

struct Vec3 {
  float x, y, z;

  Vec3(float x, float y, float z)
    : x(x), y(y), z(z)
  {
    std::cout << "Vec3 constructed\n";
  }

  ~Vec3()
  {
    std::cout << "Vec3 destroyed\n";
  }
};

int main()
{
  Arena arena(1024, alignof(std::max_align_t));

  std::cout << "=== Frame start ===\n";

  // Persistent allocation (lives until reset)
  void* mem = arena.allocate(sizeof(Vec3), alignof(Vec3));
  Vec3* persistent = new (mem) Vec3(1.f, 2.f, 3.f);

  {
    // Temporary scope
    auto marker = arena.mark();

    std::cout << "--- Temporary scope ---\n";

    void* temp_mem = arena.allocate(sizeof(Vec3), alignof(Vec3));
    Vec3* temp = new (temp_mem) Vec3(4.f, 5.f, 6.f);

    // Manual destruction (important!)
    temp->~Vec3();

    // Rewind frees memory
    arena.rewind(marker);

    std::cout << "--- Scope rewound ---\n";
  }

  std::cout << "=== Frame end ===\n";

  // Destroy persistent object before reset
  persistent->~Vec3();

  arena.reset();

  std::cout << "Arena reset\n";
}