
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

/*! \file boundarycondition.hpp
    \brief boundary conditions for differential operators

    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.7  2001/03/12 17:35:10  lballabio
    Removed global IsNull function - could have caused very vicious loops

    Revision 1.6  2001/02/19 12:21:40  marmar
    Added trailing _ to protected and private members

    Revision 1.5  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.4  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.3  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_boundary_condition
#define quantlib_boundary_condition

#include "qldefines.hpp"
#include "null.hpp"
#include "qlerrors.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        /* WARNING: for Neumann conditions. the value passed must not be the
           value of the derivative. Instead, it must be comprehensive of the
           grid step between the first two points--i.e., it must be the
           difference between f[0] and f[1].
        */

        class BoundaryCondition {
          public:
            // enumeration
            enum Type { None, Neumann, Dirichlet };
            // constructors
            BoundaryCondition(Type type = None, double value = Null<double>())
            : type_(type), value_(value) {
                if (type_ != None)
                    QL_REQUIRE(value != Null<double>(),
                        "A value must be supplied for "
                        "this type of boundary condition");
            }
            // access methods
            Type type() const { return type_; }
            double value() const { return value_; }
          private:
            Type type_;
            double value_;
        };

    }

}


#endif
