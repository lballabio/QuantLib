
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

/*! \file conjugategradient.cpp
    \brief Conjugate gradient optimization method

    \fullpath
    ql/Optimization/%conjugategradient.cpp
*/

#include "ql/Optimization/conjugategradient.hpp"

namespace QuantLib {

    namespace Optimization {

        void ConjugateGradient::Minimize(OptimizationProblem &P) {
            bool EndCriteria = false;

            // function and squared norm of gradient values;
            double f, fold, sd2, sdold2, g2, gold2, c = 0., normdiff = 0;
            // classical initial value for line-search step
            double t = 1.;

            // reference X as the optimization problem variable
            Array & X = x ();
            Array & SearchDirection = searchDirection ();
            // Set g at the size of the optimization problem search direction
            int sz = searchDirection ().size ();
            Array g (sz), d (sz), sddiff (sz);

            f = P.valueAndFirstDerivative (g, X);
            SearchDirection = -g;
            g2 = DotProduct (g, g);
            sd2 = g2;
            normdiff = sqrt (sd2);


            do {

              P.Save (iterationNumber (), f, sqrt (g2), t, *this);

              // Linesearch
              t = (*lineSearch_) (P, t, f, g2);

              if (lineSearch_->succeed ())
                {
                // Updates
                d = SearchDirection;
                // New point
                X = lineSearch_->lastX ();
                // New function value
                fold = f;
                f = lineSearch_->lastFunctionValue ();
                // New gradient and search direction vectors
                g = lineSearch_->lastGradient ();
                // orthogonalization coef
                gold2 = g2;
                g2 = lineSearch_->lastGradientNorm2 ();
                c = g2 / gold2;
                // conjugate gradient search direction
                sddiff = (-g + c * d) - SearchDirection;
                normdiff = sqrt (DotProduct (sddiff, sddiff));
                SearchDirection = -g + c * d;
                // New gradient squared norm
                sdold2 = sd2;
                sd2 = DotProduct (SearchDirection, SearchDirection);

                // End criteria
                EndCriteria =
                    endCriteria ()(iterationNumber_, fold, sqrt (gold2), f,
                           sqrt (g2), normdiff);

                // Increase interation number
                iterationNumber ()++;
                }
              }
            while ((EndCriteria == false) && (lineSearch_->succeed ()));

            P.Save (iterationNumber (), f, sqrt (g2), t, *this);

            if (!lineSearch_->succeed ())
            throw
                Error
                ("ConjugateGradient::Minimize(...), line-search failed!");
        }

    }

}
