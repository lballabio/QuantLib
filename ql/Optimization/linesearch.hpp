
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
