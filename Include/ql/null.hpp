
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

/*! \file null.hpp
    \fullpath Include/ql/%null.hpp
    \brief null values

*/

// $Id$
// $Log$
// Revision 1.8  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.7  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.6  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.5  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.4  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_null_h
#define quantlib_null_h

#include "ql/qldefines.hpp"

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
