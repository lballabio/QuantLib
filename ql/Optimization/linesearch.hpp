
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
            typedef double value_type;

            //! Default constructor
            LineSearch (double eps = 1e-8):too_small_ (eps), too_big_ (1. / eps),
            maxLoop_ (100), qt_ (0.), qpt_ (0.), succeed_ (true) {}
            //! Destructor
            virtual ~LineSearch() {}

            //! return last cost function value
            double lastFunctionValue() { return qt_; }
            //! return square norm of last gradient
            double lastGradientNorm2() {return qpt_;}

            //! return last x value
            Array& lastX() { return xtd_; }
            //! return last gradient
            Array& lastGradient() { return gradient_; }

            //! set max number of loop
            void setMaxLoop (int maxLoop) { maxLoop_ = maxLoop; }

            //! Set too small criteria
            void setTooSmall (double too_small) { too_small_ = too_small; }
            //! Set too big criteria
            void setTooBig (double too_big) { too_big_ = too_big; }

            bool succeed () { return succeed_; }

            //! Perform line search
            virtual double operator () (OptimizationProblem &P,
                            value_type t_ini, value_type q0,
                            value_type qp0) = 0;
          protected:
            //! big and small quantities
            double too_small_, too_big_;
            //! maximum number of loop in line
            int maxLoop_;
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
