/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file powell.hpp
    \brief Powell optimization method

    \fullpath
    ql/Optimization/%powell.hpp
*/

#ifndef quantlib_optimization_powell_h
#define quantlib_optimization_powell_h

#include <ql/Optimization/armijo.hpp>

namespace QuantLib {

    namespace Optimization {

        //! Multi-dimensionnal Powell class
        class Powell : public OptimizationMethod {
          public:
            //! default default constructor (msvc bug)
            Powell()
            : OptimizationMethod(),
              lineSearch_(Handle<LineSearch>(new ArmijoLineSearch ())) {}

            //! default constructor
            Powell(const Handle<LineSearch>& lineSearch)
            : OptimizationMethod(), lineSearch_(lineSearch) {}
            //! destructor
            virtual ~Powell() {}

            //! minimize the optimization problem P
            virtual void minimize(OptimizationProblem& P);
          private:
            //! line search
            Handle<LineSearch> lineSearch_;
        };

    }

}

#endif
