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
#ifndef quantlib_optimization_conjugate_gradient_h
#define quantlib_optimization_conjugate_gradient_h

#include <iostream>
#include <iomanip>
#include <cmath>

#include "ql/array.hpp"
#include "ql/handle.hpp"

#include "ql/Optimization/optimizer.hpp"
#include "ql/Optimization/linesearch.hpp"
#include "ql/Optimization/criteria.hpp"
#include "ql/Optimization/armijo.hpp"


/*!
  Multi-dimensionnal Conjugate Gradient class
  User has to provide line-search method and
  optimization end criteria
  
  search direction d_i = - f'(x_i) + c_i*d_{i-1}
  where c_i = ||f'(x_i)||^2/||f'(x_{i-1})||^2 
  and d_1 = - f'(x_1)
  
*/
namespace QuantLib {

    namespace Optimization {

        class ConjugateGradient: public OptimizationMethod {
            //! line search
            Handle<LineSearch> lineSearch_;
          public:
            //! default constructor
            ConjugateGradient() : OptimizationMethod(),
              lineSearch_(Handle<LineSearch>(new ArmijoLineSearch() )) {}
            //! default constructor
            ConjugateGradient(Handle<LineSearch>& lineSearch)    // Reference to a line search method
            : OptimizationMethod(), lineSearch_ (lineSearch) {}


            //! destructor
            virtual ~ConjugateGradient() {}

            //! minimize the optimization problem P
            virtual void Minimize(OptimizationProblem& P);
        };

    }

}


#endif
