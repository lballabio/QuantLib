
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
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

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! template class providing a null value for a given type.
    template <class Type>
    class Null {
      public:
        Null() {}
        operator Type() const { return Type(); }
    };

    #if !defined(__DOXYGEN__)
    template <>
    class Null<int> {
      public:
        Null() {}
        operator int() const { return QL_MAX_INT; }
    };

    template <>
    class Null<double> {
      public:
        Null() {}
        operator double() const { return QL_MAX_DOUBLE; }
    };
    #endif

}


#endif
