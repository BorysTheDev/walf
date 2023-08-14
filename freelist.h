#ifndef FREELIST_H
#define FREELIST_H

#include <new>
#include <atomic>

namespace walf {

template <class T>
class freelist {
    union element_t {
        T el;
        element_t* next;
    };
public:
    freelist(void* root) : _head(reinterpret_cast<std::byte*>(root)), _next(nullptr) {
    }

    T* allocate() noexcept {
        while (auto tmp_next = _next.load()) {
            if (_next.compare_exchange_weak(tmp_next, tmp_next->next)) {
                return &(tmp_next->el);
            }
        }
        auto tmp_head = _head.fetch_add(sizeof(element_t));
        return &((new(tmp_head) element_t)->el);
    }

    void deallocate(T* pos) noexcept {
        auto* el = std::launder(reinterpret_cast<element_t*>(pos));
        el->next = _next;
        _next = el;
    }

private:
    std::atomic<std::byte*> _head;
    std::atomic<element_t*> _next;
};

} // namespace walf

#endif // FREELIST_H
