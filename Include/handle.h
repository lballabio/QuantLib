
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file handle.h
    \brief Reference-counted pointer

    $Source$
    $Log$
    Revision 1.9  2001/03/12 17:35:09  lballabio
    Removed global IsNull function - could have caused very vicious loops

    Revision 1.8  2001/03/07 15:27:50  lballabio
    Modified Handle to allow keeping ownership

    Revision 1.7  2001/02/09 19:16:21  lballabio
    removed QL_PTR_CONST macro

    Revision 1.6  2001/01/17 14:37:54  nando
    tabs removed

    Revision 1.5  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_handle_h
#define quantlib_handle_h

#include "qldefines.h"
#include "qlerrors.h"

// The implementation of this class is taken from
// "The C++ Programming Language", 3rd edition, B.Stroustrup

namespace QuantLib {

    //! Reference-counted pointer
    /*! This class acts as a proxy to a pointer contained in it. Such pointer is
        owned by the handle, i.e., the handle will be responsible for its 
        deletion, unless explicitly stated by the programmer.
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
        friend bool operator==(const Handle<Type>&, const Handle<Type>&);
        friend bool operator!=(const Handle<Type>&, const Handle<Type>&);
      public:
        //! \name constructors, destructor, and assignment
        //@{
        //! Default constructor returning a null handle.
        Handle()
        : ptr_(0), n_(new int(1)), owns_(true) {}
        //! Constructor taking a pointer. 
        /*! If <b>owns</b> is set to <tt>true</tt> (the default), the handle 
            will be responsible for the deletion of the pointer. If it is set to 
            <tt>false</tt>, the programmer must make sure that the pointed 
            object remains in scope for the lifetime of the handle and its 
            copies. Destruction of the object is also responsibility of the 
            programmer.
        */
        explicit Handle(Type* ptr, bool owns=true)
        : ptr_(ptr), n_(new int(1)), owns_(owns) {}
        //! Copy from a handle to a different but compatible type
        template <class Type2> explicit Handle(const Handle<Type2>& from)
        : ptr_(0), n_(new int(1)) { HandleCopier().copy(*this,from); }
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

        // \name Inspectors
        //@{
        //! Checks if the contained pointer is actually allocated
        bool isNull() const;
        //@}
        
      private:
        Type* ptr_;
        int* n_;
        bool owns_;
        // used to convert handles to different but compatible types
        class HandleCopier;
        friend class HandleCopier;
        class HandleCopier {
          public:
            HandleCopier() {}
            template <class Type1, class Type2> void copy(Handle<Type1>& to, 
              Handle<Type2> from) const {
                if (to.ptr_ != from.ptr_) {
                    if (--(*(to.n_)) == 0) {
                        if (to.ptr_ != 0 && to.owns_)
                            delete to.ptr_;
                        delete to.n_;
                    }
                    to.ptr_  = from.ptr_;
                    to.n_    = from.n_;
                    to.owns_ = from.owns_;
                    (*(to.n_))++;
                }
            }
        };
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

    //! Returns <tt>true</tt> iff the two handles contain the same pointer
    /*! \relates Handle */
    template <class Type>
    inline bool operator==(const Handle<Type>& h1, const Handle<Type>& h2) {
        return (h1.ptr_ == h2.ptr_);
    }

    /*! \relates Handle */
    template <class Type>
    inline bool operator!=(const Handle<Type>& h1, const Handle<Type>& h2) {
        return (h1.ptr_ != h2.ptr_);
    }

}


#endif
