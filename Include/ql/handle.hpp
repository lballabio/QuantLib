
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file handle.hpp
    \brief Reference-counted pointer

    $Id$
*/

// $Source$
// $Log$
// Revision 1.16  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.15  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.14  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//
// Revision 1.13  2001/06/21 14:30:43  lballabio
// Observability is back
//
// Revision 1.12  2001/06/19 15:01:33  lballabio
// Handle can be downcasted to non-const pointer
//
// Revision 1.11  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//
// Revision 1.10  2001/05/31 14:48:10  lballabio
// Worked around Visual C++ deficiencies
//
// Revision 1.9  2001/05/31 13:54:29  lballabio
// Rewritten Handle downcast to be gcc-compatible
//
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

#include "ql/errors.hpp"

// The implementation of this class is taken from
// "The C++ Programming Language", 3rd edition, B.Stroustrup

namespace QuantLib {

    //! Reference-counted pointer
    /*! This class acts as a proxy to a pointer contained in it. Such pointer is
        owned by the handle, i.e., the handle will be responsible for its
        deletion, unless explicitly stated by the programmer.
        
        A count of the references to the contained pointer is incremented every
        time a handle is copied, and decremented every time a handle is deleted
        or goes out of scope. When the handle owns the pointer, this mechanism 
        ensures on one hand, that the pointer will not be deallocated as long as 
        a handle refers to it, and on the other hand, that it will be 
        deallocated when no more handles do.

        \note The implementation of this class was originally taken from
        "The C++ Programming Language", 3rd ed., B.Stroustrup, Addison-Wesley,
        1997.

        \warning This mechanism will break and result in untimely deallocation
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
        
        \warning When the programmer keeps the ownership of the pointer, as
        explicitly declared in
        \code
        SomeObj so;
        Handle<SomeObj> h(&so,false);
        \endcode
        it is responsibility of the programmer to make sure that the object
        remain in scope as long as there are handles pointing to it. Also, the
        programmer must explicitly delete the object if required.
    */
    template <class Type>
    class Handle {
      public:
        //! \name constructors, destructor, and assignment
        //@{
        //! Default constructor returning a null handle.
        Handle()
        : ptr_(0), n_(new int(1)), owns_(true) {}
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
        explicit Handle(Type* ptr, bool owns = true)
        : ptr_(ptr), n_(new int(1)), owns_(owns) {}
        Handle(const Handle& from)
        : ptr_(from.ptr_), n_(from.n_), owns_(from.owns_) { (*n_)++; }
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
        /*! Returns a null pointer in case of failure.
            \warning The returned pointer is not guaranteed to remain 
            allocated and should be discarded immediately after being used */
        #if QL_ALLOW_TEMPLATE_METHOD_CALLS
        template <class Type2>
        Type2* downcast() const {
            return dynamic_cast<Type2*>(ptr_);
        }
        #endif
        //@}
        
        //! \name Inspectors
        //@{
        //! Checks if the contained pointer is actually allocated
        bool isNull() const;
        #if !QL_ALLOW_TEMPLATE_METHOD_CALLS
        /*! \warning This is here only because MSVC won't compile downcast().
            I know it is dangerous. Avoid using it if you can. 
            Blame Microsoft if you can't. */
        Type* pointer() const { return ptr_; }
        #endif
        //@}
      private:
        Type* ptr_;
        int* n_;
        bool owns_;
    };


    // inline definitions

    template <class Type>
    inline Handle<Type>::~Handle() {
        if (--(*n_) == 0) {
            if (ptr_ != 0 && owns_)
                delete ptr_;
            delete n_;
        }
    }

    template <class Type>
    inline Handle<Type>& Handle<Type>::operator=(const Handle& from) {
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
