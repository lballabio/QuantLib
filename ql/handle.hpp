
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file handle.hpp
    \brief Reference-counted pointer
*/

#ifndef quantlib_handle_h
#define quantlib_handle_h

#include <ql/errors.hpp>
#include <typeinfo>

#if defined(HAVE_BOOST)

#include <boost/shared_ptr.hpp>

#define Handle boost::shared_ptr

namespace QuantLib {

    template <class T>
    bool IsNull(const boost::shared_ptr<T>&);

    template <class T>
    inline bool IsNull(const boost::shared_ptr<T>& p) {
        return !p;
    }

}

#else


namespace QuantLib {

    // The implementation of this class was originally taken from
    // B. Stroustrup, "The C++ Programming Language", 3rd edition.

    template <class T> class Handle;

    class HandleCopier {
      public:
        template <class T, class U>
        static void copy(const Handle<T>& from, const Handle<U>& to) {
            if (from.n_ != to.n_) {
                // cast to the new type - the resulting pointer will
                // be null if the two types are not compatible
                U* u  = dynamic_cast<U*>(from.ptr_);
                QL_REQUIRE(u != 0,
                           "A handle to " +
                           std::string(typeid(T).name()) +
                           " cannot be converted to a handle to " +
                           std::string(typeid(U).name()));
                // if 'to' was the last reference to its object...
                if (--(*to.n_) == 0) {
                    // ...delete the latter and the counter
                    if (to.ptr_ != 0 && to.owns_)
                        delete to.ptr_;
                    delete to.n_;
                }
                to.ptr_  = u;
                to.n_    = from.n_;
                to.owns_ = from.owns_;
                (*to.n_)++;
            }
        }
    };

    //! Reference-counted pointer
    /*! This class acts as a proxy to a pointer contained in it. Such pointer 
        is owned by the handle, i.e., the handle will be responsible for its
        deletion, unless explicitly stated by the programmer.

        A count of the references to the contained pointer is incremented 
        every time a handle is copied, and decremented every time a handle is 
        deleted or goes out of scope. When the handle owns the pointer, this 
        mechanism ensures on one hand, that the pointer will not be 
        deallocated as long as a handle refers to it, and on the other hand, 
        that it will be deallocated when no more handles do.

        \note The implementation of this class was originally taken from
              "The C++ Programming Language", 3rd ed., B.Stroustrup, 
              Addison-Wesley,
              1997.

        \warning This mechanism will break and result in untimely 
                 deallocation of the pointer (and very possible death of your 
                 executable) if two handles are explicitly initialized with 
                 the same pointer, as in
        \code
        SomeObj* so = new SomeObj;
        Handle<SomeObj> h1(so);
        Handle<SomeObj> h2 = h1;    // this is safe.
        Handle<SomeObj> h3(so);     // this is definitely not.
        \endcode
                 It is good practice to create the pointer and immediately 
                 pass it to the handle, as in
        \code
        Handle<SomeObj> h1(new SomeObj);    // this is as safe as can be.
        \endcode

        \warning When the programmer keeps the ownership of the pointer, as
                 explicitly declared in
        \code
        SomeObj so;
        Handle<SomeObj> h(&so,false);
        \endcode
                 it is responsibility of the programmer to make sure that the 
                 object remain in scope as long as there are handles pointing 
                 to it. Also, the programmer must explicitly delete the 
                 object if required.
    */
    template <class T>
    class Handle {
        friend class HandleCopier;
      public:
        //! \name constructors, destructor, and assignment
        //@{
        //! Constructor taking a pointer.
        /*! If <b>owns</b> is set to <tt>true</tt> (the default), the handle
            will be responsible for the deletion of the pointer. If it is
            set to <tt>false</tt>, the programmer must make sure that the
            pointed object remains in scope for the lifetime of the handle
            and its copies. Destruction of the object is also responsibility
            of the programmer.

            It is advised that handles be used with <tt>owns = false</tt>
            only in a controlled an self-contained environment. Such a case
            happens when an object needs to pass a handle to itself to inner
            classes or bootstrappers - i.e., contained or temporary objects
            whose lifetime is guaranteed not to last more than the lifetime
            of the object.
        */
        explicit Handle(T* ptr = 0, bool owns = true)
        : ptr_(ptr), n_(new int(1)), owns_(owns) {}
        template <class U>
        Handle(const Handle<U>& from)
        : ptr_(0), n_(new int(1)), owns_(true) {
            HandleCopier::copy(from,*this);
        }
        Handle(const Handle& from)
        : ptr_(from.ptr_), n_(from.n_), owns_(from.owns_) { (*n_)++; }
        ~Handle();
        template <class U>
        Handle& operator=(const Handle<U>& from) {
            HandleCopier::copy(from,*this);
            return *this;
        }
        Handle& operator=(const Handle& from);
        //@}

        //! \name Dereferencing
        //@{
        T& operator*() const;
        T* operator->() const;
        //@}

        //! \name Inspectors
        //@{
        //! Checks if the contained pointer is actually allocated
        bool isNull() const;
        //! Checks if the two handles point to the same object
        bool operator==(const Handle<T>&) const;
        bool operator!=(const Handle<T>&) const;
        //@}
      private:
        mutable T* ptr_;
        mutable int* n_;
        mutable bool owns_;
    };

    template <class T>
    bool IsNull(const Handle<T>&);


    // inline definitions

    template <class T>
    inline Handle<T>::~Handle() {
        if (--(*n_) == 0) {
            if (ptr_ != 0 && owns_)
                delete ptr_;
            delete n_;
        }
    }

    template <class T>
    inline Handle<T>& Handle<T>::operator=(const Handle& from) {
        if (ptr_ != from.ptr_) {
            if (--(*n_) == 0) {
                if (ptr_ != 0 && owns_)
                    delete ptr_;
                delete n_;
            }
            ptr_  = from.ptr_;
            n_    = from.n_;
            owns_ = from.owns_;
            (*n_)++;
        }
        return *this;
    }

    template <class T>
    inline T& Handle<T>::operator*() const {
        QL_REQUIRE(ptr_ != 0, 
                   "tried to dereference null handle to "
                   + std::string(typeid(T).name()));
        return *ptr_;
    }

    template <class T>
    inline T* Handle<T>::operator->() const {
        QL_REQUIRE(ptr_ != 0, 
                   "tried to dereference null handle to "
                   + std::string(typeid(T).name()));
        return ptr_;
    }

    template <class T>
    inline bool Handle<T>::isNull() const {
        return (ptr_ == 0);
    }

    template <class T>
    inline bool Handle<T>::operator==(const Handle<T>& h) const {
        return (n_ == h.n_);
    }

    template <class T>
    inline bool Handle<T>::operator!=(const Handle<T>& h) const {
        return (n_ != h.n_);
    }

    template <class T>
    bool IsNull(const Handle<T>& h) {
        return h.isNull();
    }

}


#endif

#endif
