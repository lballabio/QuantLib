

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
/*! \file linesearch.hpp
    \brief Line search abstract class

    \fullpath
    ql/Optimization/%linesearch.hpp
*/

#ifndef quantlib_optimization_line_search_h_
#define quantlib_optimization_line_search_h_

#include "ql/Optimization/optimizer.hpp"

/*!
  Base class for line search
*/

namespace QuantLib {

    namespace Optimization {

        class LineSearch {
          public:
            //! Default constructor
            LineSearch(double eps = 1e-8)
            : qt_(0.0), qpt_(0.0), succeed_(true) {}
            //! Destructor
            virtual ~LineSearch() {}

            //! return last cost function value
            double lastFunctionValue() { return qt_; }
            //! return square norm of last gradient
            double lastGradientNorm2() { return qpt_;}
            //! return last x value
            Array& lastX() { return xtd_; }
            //! return last gradient
            Array& lastGradient() { return gradient_; }

            bool succeed() { return succeed_; }

            //! Perform line search
            virtual double operator() (
                OptimizationProblem &P,
                double t_ini,
                double q0,
                double qp0) = 0;
          protected:
            //! new x and its gradient
            Array xtd_, gradient_;
            //! cost function value and gradient norm corresponding to xtd_
            double qt_, qpt_;
            //! flag to know if linesearch succeed
            bool succeed_;

        };

    }

}

#endif
