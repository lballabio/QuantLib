
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
// Revision 1.4  2001/06/21 14:30:43  lballabio
// Observability is back
//
// Revision 1.3  2001/06/20 11:52:29  lballabio
// Some observability is back
//
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
#include "ql/Patterns/observable.hpp"

namespace QuantLib {

    //! Globally accessible relinkable pointer
    /*! This class acts as a proxy to a pointer referenced through a pointer to 
        pointer to Handle . An instance can be relinked to another 
        Handle: such change will be propagated to all the copies of the 
        instance.
        \pre Class "Type" must inherit from Observable
    */
    template <class Type>
    class RelinkableHandle {
      public:
        //! Constructor returning an unlinked handle.
        RelinkableHandle();
        //! Copy constructor
        RelinkableHandle(const RelinkableHandle& from);
        ~RelinkableHandle();

        //! \name Linking
        //@{
        /*! \warning <i><b>registerAsObserver</b></i> is left as a backdoor
            in case the programmer cannot guarantee that the object pointed 
            to will remain alive for the whole lifetime of the handle - 
            namely, it should be set to <tt>false</tt> when the passed handle 
            was created with <tt>owns = false</tt> (the latter should only 
            happen in a controlled environment, so that the programmer is 
            aware of it). Failure to do so can very likely result 
            in a program crash. 
            
            If the programmer does want the relinkable handle to register as 
            observer of such a handle, it is his responsibility to ensure 
            that the relinkable handle gets destroyed before the pointed 
            object does.
        */
        void linkTo(const Handle<Type>& h, bool registerAsObserver = true);
        //@}
        
        //! \name Dereferencing
        //@{
        Type& operator*() const;
        Type* operator->() const;
        //@}

        //! \name Inspectors
        //@{
        //! Checks if the contained handle points to anything
        bool isNull() const;
        //! Returns a copy of the contained handle
        Handle<Type> linkedHandle() const;
        //@}

        /*! \name Observable interface
            Although this class does not directly inherit from Observable,
            it contains an inner class which does. This methods act as proxies
            for the corresponding methods of the data member.
        */
        //@{
        void registerObserver(Patterns::Observer*);
        void unregisterObserver(Patterns::Observer*);
        //@}
      private:
        Handle<Type>** ptr_;
        int* n_;
        // assignment would join two handle groups - 
        // inhibit it until we found out if we really want this.
        // It doesn't seem a good idea right now.
        RelinkableHandle& operator=(const RelinkableHandle& from) { 
            return *this; }
        class InnerObserver : public Patterns::Observable,
                              public Patterns::Observer {
          public:
            // Observer interface
            void update() { notifyObservers(); }
        };
        InnerObserver* observer_;
        bool *registeredAsObserver_;
    };


    // inline definitions

    template <class Type>
    inline RelinkableHandle<Type>::RelinkableHandle()
    : ptr_(new Handle<Type>*(new Handle<Type>)), n_(new int(1)),
      observer_(new InnerObserver), registeredAsObserver_(new bool(false)) {}

    template <class Type>
    inline RelinkableHandle<Type>::RelinkableHandle(
        const RelinkableHandle<Type>& from)
    : ptr_(from.ptr_), n_(from.n_), observer_(from.observer_),
      registeredAsObserver_(from.registeredAsObserver_) {
        (*n_)++;
    }

    template <class Type>
    inline RelinkableHandle<Type>::~RelinkableHandle() {
        if (--(*n_) == 0) {
            if (!isNull() && *registeredAsObserver_)
                (**ptr_)->unregisterObserver(observer_);
            delete *ptr_;
            delete ptr_;
            delete n_;
            delete observer_;
        }
    }

    template <class Type>
    inline void RelinkableHandle<Type>::linkTo(const Handle<Type>& h,
                                               bool registerAsObserver) {
        if (!isNull() && *registeredAsObserver_)
            (**ptr_)->unregisterObserver(observer_);
        delete *ptr_;
        *ptr_  = new Handle<Type>(h);
        *registeredAsObserver_ = registerAsObserver;
        if (!isNull() && registerAsObserver)
            (**ptr_)->registerObserver(observer_);
        observer_->notifyObservers();
    }

    template <class Type>
    inline Type& RelinkableHandle<Type>::operator*() const {
        QL_REQUIRE(!isNull(), "tried to dereference null handle");
        return (*ptr_)->operator*();
    }

    template <class Type>
    inline Type* RelinkableHandle<Type>::operator->() const {
        QL_REQUIRE(!isNull(), "tried to dereference null handle");
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

    template <class Type>
    inline void 
    RelinkableHandle<Type>::registerObserver(Patterns::Observer* o) {
        observer_->registerObserver(o);
    }

    template <class Type>
    inline void 
    RelinkableHandle<Type>::unregisterObserver(Patterns::Observer* o) {
        observer_->unregisterObserver(o);
    }

}


#endif
