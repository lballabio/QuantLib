// Emacs will be in -*- Mode: c++ -*-
//
// ************ DO NOT REMOVE THIS BANNER ****************
//
//  Nicolas Di Cesare <Nicolas.Dicesare@free.fr>
//  http://acm.emath.fr/~dicesare
//
//********************************************************
//
//  Steepest descent optimization method
//
//********************************************************

#include "ql/Optimization/steepestdescent.hpp"

namespace QuantLib {

    namespace Optimization {

        void SteepestDescent::Minimize (OptimizationProblem& P) {
            bool EndCriteria = false;

            // function and squared norm of gradient values;
            double fold, gold2, normdiff;
            // classical initial value for line-search step
            double t = 1.;

            // reference X as the optimization problem variable
            Array & X = x ();
            // Set gold at the size of the optimization problem search direction
            Array gold (searchDirection ().size ()), gdiff (searchDirection ().size ());

            fold = P.valueAndFirstDerivative (gold, X);
            searchDirection () = -gold;
            gold2 = DotProduct (gold, gold);
            normdiff = sqrt (gold2);

            do
              {
              P.Save (iterationNumber (), fold, sqrt (gold2), t, *this);

              // Linesearch
              t = (*lineSearch_) (P, t, fold, gold2);

              if (lineSearch_->succeed ())
                {
                // End criteria
                EndCriteria =
                    endCriteria ()(iterationNumber_, fold, sqrt (gold2),
                           lineSearch_->lastFunctionValue (),
                           sqrt (lineSearch_->lastGradientNorm2 ()),
                           normdiff);

                // Updates
                // New point
                X = lineSearch_->lastX ();
                // New function value
                fold = lineSearch_->lastFunctionValue ();
                // New gradient and search direction vectors
                gdiff = gold - lineSearch_->lastGradient ();
                normdiff = sqrt (DotProduct (gdiff, gdiff));
                gold = lineSearch_->lastGradient ();
                searchDirection () = -gold;
                // New gradient squared norm
                gold2 = lineSearch_->lastGradientNorm2 ();

                // Increase interation number
                iterationNumber ()++;
                }
              }
            while ((EndCriteria == false) && (lineSearch_->succeed ()));

            P.Save (iterationNumber (), fold, sqrt (gold2), t, *this);

            if (!lineSearch_->succeed ())
            throw
                Error ("SteepestDescent::Minimize(...), line-search failed!");
        }

    }

}
