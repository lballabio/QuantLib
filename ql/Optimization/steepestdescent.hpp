

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
/*! \file steepestdescent.hpp
    \brief Steepest descent optimization method

    \fullpath
    ql/Optimization/%steepestdescent.hpp
*/

#ifndef quantlib_optimization_steepest_descent_h
#define quantlib_optimization_steepest_descent_h

#include "ql/Optimization/armijo.hpp"

namespace QuantLib {

    namespace Optimization {

        //! Multi-dimensionnal Steepest Descend class
        /*! User has to provide line-search method and
            optimization end criteria

            search direction \f$ = - f'(x) \f$
        */
        class SteepestDescent:public OptimizationMethod {
          public:
            //! default default constructor (msvc bug)
            SteepestDescent()
            : OptimizationMethod(),
              lineSearch_(Handle<LineSearch>(new ArmijoLineSearch ())) {}

            //! default constructor
            SteepestDescent(const Handle<LineSearch>& lineSearch)
            : OptimizationMethod(), lineSearch_(lineSearch) {}
            //! destructor
            virtual ~SteepestDescent() {}

            //! minimize the optimization problem P
            virtual void Minimize(OptimizationProblem& P);
          private:
            //! line search
            Handle<LineSearch> lineSearch_;
        };

    }

}


#endif
