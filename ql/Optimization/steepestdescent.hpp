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
#ifndef quantlib_optimization_steepest_descent_h
#define quantlib_optimization_steepest_descent_h

#include <iostream>
#include <iomanip>
#include <cmath>

#include "ql/array.hpp"
#include "ql/handle.hpp"

#include "ql/Optimization/optimizer.hpp"
#include "ql/Optimization/linesearch.hpp"
#include "ql/Optimization/criteria.hpp"
#include "ql/Optimization/armijo.hpp"

namespace QuantLib {

    namespace Optimization {

        /*!
          Multi-dimensionnal Steepest Descend class
          User has to provide line-search method and
          optimization end criteria

          search direction = - f'(x)
        */
        class SteepestDescent:public OptimizationMethod {
            //! line search
            Handle<LineSearch> lineSearch_;
          public:
            //! default default constructor (msvc bug)
            SteepestDescent()
            : OptimizationMethod (),
              lineSearch_(Handle<LineSearch>(new ArmijoLineSearch ())) {}

            //! default constructor
            SteepestDescent(Handle<LineSearch>& lineSearch)	// Reference to a line search method
            : OptimizationMethod(), lineSearch_ (lineSearch) {}
            //! destructor
            virtual ~SteepestDescent () {}

            //! minimize the optimization problem P
            virtual void Minimize (OptimizationProblem &P);
        };

    }

}

 
#endif
