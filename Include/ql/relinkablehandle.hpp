
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file relinkablehandle.hpp
    \brief Globally accessible relinkable pointer

    $Id$
*/

// $Source$
// $Log$
// Revision 1.1  2001/05/28 14:54:25  lballabio
// Deposit rates are always adjusted
//
//

#ifndef quantlib_relinkable_handle_h
#define quantlib_relinkable_handle_h

#include "ql/qlerrors.hpp"

namespace QuantLib {

    //! Globally accessible relinkable pointer pointer
    /*! This class acts as a proxy to a pointer referenced through a pointer to 
        pointer. Such pointer is owned by the handle, i.e., the handle will be 
        responsible for its deletion, unless explicitly stated by the 
        programmer.
        
        A handle can be relinked to another pointer. Such change will be 
        propagated to all the copies of the handle.

        A count of the references to the contained pointer is incremented every
        time a handle is copied, and decremented every time a handle is deleted
        or goes out of scope. This mechanism ensures on one hand, that the
        pointer will not be deallocated as long as a handle refers to it, and on
        the other hand, that it will be deallocated when no more handles do.
        
        \note Part of the implementation of this class was originally taken from
        "The C++ Programming Language", 3rd ed., B.Stroustrup, Addison-Wesley,
        1997.

        \warning This mechanism will broke and result in untimely deallocation
        of the pointer (and very possible death of your executable) if two
        handles are explicitly initialized with the same pointer, as in
        \code
        SomeObj* so = new SomeObj;
        RelinkableHandle<SomeObj> h1(so);
        RelinkableHandle<SomeObj> h2 = h1;    // this is safe.
        RelinkableHandle<SomeObj> h3(so);     // this is definitely not.
        \endcode
        It is good practice to create the pointer and immediately pass it to the
        handle, as in
        \code
        RelinkableHandle<SomeObj> h1(new SomeObj);    // this is safe.
        \endcode

        \warning When the programmer keeps the ownership of the pointer, as
        explicitly declared in
        \code
        SomeObj so;
        RelinkableHandle<SomeObj> h(&so,false);
        \endcode
        it is responsibility of the programmer to make sure that the object
        remain in scope as long as there are handles pointing to it. Also, the
        programmer must explicitly delete the object if required.
    */
    template <class Type>
    class RelinkableHandle {
      public:
        //! Default constructor returning a null handle.
        RelinkableHandle()
        : ptr_(new Type*(0)), n_(new int(1)), owns_(new bool(true)) {}
        //! Constructor taking a pointer.
        /*! If <b>owns</b> is set to <tt>true</tt> (the default), the handle
            will be responsible for the deletion of the pointer. If it is set to
            <tt>false</tt>, the programmer must make sure that the pointed
            object remains in scope for the lifetime of the handle and its
            copies. Destruction of the object is also responsibility of the
            programmer.
        */
        explicit RelinkableHandle(Type* ptr, bool owns=true)
        : ptr_(new Type*(ptr)), n_(new int(1)), owns_(new bool(owns)) {}
        RelinkableHandle(const RelinkableHandle& from)
        : ptr_(from.ptr_), n_(from.n_), owns_(from.owns_) { (*n_)++; }
        ~RelinkableHandle();

        //! \name Linking
        //@{
        void linkTo(Type* ptr, bool owns=true);
        //@}
        
        //! \name Dereferencing
        //@{
        Type& operator*() const;
        Type* operator->() const;
        //@}

        // \name Inspectors
        //@{
        //! Checks if the contained pointer is actually allocated
        bool isNull() const;
        //@}

      private:
        Type** ptr_;
        int* n_;
        bool* owns_;
        // inhibited
        RelinkableHandle& operator=(const RelinkableHandle& from);
    };


    // inline definitions

    template <class Type>
    inline RelinkableHandle<Type>::~RelinkableHandle() {
        if (--(*n_) == 0) {
            if (*ptr_ != 0 && *owns_)
                delete *ptr_;
            delete ptr_;
            delete n_;
            delete owns_;
        }
    }

    template <class Type>
    inline RelinkableHandle<Type>& 
    RelinkableHandle<Type>::operator=(const RelinkableHandle&) {
        // inhibited
        return *this;
    }

    template <class Type>
    inline void RelinkableHandle<Type>::linkTo(Type* ptr, bool owns) {
        if (ptr != *from.ptr_) {
            if (*ptr_ != 0 && *owns_)
                delete *ptr_;
            *ptr_  = ptr;
            *n_    = 1;
            *owns_ = owns;
        }
    }

    template <class Type>
    inline Type& RelinkableHandle<Type>::operator*() const {
        QL_REQUIRE(*ptr_ != 0, "tried to dereference null handle");
        return **ptr_;
    }

    template <class Type>
    inline Type* RelinkableHandle<Type>::operator->() const {
        QL_REQUIRE(*ptr_ != 0, "tried to dereference null handle");
        return *ptr_;
    }

    template <class Type>
    inline bool RelinkableHandle<Type>::isNull() const {
        return (*ptr_ == 0);
    }

}


#endif
