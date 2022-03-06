/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl

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

/*! \file observablevalue.hpp
    \brief observable and assignable proxy to concrete value
*/

#ifndef quantlib_observable_value_hpp
#define quantlib_observable_value_hpp

#include <ql/patterns/observable.hpp>

namespace QuantLib {

    //! %observable and assignable proxy to concrete value
    /*! Observers can be registered with instances of this class so
        that they are notified when a different value is assigned to
        such instances. Client code can copy the contained value or
        pass it to functions via implicit conversion.
        \note it is not possible to call non-const method on the
              returned value. This is by design, as this possibility
              would necessarily bypass the notification code; client
              code should modify the value via re-assignment instead.
    */
    template <class T>
    class ObservableValue {
      public:
        ObservableValue();
        ObservableValue(const T&);
        ObservableValue(const ObservableValue<T>&);
        //! \name controlled assignment
        //@{
        ObservableValue<T>& operator=(const T&);
        ObservableValue<T>& operator=(const ObservableValue<T>&);
        //@}
        //! implicit conversion
        operator T() const;
        operator ext::shared_ptr<Observable>() const;
        //! explicit inspector
        const T& value() const;
      private:
        T value_;
        ext::shared_ptr<Observable> observable_;
    };


    // template definition

    template <class T>
    ObservableValue<T>::ObservableValue()
    : value_(), observable_(new Observable) {}

    template <class T>
    ObservableValue<T>::ObservableValue(const T& t)
    : value_(t), observable_(new Observable) {}

    template <class T>
    ObservableValue<T>::ObservableValue(const ObservableValue<T>& t)
    : value_(t.value_), observable_(new Observable) {}

    template <class T>
    ObservableValue<T>& ObservableValue<T>::operator=(const T& t) {
        value_ = t;
        observable_->notifyObservers();
        return *this;
    }

    template <class T>
    ObservableValue<T>&
    ObservableValue<T>::operator=(const ObservableValue<T>& t) { // NOLINT(bugprone-unhandled-self-assignment)
        value_ = t.value_;
        observable_->notifyObservers();
        return *this;
    }

    template <class T>
    ObservableValue<T>::operator T() const {
        return value_;
    }

    template <class T>
    ObservableValue<T>::operator ext::shared_ptr<Observable>() const {
        return observable_;
    }

    template <class T>
    const T& ObservableValue<T>::value() const {
        return value_;
    }

}

#endif


#ifndef id_bb7737a20cdb7db29d9b39de3ae52ccd
#define id_bb7737a20cdb7db29d9b39de3ae52ccd
inline bool test_bb7737a20cdb7db29d9b39de3ae52ccd(int* i) { return i != 0; }
#endif
