
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
// Revision 1.2  2001/05/29 09:24:06  lballabio
// Using relinkable handle to term structure
//
// Revision 1.1  2001/05/28 14:54:25  lballabio
// Deposit rates are always adjusted
//
//

#ifndef quantlib_relinkable_handle_h
#define quantlib_relinkable_handle_h

#include "ql/qlerrors.hpp"
#include "ql/handle.hpp"

namespace QuantLib {

    //! Globally accessible relinkable pointer
    /*! This class acts as a proxy to a pointer referenced through a pointer to 
        pointer to Handle . An instance can be relinked to another 
        Handle: such change will be propagated to all the copies of the 
        instance.
    */
    template <class Type>
    class RelinkableHandle {
      public:
        //! Constructor returning an unlinked handle.
        RelinkableHandle()
        : ptr_(new Handle<Type>*(new Handle<Type>)), n_(new int(1)) {}
        RelinkableHandle(const RelinkableHandle& from)
        : ptr_(from.ptr_), n_(from.n_) { (*n_)++; }
        ~RelinkableHandle();

        //! \name Linking
        //@{
        void linkTo(const Handle<Type>& h);
        //@}
        
        //! \name Dereferencing
        //@{
        Type& operator*() const;
        Type* operator->() const;
        //@}

        // \name Inspectors
        //@{
        //! Checks if the contained handle points to anything
        bool isNull() const;
        //! Returns a copy of the contained handle
        Handle<Type> linkedHandle() const;
        //@}

      private:
        Handle<Type>** ptr_;
        int* n_;
        // assignment would join two handle groups - 
        // inhibit it until we found out if we really want this.
        // It doesn't seem a good idea right now.
        RelinkableHandle& operator=(const RelinkableHandle& from) { 
            return *this; }
    };


    // inline definitions

    template <class Type>
    inline RelinkableHandle<Type>::~RelinkableHandle() {
        if (--(*n_) == 0) {
            delete *ptr_;
            delete ptr_;
            delete n_;
        }
    }

    template <class Type>
    inline void RelinkableHandle<Type>::linkTo(const Handle<Type>& h) {
        delete *ptr_;
        *ptr_  = new Handle<Type>(h);
    }

    template <class Type>
    inline Type& RelinkableHandle<Type>::operator*() const {
        QL_REQUIRE(!(*ptr_)->isNull(), "tried to dereference null handle");
        return (*ptr_)->operator*();
    }

    template <class Type>
    inline Type* RelinkableHandle<Type>::operator->() const {
        QL_REQUIRE(!(*ptr_)->isNull(), "tried to dereference null handle");
        return (*ptr_)->operator->();
    }

    template <class Type>
    inline bool RelinkableHandle<Type>::isNull() const {
        return (*ptr_)->isNull();
    }

    template <class Type>
    inline Handle<Type> RelinkableHandle<Type>::linkedHandle() const {
        return **ptr_;
    }

}


#endif
