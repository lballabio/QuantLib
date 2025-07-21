/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file clone.hpp
    \brief cloning proxy to an underlying object
*/

#ifndef quantlib_clone_hpp
#define quantlib_clone_hpp

#include <ql/errors.hpp>
#include <algorithm>
#include <memory>

namespace QuantLib {

    //! cloning proxy to an underlying object
    /*! When copied, this class will make a clone of its underlying
        object, which must provide a <tt>clone()</tt> method returning
        a std::auto_ptr (or a std::unique_ptr, depending on your
        configuration) to a newly-allocated instance.
    */
    template <class T>
    class Clone {
      public:
        Clone() = default;
        Clone(std::unique_ptr<T>&&);
        Clone(const T&);
        Clone(const Clone<T>&);
        Clone(Clone<T>&&) noexcept;
        Clone<T>& operator=(const T&);
        Clone<T>& operator=(const Clone<T>&);
        Clone<T>& operator=(Clone<T>&&) noexcept;
        T& operator*() const;
        T* operator->() const;
        bool empty() const;
        void swap(Clone<T>& t) noexcept;
        ~Clone() = default;
      private:
        std::unique_ptr<T> ptr_;
    };

    /*! \relates Clone */
    template <class T>
    void swap(Clone<T>&, Clone<T>&) noexcept;


    // inline definitions

    template <class T>
    inline Clone<T>::Clone(std::unique_ptr<T>&& p)
    : ptr_(std::move(p)) {}

    template <class T>
    inline Clone<T>::Clone(const T& t)
    : ptr_(t.clone().release()) {}

    template <class T>
    inline Clone<T>::Clone(const Clone<T>& t)
    : ptr_(t.empty() ? (T*)nullptr : t->clone().release()) {}

    template <class T>
    inline Clone<T>::Clone(Clone<T>&& t) noexcept {
        swap(t);
    }

    template <class T>
    inline Clone<T>& Clone<T>::operator=(const T& t) {
        ptr_ = t.clone();
        return *this;
    }

    template <class T>
    inline Clone<T>& Clone<T>::operator=(const Clone<T>& t) {
        ptr_.reset(t.empty() ? (T*)nullptr : t->clone().release());
        return *this;
    }

    template <class T>
    inline Clone<T>& Clone<T>::operator=(Clone<T>&& t) noexcept {
        swap(t);
        return *this;
    }

    template <class T>
    inline T& Clone<T>::operator*() const {
        QL_REQUIRE(!this->empty(), "no underlying objects");
        return *(this->ptr_);
    }

    template <class T>
    inline T* Clone<T>::operator->() const {
        return this->ptr_.get();
    }

    template <class T>
    inline bool Clone<T>::empty() const {
        return !ptr_;
    }

    template <class T>
    inline void Clone<T>::swap(Clone<T>& t) noexcept {
        this->ptr_.swap(t.ptr_);
    }

    template <class T>
    inline void swap(Clone<T>& t, Clone<T>& u) noexcept {
        t.swap(u);
    }

}


#endif
