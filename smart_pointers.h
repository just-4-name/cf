#pragma once

#include <iostream>

template <typename T>
struct DefaultDeleter {
    using Alloc = std::allocator<T>;
    using AllocTraits = std::allocator_traits<Alloc>;

    DefaultDeleter() = default;

    void operator()(T* ptr) const {
        Alloc alloc = Alloc();
        AllocTraits::destroy(alloc, ptr);
        AllocTraits::deallocate(alloc, ptr, 1);
    }
};

namespace cb {
    struct BaseControlBlock {
        int cnt_shared = 1;
        int cnt_weak = 0;
        BaseControlBlock() = default;
        virtual void useDeleter(void*) = 0;
        virtual void destroy_cb() = 0;
        virtual ~BaseControlBlock() = default;
    };

    template <typename T, typename Deleter = DefaultDeleter<T>,
            typename Alloc = std::allocator<T>>
    struct ControlBlockRegular : BaseControlBlock {
        using CurCB = ControlBlockRegular<T, Deleter, Alloc>;
        using CBAlloc =
                typename std::allocator_traits<Alloc>::template rebind_alloc<CurCB>;
        using CBAllocTraits = std::allocator_traits<CBAlloc>;
        Deleter deleter;
        Alloc alloc;

        ControlBlockRegular(const Deleter& deleter = Deleter(),
                            const Alloc& alloc = Alloc())
                : BaseControlBlock(), deleter(deleter), alloc(alloc) {}

        void useDeleter(void* pointer) override {
            deleter(reinterpret_cast<T*>(pointer));
        }

        void destroy_cb() override {
            CBAlloc tmp_alloc = CBAlloc(alloc);
            (&deleter)->~Deleter();
            (&alloc)->~Alloc();
            CBAllocTraits::deallocate(tmp_alloc, this, 1);
        }

        ~ControlBlockRegular() override = default;
    };

    template <typename T, typename Alloc = std::allocator<T>>
    struct ControlBlockMakeShared : BaseControlBlock {
        using CBAlloc = typename std::allocator_traits<
                Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
        using CBAllocTraits = std::allocator_traits<CBAlloc>;
        using TAllocTraits = std::allocator_traits<Alloc>;
        T object;
        Alloc alloc;

        ControlBlockMakeShared() : BaseControlBlock(), alloc(Alloc()) {}

        template <typename... Args>
        ControlBlockMakeShared(Args&&... args)
                : BaseControlBlock(),
                  object(std::forward<Args>(args)...),
                  alloc(Alloc()) {}

        template <typename... Args>
        ControlBlockMakeShared(Alloc alloc, Args&&... args)
                : BaseControlBlock(),
                  object(std::forward<Args>(args)...),
                  alloc(alloc) {}

        void useDeleter(void* pointer) override {
            TAllocTraits::destroy(alloc, &object);
        }

        void destroy_cb() override {
            CBAlloc tmp_alloc = CBAlloc(alloc);
            (&alloc)->~Alloc();
            CBAllocTraits::deallocate(tmp_alloc, this, 1);
        }

        ~ControlBlockMakeShared() override = default;
    };
}  // namespace cb

template <typename T>
class EnableSharedFromThis;

template <typename T>
class SharedPtr {

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    template <typename U, typename... Args>
    friend SharedPtr<U> makeShared(Args&&...);

    template <typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc&, Args&&...);

private:
    cb::BaseControlBlock* cb = nullptr;
    T* ptr = nullptr;

    template <typename U>
    SharedPtr(cb::ControlBlockMakeShared<U>* cb) : cb(cb) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            cb->object.wptr = *this;
        }
    }

    template <typename U, typename Alloc = std::allocator<U>>
    SharedPtr(cb::ControlBlockMakeShared<U, Alloc>* cb) : cb(cb), ptr(nullptr) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            cb->object.wptr = *this;
        }
    }

    SharedPtr(cb::BaseControlBlock* cb, T* ptr) : cb(cb), ptr(ptr) {}

