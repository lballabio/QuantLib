
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

/*! \file leastsquare.hpp
    \brief Least square cost function

    \fullpath
    ql/Optimization/%leastsquare.hpp
*/

#ifndef quantlib_optimization_leastsquare_h__
#define quantlib_optimization_leastsquare_h__

#include "ql/Math/matrix.hpp"
#include "ql/Optimization/conjugategradient.hpp"

/*!
  Base class for least square problem
*/

namespace QuantLib {

    namespace Optimization {

        class LeastSquareProblem {
          public:
            //! size of the problem ie size of target vector
            virtual int size () = 0;
            //! compute the target vector and the values of the fonction to fit
            virtual void targetAndValue (const Array& x, Array& target,
                         Array & fct2fit) = 0;
            //! compute the target vector, the values of the fonction to fit and the matrix of derivatives
            virtual void targetValueAndfirstDerivative (const Array& x,
                Math::Matrix& grad_fct2fit, Array& target, Array& fct2fit) = 0;
        };

        /*!
           Design a least square function as a cost function using
           the interface provided by LeastSquareProblem class.
           Array vector class requires function DotProduct() that computes dot product
           and - operator
           M matrix class requires function transpose() that computes transpose
           and * operator with vector class
         */
        class LeastSquareFunction:public CostFunction {
          public:
            //! value type of the vector
            typedef double value_type;
          protected:
            //! least square problem
              LeastSquareProblem &lsp_;
          public:
            //! Default constructor


            LeastSquareFunction (LeastSquareProblem& lsp) : lsp_(lsp) {}
            //! Destructor
            virtual ~LeastSquareFunction () {}

            //! compute value of the least square function
            virtual value_type value (const Array& x);
            //! compute vector of derivatives of the least square function
            virtual void firstDerivative (Array& grad_f, const Array& x);
            //! compute value and vector of derivatives of the least square function
            virtual value_type valueAndFirstDerivative (Array& grad_f,
                                const Array& x);

            //! to improve
            virtual void Update () {}
            //! to improve
            virtual void Save(OptimizationMethod&) {}
        };


        inline double LeastSquareFunction::value (const Array & x) {
            // size of target and function to fit vectors
            Array target (lsp_.size ()), fct2fit (lsp_.size ());
            // compute its values
            lsp_.targetAndValue (x, target, fct2fit);
            // do the difference
            Array diff = target - fct2fit;
            // and compute the scalar product (square of the norm)
            return DotProduct (diff, diff);
        }

        inline void LeastSquareFunction::firstDerivative (Array& grad_f,
           const Array& x) {
            // size of target and function to fit vectors
            Array target (lsp_.size ()), fct2fit (lsp_.size ());
            // size of gradient matrix
            Math::Matrix grad_fct2fit (lsp_.size (), x.size ());
            // compute its values
              lsp_.targetValueAndfirstDerivative (x, grad_fct2fit, target, fct2fit);
            // do the difference
            Array diff = target - fct2fit;
            // compute derivative
            grad_f = -2. * (Math::transpose(grad_fct2fit) * diff);
        }

        inline double LeastSquareFunction::valueAndFirstDerivative(Array& grad_f,
                                       const Array& x)
        {
            // size of target and function to fit vectors
            Array target (lsp_.size ()), fct2fit (lsp_.size ());
            // size of gradient matrix
            Math::Matrix grad_fct2fit (lsp_.size (), x.size ());
            // compute its values
            lsp_.targetValueAndfirstDerivative (x, grad_fct2fit, target,
                            fct2fit);
            // do the difference
            Array diff = target - fct2fit;
            // compute derivative
            grad_f = -2. * (Math::transpose (grad_fct2fit) * diff);
            // and compute the scalar product (square of the norm)
            return DotProduct (diff, diff);
        }

        /*!
           Default least square method using a given
           optimization algorithm (default is conjugate gradient).

           min { r(x) : x in R^n }

           where r(x) = ||f(x)||^2 the euclidian norm of f(x)
           for some vector-valued function f from R^n to R^m
           f = (f1, ..., fm) with fi(x) = bi - phi(x,ti)
           where bi is the vector of target data and phi
           is a scalar function.

           Assuming the differentiability of f, the gradient of r
           is define by
           grad r(x) = f'(x)^t.f(x)

           Array vector class has the requirement of the previous class
           Handle class is need to manage pointer to optimization method
         */
        class NonLinearLeastSquare {
            //! solution vector
            Array results_, initialValue_;
            //! least square residual norm
            double resnorm_;
            //! Exit flag of the optimization process
            int exitFlag_;
            //! required accuracy of the solver
            double accuracy_, bestAccuracy_;
            //! maximum and real number of iterations
            unsigned int maxIterations_, nbIterations_;
            //! Optimization method
            Handle<OptimizationMethod> om_;
          public:
            //! Default constructor
            inline NonLinearLeastSquare (double accuracy = 1e-4,
                         int maxiter = 100);
            //! Default constructor
            inline NonLinearLeastSquare (double accuracy,
                         int maxiter,
                         Handle<OptimizationMethod> om);
            //! Destructor
            inline ~NonLinearLeastSquare () {}

            //! Solve least square problem using numerix solver
            inline Array& Perform (LeastSquareProblem& lsProblem) {
                double eps = accuracy_;

                // set initial value of the optimization method
                om_->setInitialValue (initialValue_);
                // set end criteria with a given maximum number of iteration and a given error eps
                om_->
                    setEndCriteria (OptimizationEndCriteria
                            (maxIterations_, eps));
                om_->endCriteria ().setPositiveOptimization ();

                // wrap the least square problem in an optimization function
                LeastSquareFunction lsf(lsProblem);

                // define optimization problem
                OptimizationProblem P(lsf, *om_);

                // minimize
                P.Minimize();

                // summarize results of minimization
                exitFlag_ = om_->endCriteria ().criteria ();
                nbIterations_ = om_->iterationNumber ();

                results_ = om_->x ();
                resnorm_ = om_->functionValue();
                bestAccuracy_ = om_->functionValue();

                return results_;
            }

            inline void setInitialValue (const Array & initialValue) {
                initialValue_ = initialValue;
            }

            //! return the results
            inline Array& results () { return results_; }
            //! return the least square residual norm
            inline double residualNorm ()
            {
            return resnorm_;
            }
            //! return last function value
            inline double lastValue ()
            {
            return bestAccuracy_;
            }
            //! return exit flag
            inline int exitFlag ()
            {
            return exitFlag_;
            }
            //! return the performed number of iterations
            inline int iterationsNumber ()
            {
            return nbIterations_;
            }
        };


        inline NonLinearLeastSquare::NonLinearLeastSquare (double accuracy,
          int maxiter):
        exitFlag_(-1), accuracy_ (accuracy), maxIterations_ (maxiter),
            om_ (Handle<OptimizationMethod>(new ConjugateGradient()))
        {}

        inline NonLinearLeastSquare::NonLinearLeastSquare (double accuracy,
            int maxiter, Handle<OptimizationMethod> om)
        : exitFlag_(-1), accuracy_ (accuracy), maxIterations_ (maxiter),
          om_ (om) {}

    }

}


#endif
