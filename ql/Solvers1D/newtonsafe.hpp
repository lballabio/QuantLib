
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

/*! \file newtonsafe.hpp
    \brief safe Newton 1-D solver

    \fullpath
    ql/Solvers1D/%newtonsafe.hpp
*/

// $Id$

#ifndef quantlib_solver1d_newtonsafe_h
#define quantlib_solver1d_newtonsafe_h

#include "ql/solver1d.hpp"


namespace QuantLib {

    namespace Solvers1D {

        //! safe %Newton 1-D solver
        class NewtonSafe : public Solver1D {
          private:
            double solve_(const ObjectiveFunction& f, double xAccuracy) const;
        };

    }

}


#endif
