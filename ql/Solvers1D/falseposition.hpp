
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

/*! \file falseposition.hpp
    \brief false-position 1-D solver

    \fullpath
    ql/Solvers1D/%falseposition.hpp
*/

// $Id$

#ifndef quantlib_solver1d_falseposition_h
#define quantlib_solver1d_falseposition_h

#include <ql/solver1d.hpp>

namespace QuantLib {

    namespace Solvers1D {

        //! False position 1-D solver
        class FalsePosition : public Solver1D {
          private:
            double solve_(const ObjectiveFunction& f, double xAccuracy) const;
        };

    }

}

#endif
