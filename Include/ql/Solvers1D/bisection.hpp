
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

/*! \file bisection.hpp
    \brief bisection 1-D solver

    $Source$
    $Log$
    Revision 1.3  2001/05/15 12:39:38  lballabio
    Removed unnecessary constructors

    Revision 1.2  2001/05/09 11:06:18  nando
    A few comments modified/removed

    Revision 1.1  2001/04/09 14:05:49  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

*/

#ifndef quantlib_solver1d_bisection_h
#define quantlib_solver1d_bisection_h

#include "ql/qldefines.hpp"
#include "ql/solver1d.hpp"

namespace QuantLib {

    namespace Solvers1D {

        class Bisection : public Solver1D {
          protected:
            double solve_(const ObjectiveFunction& f, double xAccuracy) const;
        };

    }

}

#endif
