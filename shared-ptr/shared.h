#pragma once

#include "sw_fwd.h" 

#include <cstddef>

struct ControlBlockBase {
    int shared_refs = 0, weak_refs = 0;

    virtual ~ControlBlockBase() = default;
    virtual void IfNoShared() = 0;
};

template <typename T>
struct ControlBlockPtr : ControlBlockBase {
    T* ptr_;

    ControlBlockPtr(T* ptr) : ptr_(ptr) {
    }

    void IfNoShared() override {
        delete ptr_;
        ptr_ = nullptr;
    }

    ~ControlBlockPtr() override = default;
};

template <typename T>
struct ControlBlockHolder : ControlBlockBase {
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;

    template <typename... Args>
    ControlBlockHolder(Args&&... args) {
        new (&storage_) T{std::forward<Args>(args)...};
    }

    void IfNoShared() override {
        reinterpret_cast<T*>(&storage_)->~T();
    }

    ~ControlBlockHolder() override = default;

    T* Get() {
        return reinterpret_cast<T*>(&storage_);
    }
};

template <typename T>
class SharedPtr {
private:
    T* ptr_;
    ControlBlockBase* block_;

    template <typename Y, typename... Args>
    friend SharedPtr<Y> MakeShared(Args&&... args);

    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

public:
    // Constructors
    ///////////////////////////////////////////////////////////////////////

    SharedPtr() noexcept : ptr_(nullptr), block_(nullptr) {
    }
    SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), block_(nullptr) {
    }
    template <typename Y>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), block_(new ControlBlockPtr<Y>(ptr)) {
        block_->shared_refs = 1;
    }
    template <typename Y>
    explicit SharedPtr(Y* ptr, ControlBlockBase* block) : ptr_(ptr), block_(block) {
        if (block_) {
            ++(block_->shared_refs);
        }
    }
    SharedPtr(const SharedPtr& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++(block_->shared_refs);
        }
    }
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++(block_->shared_refs);
        }
    }
    SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) noexcept : ptr_(ptr), block_(other.block_) {
        if (block_) {
            ++(block_->shared_refs);
        }
    }

    // Promote `WeakPtr`
    ///////////////////////////////////////////////////////////////////////
 
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr{};
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            ++(block_->shared_refs);
        }
    }
    template <typename Y>
    explicit SharedPtr(const WeakPtr<Y>& other) {
        if (other.Expired()) {
            throw BadWeakPtr{};
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            ++(block_->shared_refs);
        }
    }

    // `operator=`-s
    ///////////////////////////////////////////////////////////////////////

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        SharedPtr<T>(other).Swap(*this);
        return *this;
    }
    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) noexcept {
        SharedPtr<T>(other).Swap(*this);
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        SharedPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }
    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) noexcept {
        SharedPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    // Destructor
    ///////////////////////////////////////////////////////////////////////

    ~SharedPtr() {
        if (block_) {
            --block_->shared_refs;
            if (block_->shared_refs == 0) {
                block_->IfNoShared();
            }
            if (block_->shared_refs + block_->weak_refs == 0) {
                delete block_;
            }
        }
    }

    // Modifiers
    ///////////////////////////////////////////////////////////////////////

    void Reset() noexcept {
        SharedPtr().Swap(*this);
    }
    template <typename Y>
    void Reset(Y* ptr) {
        SharedPtr<T>(ptr).Swap(*this);
    }
    void Swap(SharedPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    // Observers
    ///////////////////////////////////////////////////////////////////////

    T* Get() const noexcept {
        return ptr_;
    }
    T& operator*() const noexcept {
        return *ptr_;
    }
    T* operator->() const noexcept {
        return ptr_;
    }
    size_t UseCount() const noexcept {
        if (block_) {
            return block_->shared_refs;
        }
        return 0;
    }
    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto holder_block = new ControlBlockHolder<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(holder_block->Get(), holder_block);
}

template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};

