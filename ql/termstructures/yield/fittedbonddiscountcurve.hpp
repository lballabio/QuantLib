/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Allen Kuo
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2015 Andres Hernandez

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

/*! \file fittedbonddiscountcurve.hpp
    \brief discount curve fitted to a set of bonds
*/

#ifndef quantlib_fitted_bond_discount_curve_hpp
#define quantlib_fitted_bond_discount_curve_hpp

#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/math/optimization/method.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/math/array.hpp>
#include <ql/utilities/clone.hpp>

namespace QuantLib {

    //! Discount curve fitted to a set of fixed-coupon bonds
    /*! This class fits a discount function \f$ d(t) \f$ over a set of
        bonds, using a user defined fitting method. The discount
        function is fit in such a way so that all cashflows of all
        input bonds, when discounted using \f$ d(t) \f$, will
        reproduce the set of input bond prices in an optimized
        sense. Minimized price errors are weighted by the inverse of
        their respective bond duration.

        The FittedBondDiscountCurve class acts as a generic wrapper,
        while its inner class FittingMethod provides the
        implementation details. Developers thus need only derive new
        fitting methods from the latter.

        \warning The method can be slow if there are many bonds to
                 fit. Speed also depends on the particular choice of
                 fitting method chosen and its convergence properties
                 under optimization.  See also todo list for
                 BondDiscountCurveFittingMethod.

        \todo refactor the bond helper class so that it is pure
              virtual and returns a generic bond or its cash
              flows. Derived classes would include helpers for
              fixed-rate and zero-coupon bonds. In this way, both
              bonds and bills can be used to fit a discount curve
              using the exact same machinery. At present, only
              fixed-coupon bonds are supported. An even better way to
              move forward might be to get rate helpers to return
              cashflows, in which case this class could be used to fit
              any set of cash flows, not just bonds.

        \todo add more fitting diagnostics: smoothness, standard
              deviation, student-t test, etc. Generic smoothness
              method may be useful for smoothing splines fitting. See
              Fisher, M., D. Nychka and D. Zervos: "Fitting the term
              structure of interest rates with smoothing splines."
              Board of Governors of the Federal Reserve System,
              Federal Resere Board Working Paper, 95-1.

        \todo add extrapolation routines

        \ingroup yieldtermstructures
    */
    class FittedBondDiscountCurve : public YieldTermStructure,
                                    public LazyObject {
      public:
        class FittingMethod;

        //! \name Constructors
        //@{
        //! reference date based on current evaluation date
        FittedBondDiscountCurve(Natural settlementDays,
                                const Calendar& calendar,
                                std::vector<ext::shared_ptr<BondHelper> > bonds,
                                const DayCounter& dayCounter,
                                const FittingMethod& fittingMethod,
                                Real accuracy = 1.0e-10,
                                Size maxEvaluations = 10000,
                                Array guess = Array(),
                                Real simplexLambda = 1.0,
                                Size maxStationaryStateIterations = 100);
        //! curve reference date fixed for life of curve
        FittedBondDiscountCurve(const Date& referenceDate,
                                std::vector<ext::shared_ptr<BondHelper> > bonds,
                                const DayCounter& dayCounter,
                                const FittingMethod& fittingMethod,
                                Real accuracy = 1.0e-10,
                                Size maxEvaluations = 10000,
                                Array guess = Array(),
                                Real simplexLambda = 1.0,
                                Size maxStationaryStateIterations = 100);
        //@}

        //! \name Inspectors
        //@{
        //! total number of bonds used to fit the yield curve
        Size numberOfBonds() const;
        //! the latest date for which the curve can return values
        Date maxDate() const override;
        //! class holding the results of the fit
        const FittingMethod& fitResults() const;
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}

