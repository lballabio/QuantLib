// Emacs will be in -*- Mode: c++ -*-
//
// ************ DO NOT REMOVE THIS BANNER ****************
//
//  Nicolas Di Cesare <Nicolas.Dicesare@free.fr>
//  http://acm.emath.fr/~dicesare
//
//********************************************************
//
//  Armijo Line Search
//
//********************************************************
#ifndef quantlib_optimization_armijo_h_
#define quantlib_optimization_armijo_h

#include "ql/Optimization/linesearch.hpp"
#include "ql/Optimization/optimizer.hpp"

/*! 
  Armijo linesearch.

  Let alpha and beta be 2 scalars in [0,1].
  Let x be the current value of the unknow, d the search direction and 
  t the step. Let f be the function to minimize. 
  The line search stop when t verifies
  f(x+t*d) - f(x) <= -alpha*t*f'(x+t*d) and f(x+t/beta*d) - f(x) > -alpha*t*f'(x+t*d)/beta

  (see Polak. Algorithms and consitent approximations, Optimization, 
  volume 124 of Apllied Mathematical Sciences. Springer-Arrayerlag, N-Y, 1997)
*/

namespace QuantLib {

    namespace Optimization {

        class ArmijoLineSearch:public LineSearch {
          public:
            typedef double value_type;
          protected:
            //! Armijo paramters
            double alpha_, beta_;
          public:
            //! Default constructor
            ArmijoLineSearch (double eps = 1e-8, double alpha = 0.5, 
                double beta = 0.65)
            : LineSearch(eps), alpha_(alpha), beta_(beta) {}
            //! Destructor
            virtual ~ ArmijoLineSearch () {}

            //! Perform line search
            virtual double operator () (OptimizationProblem &P,	// Optimization problem 
                            value_type t_ini,	// initial value of line-search step
                            value_type q0,	// function value
                            value_type qp0);	// squared norm of gradient vector
        };

    }

}


#endif
