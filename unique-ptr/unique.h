#pragma once

#include "compressed_pair.h"

#include <cstddef>
#include <type_traits>

template <typename T>
struct Slug {
    void operator()(T* ptr) noexcept {
        delete ptr;
    }
};

template <typename T>
struct Slug<T[]> {
    void operator()(T* ptr) noexcept {
        delete[] ptr;
    }
};

template <typename T, typename Deleter = Slug<T> >
class UniquePtr {
private:
    CompressedPair<T*, Deleter> uptr_;

public:
    // Constructors
    ///////////////////////////////////////////////////////////////////////

    UniquePtr() noexcept : uptr_(nullptr, Deleter()) {
    }
    UniquePtr(std::nullptr_t) noexcept : uptr_(nullptr, Deleter()) {
    }
    explicit UniquePtr(T* ptr) noexcept : uptr_(ptr, Deleter()) {
    }

    UniquePtr(const UniquePtr&) = delete;
    template <typename Del>
    UniquePtr(T* ptr, Del&& deleter) noexcept : uptr_(ptr, std::forward<Del>(deleter)) {
    }

    template <typename U, typename Del>
    UniquePtr(UniquePtr<U, Del>&& other) noexcept
        : uptr_(other.Release(), std::forward<Del>(other.GetDeleter())) {
    }

    // `operator=`-s
    ///////////////////////////////////////////////////////////////////////

    UniquePtr& operator=(const UniquePtr&) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        this->Reset(other.Release());
        GetDeleter() = std::forward<Deleter>(other.GetDeleter());
        return *this;
    }
    template <typename U, typename Del>
    UniquePtr& operator=(UniquePtr<U, Del>&& other) noexcept {
        this->Reset(other.Release());
        GetDeleter() = std::forward<Del>(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    // Destructor
    ///////////////////////////////////////////////////////////////////////

    ~UniquePtr() noexcept {
        if (uptr_.GetFirst()) {
            GetDeleter()(Get());
        }
    }

    // Modifiers
    ///////////////////////////////////////////////////////////////////////

    T* Release() noexcept {
        T* temp = uptr_.GetFirst();
        uptr_.GetFirst() = nullptr;
        return temp;
    }
    void Reset(T* ptr = nullptr) noexcept {
        T* old_ptr = uptr_.GetFirst();
        uptr_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Reset(std::nullptr_t) noexcept {
        this->Reset();
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(uptr_.GetFirst(), other.uptr_.GetFirst());
        std::swap(uptr_.GetSecond(), other.uptr_.GetSecond());
    }

    // Observers
    ///////////////////////////////////////////////////////////////////////

    T* Get() const noexcept {
        return uptr_.GetFirst();
    }
    Deleter& GetDeleter() noexcept {
        return uptr_.GetSecond();
    }
    const Deleter& GetDeleter() const noexcept {
        return uptr_.GetSecond();
    }
    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    // Single-object dereference operators
    ///////////////////////////////////////////////////////////////////////

    typename std::add_lvalue_reference_t<T> operator*() const noexcept {
        return *(uptr_.GetFirst());
    }
    T* operator->() const noexcept {
        return uptr_.GetFirst();
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    CompressedPair<T*, Deleter> uptr_;

public:
    // Constructors
    ///////////////////////////////////////////////////////////////////////

    UniquePtr() noexcept : uptr_(nullptr, Deleter()) {
    }
    UniquePtr(std::nullptr_t) noexcept : uptr_(nullptr, Deleter()) {
    }
    UniquePtr(const UniquePtr&) = delete;
    template <typename U>
    explicit UniquePtr(U ptr) noexcept : uptr_(ptr, Deleter()) {
    }
    template <typename U, typename Del>
    UniquePtr(U ptr, Del&& deleter) noexcept : uptr_(ptr, std::forward<Del>(deleter)) {
    }
    template <typename U, typename Del>
    UniquePtr(UniquePtr<U, Del>&& other) noexcept
        : uptr_(other.Release(), std::forward<Del>(other.GetDeleter())) {
    }

    // `operator=`-s
    ///////////////////////////////////////////////////////////////////////

    UniquePtr& operator=(const UniquePtr&) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        this->Reset(other.Release());
        GetDeleter() = std::forward<Deleter>(other.GetDeleter());
        return *this;
    }
    template <typename U, typename Del>
    UniquePtr& operator=(UniquePtr<U, Del>&& other) noexcept {
        this->Reset(other.Release());
        GetDeleter() = std::forward<Del>(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    // Destructor
    ///////////////////////////////////////////////////////////////////////

    ~UniquePtr() noexcept {
        if (uptr_.GetFirst()) {
            GetDeleter()(Get());
        }
    }

    // Modifiers
    ///////////////////////////////////////////////////////////////////////

    T* Release() noexcept {
        T* temp = uptr_.GetFirst();
        uptr_.GetFirst() = nullptr;
        return temp;
    }
    void Reset(T* ptr = nullptr) noexcept {
        T* old_ptr = uptr_.GetFirst();
        uptr_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Reset(std::nullptr_t) noexcept {
        this->Reset();
    }
    void Swap(UniquePtr& other) noexcept {
        std::swap(uptr_.GetFirst(), other.uptr_.GetFirst());
        std::swap(uptr_.GetSecond(), other.uptr_.GetSecond());
    }

    // Observers
    ///////////////////////////////////////////////////////////////////////

    T* Get() const noexcept {
        return uptr_.GetFirst();
    }
    Deleter& GetDeleter() noexcept {
        return uptr_.GetSecond();
    }
    T& operator[](std::size_t i) {
        return *(uptr_.GetFirst() + i);
    }
    const T& operator[](std::size_t i) const {
        return *(uptr_.GetFirst() + i);
    }
    const Deleter& GetDeleter() const noexcept {
        return uptr_.GetSecond();
    }
    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    // Single-object dereference operators
    ///////////////////////////////////////////////////////////////////////

    typename std::add_lvalue_reference_t<T> operator*() const noexcept {
        return *(uptr_.GetFirst());
    }
    T* operator->() const noexcept {
        return uptr_.GetFirst();
    }
};

