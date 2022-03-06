/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file disposable.hpp
    \brief generic disposable object with move semantics
*/

#ifndef quantlib_disposable_hpp
#define quantlib_disposable_hpp

#include <ql/qldefines.hpp>

namespace QuantLib {

    #ifndef QL_USE_DISPOSABLE

    template <class T>
    using Disposable = T;

    #else

    #pragma message("Warning: enabling the old Disposable class template is deprecated.")
    #pragma message("    If you're using --enable-disposable in your build")
    #pragma message("    or if you defined QL_USE_DISPOSABLE in ql/userconfig.hpp,")
    #pragma message("    please restore the default compilation options in the near future.")

    //! generic disposable object with move semantics
    /*! This class can be used for returning a value by copy. It relies
        on the returned object exposing a <tt>swap(T\&)</tt> method through
        which the copy constructor and assignment operator are implemented,
        thus resulting in actual move semantics. Typical use of this
        class is along the following lines:
        \code
        Disposable<Foo> bar(Integer i) {
            Foo f(i*2);
            return f;
        }
        \endcode

        \warning In order to avoid copies in code such as shown above,
                 the conversion from <tt>T</tt> to <tt>Disposable\<T\></tt>
                 is destructive, i.e., it does <b>not</b> preserve the
                 state of the original object. Therefore, it is necessary
                 for the developer to avoid code such as
        \code
        Disposable<Foo> bar(Foo& f) {
            return f;
        }
        \endcode
                 which would likely render the passed object unusable.
                 The correct way to obtain the desired behavior would be:
        \code
        Disposable<Foo> bar(Foo& f) {
            Foo temp = f;
            return temp;
        }
        \endcode
    */
    template <class T>
    class Disposable : public T {
      public:
        Disposable(T& t);
        Disposable(const Disposable<T>& t);
        Disposable<T>& operator=(const Disposable<T>& t);
    };


    // inline definitions

    template <class T>
    inline Disposable<T>::Disposable(T& t) {
        this->swap(t);
    }

    template <class T>
    inline Disposable<T>::Disposable(const Disposable<T>& t) : T() {
        this->swap(const_cast<Disposable<T>&>(t));
    }

    template <class T>
    inline Disposable<T>& Disposable<T>::operator=(const Disposable<T>& t) {
        this->swap(const_cast<Disposable<T>&>(t));
        return *this;
    }

    #endif

}


#endif


#ifndef id_034b0b25a7525a8b3029fbd4927ed200
#define id_034b0b25a7525a8b3029fbd4927ed200
inline bool test_034b0b25a7525a8b3029fbd4927ed200(int* i) { return i != 0; }
#endif