      private:
        void setup();
        void performCalculations() const override;
        DiscountFactor discountImpl(Time) const override;
        // target accuracy level to be used in the optimization routine
        Real accuracy_;
        // max number of evaluations to be used in the optimization routine
        Size maxEvaluations_;
        // sets the scale in the (Simplex) optimization routine
        Real simplexLambda_;
        // max number of evaluations where no improvement to solution is made
        Size maxStationaryStateIterations_;
        // a guess solution may be passed into the constructor to speed calcs
        Array guessSolution_;
        mutable Date maxDate_;
        std::vector<ext::shared_ptr<BondHelper> > bondHelpers_;
        Clone<FittingMethod> fittingMethod_;
    };


    //! Base fitting method used to construct a fitted bond discount curve
    /*! This base class provides the specific methodology/strategy
        used to construct a FittedBondDiscountCurve.  Derived classes
        need only define the virtual function discountFunction() based
        on the particular fitting method to be implemented, as well as
        size(), the number of variables to be solved for/optimized. The
        generic fitting methodology implemented here can be termed
        nonlinear, in contrast to (typically faster, computationally)
        linear fitting method.

        Optional parameters for FittingMethod include an Array of
        weights, which will be used as weights to each bond. If not given
        or empty, then the bonds will be weighted by inverse duration

        An optional Array may be provided as an L2 regularizor in this case
        a L2 (gaussian) penalty is applied to each parameter starting from the 
        initial guess. This is the same as giving a Gaussian prior on the parameters

        \todo derive the special-case class LinearFittingMethods from
              FittingMethod. A linear fitting to a set of basis
              functions \f$ b_i(t) \f$ is any fitting of the form
              \f[
              d(t) = \sum_{i=0} c_i b_i(t)
              \f]
              i.e., linear in the unknown coefficients \f$ c_i
              \f$. Such a fitting can be reduced to a linear algebra
              problem \f$ Ax = b \f$, and for large numbers of bonds,
              would typically be much faster computationally than the
              generic non-linear fitting method.

        \warning some parameters to the Simplex optimization method
                 may need to be tweaked internally to the class,
                 depending on the fitting method used, in order to get
                 proper/reasonable/faster convergence.
    */
    class FittedBondDiscountCurve::FittingMethod {
        friend class FittedBondDiscountCurve;
        // internal class
        class FittingCost;
      public:
        virtual ~FittingMethod() = default;
        //! total number of coefficients to fit/solve for
        virtual Size size() const = 0;
        //! output array of results of optimization problem
        Array solution() const;
        //! final number of iterations used in the optimization problem
        Integer numberOfIterations() const;
        //! final value of cost function after optimization
        Real minimumCostValue() const;
        //! error code of the optimization
        EndCriteria::Type errorCode() const;
        //! clone of the current object
        virtual std::unique_ptr<FittingMethod> clone() const = 0;
        //! return whether there is a constraint at zero
        bool constrainAtZero() const;
        //! return weights being used
        Array weights() const;
        //! return l2 penalties being used
        Array l2() const;
        //! return optimization method being used
        ext::shared_ptr<OptimizationMethod> optimizationMethod() const;
        //! return constraint of the solution being used
        const Constraint& constraint() const;
        //! open discountFunction to public
        DiscountFactor discount(const Array& x, Time t) const;
      protected:
        //! constructors
        FittingMethod(bool constrainAtZero = true,
                      const Array& weights = Array(),
                      ext::shared_ptr<OptimizationMethod> optimizationMethod = {},
                      Array l2 = Array(),
                      Real minCutoffTime = 0.0,
                      Real maxCutoffTime = QL_MAX_REAL,
                      Constraint constraint = NoConstraint{});
        //! rerun every time instruments/referenceDate changes
        virtual void init();
        //! discount function called by FittedBondDiscountCurve
        virtual DiscountFactor discountFunction(const Array& x,
                                                Time t) const = 0;

        //! constrains discount function to unity at \f$ T=0 \f$, if true
        bool constrainAtZero_;
        //! internal reference to the FittedBondDiscountCurve instance
        FittedBondDiscountCurve* curve_;
        //! solution array found from optimization, set in calculate()
        Array solution_;
        //! optional guess solution to be passed into constructor.
        /*! The idea is to use a previous solution as a guess solution to
            the discount curve, in an attempt to speed up calculations.
        */
        Array guessSolution_;
        //! base class sets this cost function used in the optimization routine
        ext::shared_ptr<FittingCost> costFunction_;
      private:
        // curve optimization called here- adjust optimization parameters here
        void calculate();
        // array of normalized (duration) weights, one for each bond helper
        Array weights_;
        // array of l2 penalties one for each parameter
        Array l2_;
        // whether or not the weights should be calculated internally
        bool calculateWeights_;
        // total number of iterations used in the optimization routine
        // (possibly including gradient evaluations)
        Integer numberOfIterations_;
        // final value for the minimized cost function
        Real costValue_;
        // error code returned by OptimizationMethod::minimize()
        EndCriteria::Type errorCode_ = EndCriteria::None;
        // optimization method to be used, if none provided use Simplex
        ext::shared_ptr<OptimizationMethod> optimizationMethod_;
        // flat extrapolation of instantaneous forward before / after cutoff
        Real minCutoffTime_, maxCutoffTime_;
        // constraint for the solution
        Constraint constraint_;
    };

    // inline

    inline Size FittedBondDiscountCurve::numberOfBonds() const {
        return bondHelpers_.size();
    }

    inline Date FittedBondDiscountCurve::maxDate() const {
        calculate();
        return maxDate_;
    }

    inline const FittedBondDiscountCurve::FittingMethod&
    FittedBondDiscountCurve::fitResults() const {
        calculate();
        return *fittingMethod_;
    }

    inline void FittedBondDiscountCurve::update() {
        YieldTermStructure::update();
        LazyObject::update();
    }

    inline void FittedBondDiscountCurve::setup() {
        for (auto& bondHelper : bondHelpers_)
            registerWith(bondHelper);
    }

    inline DiscountFactor FittedBondDiscountCurve::discountImpl(Time t) const {
        calculate();
        return fittingMethod_->discount(fittingMethod_->solution_, t);
    }

    inline Integer
    FittedBondDiscountCurve::FittingMethod::numberOfIterations() const {
        return numberOfIterations_;
    }

    inline
    Real FittedBondDiscountCurve::FittingMethod::minimumCostValue() const {
        return costValue_;
    }

    inline 
    EndCriteria::Type FittedBondDiscountCurve::FittingMethod::errorCode() const {
        return errorCode_;
    }

    inline Array FittedBondDiscountCurve::FittingMethod::solution() const {
        return solution_;
    }
    
    inline bool FittedBondDiscountCurve::FittingMethod::constrainAtZero() const {
        return constrainAtZero_;
    }
    
    inline Array FittedBondDiscountCurve::FittingMethod::weights() const {
        return weights_;
    }

    inline Array FittedBondDiscountCurve::FittingMethod::l2() const {
        return l2_;
    }

    inline ext::shared_ptr<OptimizationMethod> 
    FittedBondDiscountCurve::FittingMethod::optimizationMethod() const {
        return optimizationMethod_;
    }

    inline const Constraint&
    FittedBondDiscountCurve::FittingMethod::constraint() const {
        return constraint_;
    }

    inline DiscountFactor FittedBondDiscountCurve::FittingMethod::discount(const Array& x, Time t) const {
        if (t < minCutoffTime_) {
            // flat fwd extrapolation before min cutoff time
            return std::exp(std::log(discountFunction(x, minCutoffTime_)) / minCutoffTime_ * t);
        } else if (t > maxCutoffTime_) {
            // flat fwd extrapolation after max cutoff time
            return discountFunction(x, maxCutoffTime_) *
                   std::exp((std::log(discountFunction(x, maxCutoffTime_ + 1E-4)) -
                             std::log(discountFunction(x, maxCutoffTime_))) *
                            1E4 * (t - maxCutoffTime_));
        } else {
            return discountFunction(x, t);
        }
    }
}

#endif
