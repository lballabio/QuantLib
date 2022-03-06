/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file solver1d.hpp
    \brief Abstract 1-D solver class
*/

#ifndef quantlib_solver1d_hpp
#define quantlib_solver1d_hpp

#include <ql/math/comparison.hpp>
#include <ql/utilities/null.hpp>
#include <ql/patterns/curiouslyrecurring.hpp>
#include <ql/errors.hpp>
#include <algorithm>
#include <iomanip>

namespace QuantLib {

    #define MAX_FUNCTION_EVALUATIONS 100

    //! Base class for 1-D solvers
    /*! The implementation of this class uses the so-called
        "Barton-Nackman trick", also known as "the curiously recurring
        template pattern". Concrete solvers will be declared as:
        \code
        class Foo : public Solver1D<Foo> {
          public:
            ...
            template <class F>
            Real solveImpl(const F& f, Real accuracy) const {
                ...
            }
        };
        \endcode
        Before calling <tt>solveImpl</tt>, the base class will set its
        protected data members so that:
        - <tt>xMin_</tt> and  <tt>xMax_</tt> form a valid bracket;
        - <tt>fxMin_</tt> and <tt>fxMax_</tt> contain the values of
          the function in <tt>xMin_</tt> and <tt>xMax_</tt>;
        - <tt>root_</tt> is a valid initial guess.
        The implementation of <tt>solveImpl</tt> can safely assume all
        of the above.

        \todo
        - clean up the interface so that it is clear whether the
          accuracy is specified for \f$ x \f$ or \f$ f(x) \f$.
        - add target value (now the target value is 0.0)
    */
    template <class Impl>
    class Solver1D : public CuriouslyRecurringTemplate<Impl> {
      public:
        Solver1D() = default;
        //! \name Modifiers
        //@{
        /*! This method returns the zero of the function \f$ f \f$,
            determined with the given accuracy \f$ \epsilon \f$;
            depending on the particular solver, this might mean that
            the returned \f$ x \f$ is such that \f$ |f(x)| < \epsilon
            \f$, or that \f$ |x-\xi| < \epsilon \f$ where \f$ \xi \f$
            is the real zero.

            This method contains a bracketing routine to which an
            initial guess must be supplied as well as a step used to
            scan the range of the possible bracketing values.
        */
        template <class F>
        Real solve(const F& f,
                   Real accuracy,
                   Real guess,
                   Real step) const {

            QL_REQUIRE(accuracy>0.0,
                       "accuracy (" << accuracy << ") must be positive");
            // check whether we really want to use epsilon
            accuracy = std::max(accuracy, QL_EPSILON);

            const Real growthFactor = 1.6;
            Integer flipflop = -1;

            root_ = guess;
            fxMax_ = f(root_);

            // monotonically crescent bias, as in optionValue(volatility)
            if (close(fxMax_,0.0))
                return root_;
            else if (fxMax_ > 0.0) {
                xMin_ = enforceBounds_(root_ - step);
                fxMin_ = f(xMin_);
                xMax_ = root_;
            } else {
                xMin_ = root_;
                fxMin_ = fxMax_;
                xMax_ = enforceBounds_(root_+step);
                fxMax_ = f(xMax_);
            }

            evaluationNumber_ = 2;
            while (evaluationNumber_ <= maxEvaluations_) {
                if (fxMin_*fxMax_ <= 0.0) {
                    if (close(fxMin_, 0.0))
                        return xMin_;
                    if (close(fxMax_, 0.0))
                        return xMax_;
                    root_ = (xMax_+xMin_)/2.0;
                    return this->impl().solveImpl(f, accuracy);
                }
                if (std::fabs(fxMin_) < std::fabs(fxMax_)) {
                    xMin_ = enforceBounds_(xMin_+growthFactor*(xMin_ - xMax_));
                    fxMin_= f(xMin_);
                } else if (std::fabs(fxMin_) > std::fabs(fxMax_)) {
                    xMax_ = enforceBounds_(xMax_+growthFactor*(xMax_ - xMin_));
                    fxMax_= f(xMax_);
                } else if (flipflop == -1) {
                    xMin_ = enforceBounds_(xMin_+growthFactor*(xMin_ - xMax_));
                    fxMin_= f(xMin_);
                    evaluationNumber_++;
                    flipflop = 1;
                } else if (flipflop == 1) {
                    xMax_ = enforceBounds_(xMax_+growthFactor*(xMax_ - xMin_));
                    fxMax_= f(xMax_);
                    flipflop = -1;
                }
                evaluationNumber_++;
            }

            QL_FAIL("unable to bracket root in " << maxEvaluations_
                    << " function evaluations (last bracket attempt: "
                    << "f[" << xMin_ << "," << xMax_ << "] "
                    << "-> [" << fxMin_ << "," << fxMax_ << "])");
        }
        /*! This method returns the zero of the function \f$ f \f$,
            determined with the given accuracy \f$ \epsilon \f$;
            depending on the particular solver, this might mean that
            the returned \f$ x \f$ is such that \f$ |f(x)| < \epsilon
            \f$, or that \f$ |x-\xi| < \epsilon \f$ where \f$ \xi \f$
            is the real zero.

            An initial guess must be supplied, as well as two values
            \f$ x_\mathrm{min} \f$ and \f$ x_\mathrm{max} \f$ which
            must bracket the zero (i.e., either \f$ f(x_\mathrm{min})
            \leq 0 \leq f(x_\mathrm{max}) \f$, or \f$
            f(x_\mathrm{max}) \leq 0 \leq f(x_\mathrm{min}) \f$ must
            be true).
        */
        template <class F>
        Real solve(const F& f,
                   Real accuracy,
                   Real guess,
                   Real xMin,
                   Real xMax) const {

            QL_REQUIRE(accuracy>0.0,
                       "accuracy (" << accuracy << ") must be positive");
            // check whether we really want to use epsilon
            accuracy = std::max(accuracy, QL_EPSILON);

            xMin_ = xMin;
            xMax_ = xMax;

            QL_REQUIRE(xMin_ < xMax_,
                       "invalid range: xMin_ (" << xMin_
                       << ") >= xMax_ (" << xMax_ << ")");
            QL_REQUIRE(!lowerBoundEnforced_ || xMin_ >= lowerBound_,
                       "xMin_ (" << xMin_
                       << ") < enforced low bound (" << lowerBound_ << ")");
            QL_REQUIRE(!upperBoundEnforced_ || xMax_ <= upperBound_,
                       "xMax_ (" << xMax_
                       << ") > enforced hi bound (" << upperBound_ << ")");

            fxMin_ = f(xMin_);
            if (close(fxMin_, 0.0))
                return xMin_;

            fxMax_ = f(xMax_);
            if (close(fxMax_, 0.0))
                return xMax_;

            evaluationNumber_ = 2;

            QL_REQUIRE(fxMin_*fxMax_ < 0.0,
                       "root not bracketed: f["
                       << xMin_ << "," << xMax_ << "] -> ["
                       << std::scientific
                       << fxMin_ << "," << fxMax_ << "]");

            QL_REQUIRE(guess > xMin_,
                       "guess (" << guess << ") < xMin_ (" << xMin_ << ")");
            QL_REQUIRE(guess < xMax_,
                       "guess (" << guess << ") > xMax_ (" << xMax_ << ")");

            root_ = guess;

            return this->impl().solveImpl(f, accuracy);
        }

