
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/05/24 12:52:01  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:03:54  nando
    all the *.hpp moved below the Include/ql level

*/

/*! \file null.hpp
    \brief null values
*/

#ifndef quantlib_null_h
#define quantlib_null_h

#include "ql/qldefines.hpp"

namespace QuantLib {

    //! template class providing a null value.
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
