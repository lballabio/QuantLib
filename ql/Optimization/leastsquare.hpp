
/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file leastsquare.hpp
    \brief Least square cost function
*/

#ifndef quantlib_least_square_hpp
#define quantlib_least_square_hpp

#include <ql/Math/matrix.hpp>
#include <ql/Optimization/conjugategradient.hpp>

namespace QuantLib {

    //! Base class for least square problem
    class LeastSquareProblem {
      public:
        //! size of the problem ie size of target vector
        virtual Size size() = 0;
        //! compute the target vector and the values of the function to fit
        virtual void targetAndValue(const Array& x, Array& target,
                                    Array& fct2fit) = 0;
        /*! compute the target vector, the values of the function to fit
            and the matrix of derivatives
        */
        virtual void targetValueAndGradient(const Array& x,
                                            Matrix& grad_fct2fit,
                                            Array& target,
                                            Array& fct2fit) = 0;
    };

    //! Cost function for least-square problems
    /*! Implements a cost function using the interface provided by
        the LeastSquareProblem class.
    */
    class LeastSquareFunction : public CostFunction {
      public:
        //! Default constructor
        LeastSquareFunction(LeastSquareProblem& lsp) : lsp_(lsp) {}
        //! Destructor
        virtual ~LeastSquareFunction() {}

        //! compute value of the least square function
        virtual Real value(const Array& x) const;
        //! compute vector of derivatives of the least square function
        virtual void gradient(Array& grad_f, const Array& x) const;
        //! compute value and gradient of the least square function
        virtual Real valueAndGradient(Array& grad_f,
                                      const Array& x) const;
      protected:
        //! least square problem
        LeastSquareProblem &lsp_;
    };

    //! Non-linear least-square method.
    /*! Using a given optimization algorithm (default is conjugate
        gradient),

        \f[ min \{ r(x) : x in R^n \} \f]

        where \f$ r(x) = |f(x)|^2 \f$ is the Euclidean norm of \f$
        f(x) \f$ for some vector-valued function \f$ f \f$ from
        \f$ R^n \f$ to \f$ R^m \f$,
        \f[ f = (f_1, ..., f_m) \f]
        with \f$ f_i(x) = b_i - \phi(x,t_i) \f$ where \f$ b \f$ is the
        vector of target data and \f$ phi \f$ is a scalar function.

        Assuming the differentiability of \f$ f \f$, the gradient of
        \f$ r \f$ is defined by
        \f[ grad r(x) = f'(x)^t.f(x) \f]
    */
    class NonLinearLeastSquare {
      public:
        //! Default constructor
        inline NonLinearLeastSquare(Constraint& c,
                                    Real accuracy = 1e-4,
                                    Size maxiter = 100);
        //! Default constructor
        inline NonLinearLeastSquare(Constraint& c,
                                    Real accuracy,
                                    Size maxiter,
                                    boost::shared_ptr<OptimizationMethod> om);
        //! Destructor
        inline ~NonLinearLeastSquare() {}

        //! Solve least square problem using numerix solver
        inline Array& perform(LeastSquareProblem& lsProblem);

        inline void setInitialValue(const Array& initialValue) {
            initialValue_ = initialValue;
        }

        //! return the results
        inline Array& results() { return results_; }

        //! return the least square residual norm
        inline Real residualNorm() { return resnorm_; }

        //! return last function value
        inline Real lastValue() { return bestAccuracy_; }

        //! return exit flag
        inline Integer exitFlag() { return exitFlag_; }

        //! return the performed number of iterations
        inline Integer iterationsNumber() { return nbIterations_; }
      private:
        //! solution vector
        Array results_, initialValue_;
        //! least square residual norm
        Real resnorm_;
        //! Exit flag of the optimization process
        Integer exitFlag_;
        //! required accuracy of the solver
        Real accuracy_, bestAccuracy_;
        //! maximum and real number of iterations
        Size maxIterations_, nbIterations_;
        //! Optimization method
        boost::shared_ptr<OptimizationMethod> om_;
        //constraint
        Constraint& c_;

    };

    // inline definitions

    inline Real LeastSquareFunction::value(const Array & x) const {
        // size of target and function to fit vectors
        Array target(lsp_.size()), fct2fit(lsp_.size());
        // compute its values
        lsp_.targetAndValue(x, target, fct2fit);
        // do the difference
        Array diff = target - fct2fit;
        // and compute the scalar product (square of the norm)
        return DotProduct(diff, diff);
    }

    inline void LeastSquareFunction::gradient(Array& grad_f,
                                              const Array& x) const {
        // size of target and function to fit vectors
        Array target (lsp_.size ()), fct2fit (lsp_.size ());
        // size of gradient matrix
        Matrix grad_fct2fit (lsp_.size (), x.size ());
        // compute its values
        lsp_.targetValueAndGradient(x, grad_fct2fit, target, fct2fit);
        // do the difference
        Array diff = target - fct2fit;
        // compute derivative
        grad_f = -2.0*(transpose(grad_fct2fit)*diff);
    }

    inline Real LeastSquareFunction::valueAndGradient(Array& grad_f,
                                                      const Array& x) const {
        // size of target and function to fit vectors
        Array target(lsp_.size()), fct2fit(lsp_.size());
        // size of gradient matrix
        Matrix grad_fct2fit(lsp_.size(), x.size());
        // compute its values
        lsp_.targetValueAndGradient(x, grad_fct2fit, target, fct2fit);
        // do the difference
        Array diff = target - fct2fit;
        // compute derivative
        grad_f = -2.0*(transpose(grad_fct2fit)*diff);
        // and compute the scalar product (square of the norm)
        return DotProduct(diff, diff);
    }

    inline NonLinearLeastSquare::NonLinearLeastSquare(Constraint& c,
                                                      Real accuracy,
                                                      Size maxiter)
    : exitFlag_(-1), accuracy_ (accuracy), maxIterations_ (maxiter),
      om_ (boost::shared_ptr<OptimizationMethod>(new ConjugateGradient())),
      c_(c)
    {}

    inline NonLinearLeastSquare::NonLinearLeastSquare(
                                     Constraint& c,
                                     Real accuracy,
                                     Size maxiter,
                                     boost::shared_ptr<OptimizationMethod> om)
    : exitFlag_(-1), accuracy_ (accuracy), maxIterations_ (maxiter),
      om_ (om), c_(c) {}

    inline
    Array& NonLinearLeastSquare::perform(LeastSquareProblem& lsProblem) {
        Real eps = accuracy_;

        // set initial value of the optimization method
        om_->setInitialValue (initialValue_);
        // set end criteria with a given maximum number of iteration
        // and a given error eps
        om_->setEndCriteria(EndCriteria(maxIterations_, eps));
        om_->endCriteria().setPositiveOptimization();

        // wrap the least square problem in an optimization function
        LeastSquareFunction lsf(lsProblem);

        // define optimization problem
        Problem P(lsf, c_, *om_);

        // minimize
        P.minimize();

        // summarize results of minimization
        exitFlag_ = om_->endCriteria ().criteria();
        nbIterations_ = om_->iterationNumber();

        results_ = om_->x();
        resnorm_ = om_->functionValue();
        bestAccuracy_ = om_->functionValue();

        return results_;
    }

}


#endif