public:
    void swap(SharedPtr& other) {
        std::swap(cb, other.cb);
        std::swap(ptr, other.ptr);
    }

    SharedPtr() = default;

    template <typename U>
    SharedPtr(U* pointer) : cb(new cb::ControlBlockRegular<U>()), ptr(pointer) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            ptr->wptr = *this;
        }
    }

    template <typename U, typename Deleter>
    SharedPtr(U* pointer, const Deleter& deleter)
            : cb(new cb::ControlBlockRegular<U, Deleter>(deleter)), ptr(pointer) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            ptr->wptr = *this;
        }
    }

    template <typename U, typename Deleter, typename Alloc>
    SharedPtr(U* pointer, const Deleter& deleter, const Alloc& alloc)
            : ptr(pointer) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            ptr->wptr = *this;
        }
        using CurCB = cb::ControlBlockRegular<U, Deleter, Alloc>;
        using CBAllocator = typename CurCB::CBAlloc;
        CBAllocator cb_alloc = CBAllocator(alloc);
        using CurAllocTraits = std::allocator_traits<CBAllocator>;
        CurCB* cur_cb = CurAllocTraits::allocate(cb_alloc, 1);
        new (cur_cb) CurCB(deleter, alloc);
        cb = cur_cb;
    }

    SharedPtr(const SharedPtr& other) : cb(other.cb), ptr(other.ptr) {
        if (cb != nullptr) {
            ++cb->cnt_shared;
        }
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        SharedPtr copy = SharedPtr(other);
        swap(copy);
        return *this;
    }

    SharedPtr(SharedPtr&& other) : cb(other.cb), ptr(other.ptr) {
        other.cb = nullptr;
        other.ptr = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        SharedPtr copy = SharedPtr(std::move(other));
        swap(copy);
        return *this;
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : cb(other.cb), ptr(other.ptr) {
        if (cb != nullptr) {
            ++cb->cnt_shared;
        }
    }

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        SharedPtr copy = SharedPtr(other);
        swap(copy);
        return *this;
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) : cb(other.cb), ptr(other.ptr) {
        other.cb = nullptr;
        other.ptr = nullptr;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        SharedPtr copy = SharedPtr(std::move(other));
        swap(copy);
        return *this;
    }

    ~SharedPtr() {
        if (cb != nullptr) {
            --cb->cnt_shared;
            if (cb->cnt_shared == 0) {
                cb->useDeleter(ptr);
                if (cb->cnt_weak == 0) {
                    cb->destroy_cb();
                }
            }
        }
    }

    T& operator*() const {
        if (ptr != nullptr) {
            return *ptr;
        } else {
            return static_cast<cb::ControlBlockMakeShared<T>*>(cb)->object;
        }
    }

    T* operator->() const {
        if (ptr != nullptr || cb == nullptr) {
            return ptr;
        } else {
            return &static_cast<cb::ControlBlockMakeShared<T>*>(cb)->object;
        }
    }

    size_t use_count() const {
        return cb != nullptr ? cb->cnt_shared : 0;
    }

    template <typename U>
    void reset(U* pointer) {
        SharedPtr new_ptr(pointer);
        new_ptr.swap(*this);
    }

    template <typename U, typename Deleter>
    void reset(U* pointer, const Deleter& deleter) {
        SharedPtr new_ptr(pointer, deleter);
        new_ptr.swap(*this);
    }

    template <typename U, typename Deleter, typename Alloc>
    void reset(U* pointer, const Deleter& deleter, const Alloc& alloc) {
        SharedPtr new_ptr(pointer, deleter, alloc);
        new_ptr.swap(*this);
    }

    void reset() {
        SharedPtr new_ptr;
        new_ptr.swap(*this);
    }

    T* get() const {
        if (ptr != nullptr || cb == nullptr) {
            return ptr;
        } else {
            return &static_cast<cb::ControlBlockMakeShared<T>*>(cb)->object;
        }
    }
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    using CBAlloc = typename std::allocator<cb::ControlBlockMakeShared<T>>;
    CBAlloc cb_alloc;
    //auto cb = new cb::ControlBlockMakeShared<T>(std::forward<Args>(args)...);
    auto cb = std::allocator_traits<CBAlloc>::allocate(cb_alloc, 1);

    try {
        std::allocator_traits<CBAlloc>::construct(cb_alloc, cb, std::forward<Args>(args)...);
    } catch (...) {
        std::allocator_traits<CBAlloc>::deallocate(cb_alloc, cb, 1);
        throw;
    }
    return SharedPtr<T>(cb);
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    using CBAlloc = typename std::allocator_traits<
            Alloc>::template rebind_alloc<cb::ControlBlockMakeShared<T, Alloc>>;
    CBAlloc cb_alloc = CBAlloc(alloc);
    auto cb = std::allocator_traits<CBAlloc>::allocate(cb_alloc, 1);