        /*! This method sets the maximum number of function
            evaluations for the bracketing routine. An error is thrown
            if a bracket is not found after this number of
            evaluations.
        */
        void setMaxEvaluations(Size evaluations);
        //! sets the lower bound for the function domain
        void setLowerBound(Real lowerBound);
        //! sets the upper bound for the function domain
        void setUpperBound(Real upperBound);
        //@}
      protected:
        mutable Real root_, xMin_, xMax_, fxMin_, fxMax_;
        Size maxEvaluations_ = MAX_FUNCTION_EVALUATIONS;
        mutable Size evaluationNumber_;
      private:
        Real enforceBounds_(Real x) const;
        Real lowerBound_, upperBound_;
        bool lowerBoundEnforced_ = false, upperBoundEnforced_ = false;
    };


    // inline definitions

    template <class T>
    inline void Solver1D<T>::setMaxEvaluations(Size evaluations) {
        maxEvaluations_ = evaluations;
    }

    template <class T>
    inline void Solver1D<T>::setLowerBound(Real lowerBound) {
        lowerBound_ = lowerBound;
        lowerBoundEnforced_ = true;
    }

    template <class T>
    inline void Solver1D<T>::setUpperBound(Real upperBound) {
        upperBound_ = upperBound;
        upperBoundEnforced_ = true;
    }

    template <class T>
    inline Real Solver1D<T>::enforceBounds_(Real x) const {
        if (lowerBoundEnforced_ && x < lowerBound_)
            return lowerBound_;
        if (upperBoundEnforced_ && x > upperBound_)
            return upperBound_;
        return x;
    }

}

#endif


#ifndef id_703549bfa3dc9b15bd754010f2343bd2
#define id_703549bfa3dc9b15bd754010f2343bd2
inline bool test_703549bfa3dc9b15bd754010f2343bd2(const int* i) {
    return i != nullptr;
}
#endif
