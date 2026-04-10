#include <cstddef>
#include <limits>

class Arena {
private:
  char* _buffer;
  size_t _capacity;
  size_t _max_alignment;
  size_t _offset;

public:
  class Marker {
  private:
    size_t _offset;
    const Arena* _owner;

    Marker(size_t offset, const Arena* owner) : _offset(offset), _owner(owner) { }

    friend class Arena;
  };

  class ScopedRewind {
  private:
    Marker _marker;
    Arena* _owner;
    bool _active;

    ScopedRewind(Arena* owner) : _marker(owner->mark()), _owner(owner), _active(true) { }

    friend class Arena;

  public:
    ~ScopedRewind() noexcept {
      if (_active) {
        _owner->rewind(_marker);
      }
    }

    ScopedRewind(const ScopedRewind&) = delete;
    ScopedRewind& operator=(const ScopedRewind&) = delete;
    ScopedRewind(ScopedRewind&&) = delete;
    ScopedRewind& operator=(ScopedRewind&&) = delete;

    void dismiss() noexcept { _active = false; }
  };

  explicit Arena(size_t capacity, size_t max_alignment);
  ~Arena();

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  Arena(Arena&&) = delete;
  Arena& operator=(Arena&&) = delete;

  void* allocate(size_t size, size_t alignment);
  void reset();
  Marker mark() const;
  void rewind(Marker marker);
  [[nodiscard]] ScopedRewind scoped_rewind();

  size_t capacity() const noexcept { return _capacity; }
  size_t max_alignment() const noexcept { return _max_alignment; }
  size_t used() const noexcept { return _offset; }
  size_t remaining() const noexcept { return _capacity - _offset; }

  // Returns raw storage for T, does not construct objects
  template<typename T>
  void* allocate(size_t count = 1)
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
    return ptr;
  }
};