//    try {
//        new (&cb->alloc) Alloc(alloc);
//    } catch (...) {
//        std::allocator_traits<CBAlloc>::deallocate(cb_alloc, cb, 1);
//        throw;
//    }
    try {
        std::allocator_traits<CBAlloc>::construct(cb_alloc, cb, alloc, std::forward<Args>(args)...);
    } catch (...) {
        std::allocator_traits<CBAlloc>::deallocate(cb_alloc, cb, 1);
        throw;
    }
    return SharedPtr<T>(cb);
}

template <typename T>
class WeakPtr {
    template <typename U>
    friend class WeakPtr;

private:
    cb::BaseControlBlock* cb = nullptr;
    T* ptr = nullptr;

public:
    void swap(WeakPtr& other) {
        std::swap(cb, other.cb);
    }

    WeakPtr() = default;

    template <typename U>
    WeakPtr(const SharedPtr<U>& shared_ptr)
            : cb(shared_ptr.cb), ptr(shared_ptr.ptr) {
        if (cb != nullptr) {
            ++cb->cnt_weak;
        }
    }

    template <typename U>
    WeakPtr& operator=(const SharedPtr<U>& shared_ptr) {
        WeakPtr copy(shared_ptr);
        swap(copy);
        return *this;
    }

    WeakPtr(const WeakPtr& other) : cb(other.cb), ptr(other.ptr) {
        if (cb != nullptr) {
            ++cb->cnt_weak;
        }
    }

    WeakPtr& operator=(const WeakPtr& other) {
        WeakPtr copy(other);
        swap(copy);
        return *this;
    }

    WeakPtr(WeakPtr&& other) : cb(other.cb), ptr(other.ptr) {
        other.cb = nullptr;
        other.ptr = nullptr;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        WeakPtr copy(std::move(other));
        swap(copy);
        return *this;
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : cb(other.cb), ptr(other.ptr) {
        if (cb != nullptr) {
            ++cb->cnt_weak;
        }
    }

    template <typename U>
    WeakPtr& operator=(const WeakPtr<U>& other) {
        WeakPtr copy(other);
        swap(copy);
        return *this;
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& other) : cb(other.cb), ptr(other.ptr) {
        other.cb = nullptr;
        other.ptr = nullptr;
    }

    template <typename U>
    WeakPtr& operator=(WeakPtr<U>&& other) {
        WeakPtr copy(std::move(other));
        swap(copy);
        return *this;
    }

    ~WeakPtr() {
        if (cb != nullptr) {
            --cb->cnt_weak;
            if (cb->cnt_weak == 0 && cb->cnt_shared == 0) {
                cb->destroy_cb();
            }
        }
    }

    bool expired() const {
        return cb == nullptr || cb->cnt_shared == 0;
    }

    SharedPtr<T> lock() const {
        if (cb != nullptr) {
            ++cb->cnt_shared;
        }
        return SharedPtr<T>(cb, ptr);
    }

    int use_count() const {
        return cb != nullptr ? cb->cnt_shared : 0;
    }
};

template <typename T>
class EnableSharedFromThis {
    template <typename U>
    friend class SharedPtr;

private:
    WeakPtr<T> wptr;

public:
    SharedPtr<T> shared_from_this() const {
        return wptr.lock();
    }

    WeakPtr<T> weak_from_this() const {
        return wptr;
    }
};
