// Emacs will be in -*- Mode: c++ -*-
//
// ************ DO NOT REMOVE THIS BANNER ****************
//
//  Nicolas Di Cesare <Nicolas.Dicesare@free.fr>
//  http://acm.emath.fr/~dicesare
//
//********************************************************
//
//  Conjugate gradient optimization method
//
//********************************************************

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

#ifdef DEBUG_CG
              std::cout << "searchDirection() = " << searchDirection ()
                  << "gradient          = " << g
                  << "norm of gradient  = " << sqrt (g2) << std::endl
                  << "search direction  = " << SearchDirection
                  << "norm              = " << sqrt (sd2) << std::endl
                  << "t                 = " << t << std::endl
                  << "c                 = " << c << std::endl
                  << "d                 = " << d
                  << "x                 = " << X << std::endl;
#endif
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
