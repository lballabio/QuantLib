/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2007 Allen Kuo

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

        <b> Example: </b>
        \link FittedBondCurve.cpp
        compares various bond discount curve fitting methodologies
        \endlink

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
        friend class FittingMethod;

        //! \name Constructors
        //@{
        //! reference date based on current evaluation date
        FittedBondDiscountCurve(
                 Natural settlementDays,
                 const Calendar& calendar,
                 const std::vector<boost::shared_ptr<BondHelper> >& bonds,
                 const DayCounter& dayCounter,
                 const FittingMethod& fittingMethod,
                 Real accuracy = 1.0e-10,
                 Size maxEvaluations = 10000,
                 const Array& guess = Array(),
                 Real simplexLambda = 1.0);
        //! curve reference date fixed for life of curve
        FittedBondDiscountCurve(
                 const Date &referenceDate,
                 const std::vector<boost::shared_ptr<BondHelper> >& bonds,
                 const DayCounter& dayCounter,
                 const FittingMethod& fittingMethod,
                 Real accuracy = 1.0e-10,
                 Size maxEvaluations = 10000,
                 const Array &guess = Array(),
                 Real simplexLambda = 1.0);
        //! reference date based on current evaluation date
        /*! \deprecated */
        QL_DEPRECATED
        FittedBondDiscountCurve(
                 Natural settlementDays,
                 const Calendar& calendar,
                 const std::vector<boost::shared_ptr<FixedRateBondHelper> >& bonds,
                 const DayCounter& dayCounter,
                 const FittingMethod& fittingMethod,
                 Real accuracy = 1.0e-10,
                 Size maxEvaluations = 10000,
                 const Array& guess = Array(),
                 Real simplexLambda = 1.0);
        //! curve reference date fixed for life of curve
        /*! \deprecated */
        QL_DEPRECATED
        FittedBondDiscountCurve(
                 const Date &referenceDate,
                 const std::vector<boost::shared_ptr<FixedRateBondHelper> >& bonds,
                 const DayCounter& dayCounter,
                 const FittingMethod& fittingMethod,
                 Real accuracy = 1.0e-10,
                 Size maxEvaluations = 10000,
                 const Array &guess = Array(),
                 Real simplexLambda = 1.0);
        //@}

        //! \name Inspectors
        //@{
        //! total number of bonds used to fit the yield curve
        Size numberOfBonds() const;
        //! the latest date for which the curve can return values
        Date maxDate() const;
        //! class holding the results of the fit
        const FittingMethod& fitResults() const;
        //@}

        //! \name Observer interface
        //@{
        void update();
        //@}

      private:
        void setup();
        void performCalculations() const;
        DiscountFactor discountImpl(Time) const;
        // target accuracy level to be used in the optimization routine
        Real accuracy_;
        // max number of evaluations to be used in the optimization routine
        Size maxEvaluations_;
        // sets the scale in the (Simplex) optimization routine
        Real simplexLambda_;
        // a guess solution may be passed into the constructor to speed calcs
        Array guessSolution_;
        mutable Date maxDate_;
        std::vector<boost::shared_ptr<BondHelper> > bondHelpers_;
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
        virtual ~FittingMethod() {}
        //! total number of coefficients to fit/solve for
        virtual Size size() const = 0;
        //! output array of results of optimization problem
        Array solution() const;
        //! final number of iterations used in the optimization problem
        Integer numberOfIterations() const;
        //! final value of cost function after optimization
        Real minimumCostValue() const;
        //! clone of the current object
        virtual std::auto_ptr<FittingMethod> clone() const = 0;
      protected:
        //! constructor
        FittingMethod(bool constrainAtZero = true);
        //! rerun every time instruments/referenceDate changes
        void init();
        //! derived classes must set this
        /*! user-defined discount curve, as a function of time and an
            array of unknown fitting coefficients \f$ x_i \f$.
        */
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
        boost::shared_ptr<FittingCost> costFunction_;
      private:
        // curve optimization called here- adjust optimization parameters here
        void calculate();
        // array of normalized (duration) weights, one for each bond helper
        Array weights_;
        // total number of iterations used in the optimization routine
        // (possibly including gradient evaluations)
        Integer numberOfIterations_;
        // final value for the minimized cost function
        Real costValue_;
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
        TermStructure::update();
        LazyObject::update();
    }

    inline void FittedBondDiscountCurve::setup() {
        for (Size i=0; i<bondHelpers_.size(); ++i)
            registerWith(bondHelpers_[i]);
    }

    inline DiscountFactor FittedBondDiscountCurve::discountImpl(Time t) const {
        calculate();
        return fittingMethod_->discountFunction(fittingMethod_->solution_, t);
    }

    inline Integer
    FittedBondDiscountCurve::FittingMethod::numberOfIterations() const {
        return numberOfIterations_;
    }

    inline
    Real FittedBondDiscountCurve::FittingMethod::minimumCostValue() const {
        return costValue_;
    }

    inline Array FittedBondDiscountCurve::FittingMethod::solution() const {
        return solution_;
    }

}

#endif
