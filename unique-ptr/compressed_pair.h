#pragma once

#include <cstddef>
#include <utility>

template <typename T, size_t I, bool = std::is_empty<T>::value && !std::is_final<T>::value >
class CompressedPairElement {
private:
    T elem;

public:
    // Constructors
    ///////////////////////////////////////////////////////////////////////

    CompressedPairElement() : elem(T()) {
    }
    template <typename Type>
    CompressedPairElement(Type&& other_elem) : elem(std::forward<Type>(other_elem)) {
    }

    // Observers
    ///////////////////////////////////////////////////////////////////////

    T& Get() {
        return elem;
    }
    const T& Get() const {
        return elem;
    }
};

template <typename T, size_t I>
class CompressedPairElement<T, I, true> : public T {
    // Constructors
    ///////////////////////////////////////////////////////////////////////
    CompressedPairElement(){};
    template <typename Tt>
    CompressedPairElement(Tt&&) {
    }

    // Observers
    ///////////////////////////////////////////////////////////////////////
    T& Get() {
        return *this;
    }
    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : CompressedPairElement<F, 0>, CompressedPairElement<S, 1> {
public:
    CompressedPair() : CompressedPairElement<F, 0>(), CompressedPairElement<S, 1>() {
    }

    template <typename Ff, typename Ss>
    CompressedPair(Ff&& first, Ss&& second)
        : CompressedPairElement<F, 0>(std::forward<Ff>(first)),
          CompressedPairElement<S, 1>(std::forward<Ss>(second)) {
    }

    F& GetFirst() {
        return CompressedPairElement<F, 0>::Get();
    }

    const F& GetFirst() const {
        return CompressedPairElement<F, 0>::Get();
    }

    S& GetSecond() {
        return CompressedPairElement<S, 1>::Get();
    }

    const S& GetSecond() const {
        return CompressedPairElement<S, 1>::Get();
    }
};

