
/*
 Copyright (C) 2000-2004 StatPro Italia srl

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

/*! \file null.hpp
    \brief null values
*/

#ifndef quantlib_null_h
#define quantlib_null_h

#include <ql/types.hpp>

namespace QuantLib {

    //! template class providing a null value for a given type.
    template <class Type>
    class Null {
      public:
        Null() {}
        operator Type() const { return Type(); }
    };

#if !defined(__DOXYGEN__)

    /* here we're trying to cover a number of types that one would possibly
       use as Real, Integer and such. Were one to add his own user-defined
       type, a suitable Null specialization should be added. */

    template <>
    class Null<int> {
      public:
        Null() {}
        operator int() const { return int(QL_NULL_INTEGER); }
    };

    template <>
    class Null<long> {
      public:
        Null() {}
        operator long() const { return long(QL_NULL_INTEGER); }
    };

    template <>
    class Null<unsigned int> {
      public:
        Null() {}
        operator unsigned int() const {
            return (unsigned int)(QL_NULL_INTEGER);
        }
    };

    template <>
    class Null<unsigned long> {
      public:
        Null() {}
        operator unsigned long() const {
            return (unsigned long)(QL_NULL_INTEGER);
        }
    };

    #if defined(HAVE_LONG_LONG)

    template <>
    class Null<long long> {
      public:
        Null() {}
        operator long long() const { return (long long)(QL_NULL_INTEGER); }
    };

    template <>
    class Null<unsigned long long> {
      public:
        Null() {}
        operator unsigned long long() const {
            return (unsigned long long)(QL_NULL_INTEGER);
        }
    };

    #endif

    template <>
    class Null<float> {
      public:
        Null() {}
        operator float() const { return float(QL_NULL_REAL); }
    };

    template <>
    class Null<double> {
      public:
        Null() {}
        operator double() const { return double(QL_NULL_REAL); }
    };

    template <>
    class Null<long double> {
      public:
        Null() {}
        operator long double() const { return (long double)(QL_NULL_REAL); }
    };

#endif

}


#endif
