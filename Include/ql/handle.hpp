
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

/*! \file handle.hpp
    \brief Reference-counted pointer

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/05/28 13:21:45  lballabio
// Trying to get g++ to compile
//
// Revision 1.7  2001/05/28 12:52:58  lballabio
// Simplified Instrument interface
//
// Revision 1.6  2001/05/24 17:50:23  nando
// no message
//
// Revision 1.5  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_handle_h
#define quantlib_handle_h

#include "ql/qlerrors.hpp"

// The implementation of this class is taken from
// "The C++ Programming Language", 3rd edition, B.Stroustrup

namespace QuantLib {

    //! Reference-counted pointer
    /*! This class acts as a proxy to a pointer contained in it. Such pointer is
        owned by the handle, i.e., the handle will be responsible for its
        deletion.
        
        A count of the references to the contained pointer is incremented every
        time a handle is copied, and decremented every time a handle is deleted
        or goes out of scope. This mechanism ensures on one hand, that the
        pointer will not be deallocated as long as a handle refers to it, and on
        the other hand, that it will be deallocated when no more handles do.

        \note The implementation of this class was originally taken from
        "The C++ Programming Language", 3rd ed., B.Stroustrup, Addison-Wesley,
        1997.

        \warning This mechanism will broke and result in untimely deallocation
        of the pointer (and very possible death of your executable) if two
        handles are explicitly initialized with the same pointer, as in
        \code
        SomeObj* so = new SomeObj;
        Handle<SomeObj> h1(so);
        Handle<SomeObj> h2 = h1;    // this is safe.
        Handle<SomeObj> h3(so);     // this is definitely not.
        \endcode
        It is good practice to create the pointer and immediately pass it to the
        handle, as in
        \code
        Handle<SomeObj> h1(new SomeObj);    // this is as safe as can be.
        \endcode
    */
    template <class Type>
    class Handle {
      public:
        //! \name constructors, destructor, and assignment
        //@{
        //! Default constructor returning a null handle.
        Handle()
        : ptr_(0), n_(new int(1)) {}
        //! Constructor taking a pointer.
        /*! The handle will be responsible for its deletion.
        */
        explicit Handle(Type* ptr)
        : ptr_(ptr), n_(new int(1)) {}
        Handle(const Handle& from)
        : ptr_(from.ptr_), n_(from.n_) { (*n_)++; }
        ~Handle();
        Handle& operator=(const Handle& from);
        //@}

        //! \name Dereferencing
        //@{
        Type& operator*() const;
        Type* operator->() const;
        //@}

        //! \name Casting
        //@{
        template <class Type2>
        Handle<Type2> downcast() const {
            Handle<Type2> to;
            HandleConverter().cast(*this,to);
            return to;
        }
        //@}
        
        //! \name Inspectors
        //@{
        //! Checks if the contained pointer is actually allocated
        bool isNull() const;
        //@}

      private:
        Type* ptr_;
        int* n_;
        // used to convert handles to different but compatible types
        class HandleConverter;
        friend class HandleConverter;
        class HandleConverter {
          public:
            template <class ToType, class FromType> 
            void cast(Handle<FromType> from, Handle<ToType>& to) const {
                if (--(*to.n_) == 0) {
                    if (to.ptr_ != 0)
                        delete to.ptr_;
                    delete to.n_;
                }
                to.ptr_ = dynamic_cast<ToType*>(from.ptr_);
                if (to.ptr_ == 0) {
                    to.n_ = new int(1);
                } else {
                    to.n_    = from.n_;
                    (*(to.n_))++;
                }
            }
        };
    };


    // inline definitions

    template <class Type>
    inline Handle<Type>::~Handle() {
        if (--(*n_) == 0) {
            if (ptr_ != 0)
                delete ptr_;
            delete n_;
        }
    }

    template <class Type>
    inline Handle<Type>& Handle<Type>::operator=(const Handle& from) {
        if (ptr_ != from.ptr_) {
            if (--(*n_) == 0) {
                if (ptr_ != 0)
                    delete ptr_;
                delete n_;
            }
            ptr_  = from.ptr_;
            n_    = from.n_;
            (*n_)++;
        }
        return *this;
    }

    template <class Type>
    inline Type& Handle<Type>::operator*() const {
        QL_REQUIRE(ptr_ != 0, "tried to dereference null handle");
        return *ptr_;
    }

    template <class Type>
    inline Type* Handle<Type>::operator->() const {
        QL_REQUIRE(ptr_ != 0, "tried to dereference null handle");
        return ptr_;
    }

    template <class Type>
    inline bool Handle<Type>::isNull() const {
        return (ptr_ == 0);
    }

}


#endif
