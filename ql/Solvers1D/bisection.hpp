
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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

/*! \file bisection.hpp
    \brief bisection 1-D solver

    \fullpath
    ql/Solvers1D/%bisection.hpp
*/

// $Id$

#ifndef quantlib_solver1d_bisection_h
#define quantlib_solver1d_bisection_h

#include <ql/solver1d.hpp>

namespace QuantLib {

    namespace Solvers1D {

        //! %bisection 1-D solver
        class Bisection : public Solver1D {
          protected:
            double solve_(const ObjectiveFunction& f, double xAccuracy) const;
        };

    }

}

#endif
