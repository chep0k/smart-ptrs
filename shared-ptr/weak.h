#pragma once

#include "sw_fwd.h"
#include "shared.h"

template <typename T>
class WeakPtr {
private:
    T* ptr_;
    ControlBlockBase* block_;

    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

public:
    // Constructors
    ///////////////////////////////////////////////////////////////////////

    WeakPtr() noexcept : ptr_(nullptr), block_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->weak_refs;
        }
    }
    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++(block_->weak_refs);
        }
    }
    WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }
    template <typename Y>
    WeakPtr(WeakPtr<Y>&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++(block_->weak_refs);
        }
    }

    // `operator=`-s
    ///////////////////////////////////////////////////////////////////////

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }
    template <typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) noexcept {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }
    template <typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) noexcept {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }
    template <typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& sptr) noexcept {
        WeakPtr<T>(sptr).Swap(*this);
        return *this;
    }

    // Destructor
    ///////////////////////////////////////////////////////////////////////

    ~WeakPtr() {
        if (block_) {
            --block_->weak_refs;
            if (block_->shared_refs + block_->weak_refs == 0) {
                delete block_;
            }
        }
    }

    // Modifiers
    ///////////////////////////////////////////////////////////////////////

    void Reset() {
        WeakPtr<T>().Swap(*this);
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    // Observers
    ///////////////////////////////////////////////////////////////////////

    size_t UseCount() const noexcept {
        if (block_) {
            return block_->shared_refs;
        }
        return 0;
    }
    bool Expired() const noexcept {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const noexcept {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }
};

