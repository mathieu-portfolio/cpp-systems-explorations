#include <cstddef>
#include <limits>

class Arena {
private:
  char* buffer;
  size_t capacity;
  size_t offset;

public:
  explicit Arena(size_t capacity);
  ~Arena();

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  Arena(Arena&&) = delete;
  Arena& operator=(Arena&&) = delete;

  void* allocate(size_t size, size_t alignment);
  void reset();

  // Returns raw storage for T, does not construct objects
  template<typename T>
  T* allocate(size_t count = 1)
  {
    if (count == 0)
    {
      return nullptr;
    }

    constexpr size_t limit = std::numeric_limits<size_t>::max();
    if (count > limit / sizeof(T))
    {
      return nullptr;
    }

    size_t size = sizeof(T) * count;
    size_t alignment = alignof(T);

    void* ptr = allocate(size, alignment);
    T* typed_ptr = reinterpret_cast<T*>(ptr);

    return typed_ptr;
  }
};