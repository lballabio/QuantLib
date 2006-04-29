/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file piecewiseflatforward.hpp
    \brief piecewise flat forward term structure
*/

#ifndef quantlib_piecewise_flat_forward_curve_hpp
#define quantlib_piecewise_flat_forward_curve_hpp

#include <ql/qldefines.hpp>
#include <ql/Math/cubicspline.hpp>

#if !defined(QL_PATCH_MSVC6)

#include <ql/TermStructures/piecewiseyieldcurve.hpp>

namespace QuantLib {

    //! Piecewise flat-forward term structure
    /*! \ingroup yieldtermstructures */
    typedef PiecewiseYieldCurve<Discount,LogLinear> PiecewiseFlatForward;

    typedef PiecewiseYieldCurve<ForwardRate,Cubic
        //(CubicSpline::SecondDerivative, 0.0,
        // CubicSpline::FirstDerivative, 0.0,
        // true)
    > PiecewiseMonotoneForward;

}

#else

#include <ql/ratehelper.hpp>

namespace QuantLib {

    //! Piecewise flat forward term structure
    /*! This term structure is bootstrapped on a number of interest
        rate instruments which are passed as a vector of handles to
        RateHelper instances. Their maturities mark the boundaries of
        the flat forward segments.

        The values of the forward rates for each segment are determined
        sequentially starting from the earliest period to the latest.

        The value for each segment is chosen so that the instrument whose
        maturity marks the end of such segment is correctly repriced on
        the curve.

        Rates are assumed to be annual continuos compounding.

        \warning The bootstrapping algorithm will raise an exception if
        any two instruments have the same maturity date.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against the original inputs.
        - the observability of the term structure is tested.
    */
    class PiecewiseFlatForward : public YieldTermStructure,
                                 public LazyObject {
      public:
        //! \name Constructors
        //@{
        PiecewiseFlatForward(
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy = 1.0e-12);
        PiecewiseFlatForward(
               Integer settlementDays, const Calendar& calendar,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy = 1.0e-12);
        /*! In this constructor, the first date must be the reference
            date of the curve, the other dates are the nodes of the
            term structure. The forward rate at index \f$i\f$ is used
            in the period \f$t_{i-1} < t \le t_i\f$. Therefore,
            forwards[0] is used only to compute the zero yield for
            \f$t = 0\f$.

            \deprecated use ForwardCurve instead
        */
        PiecewiseFlatForward(const std::vector<Date>& dates,
                             const std::vector<Rate>& forwards,
                             const DayCounter& dayCounter);
        //@}
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        const std::vector<Date>& dates() const;
        Date maxDate() const;
        const std::vector<Time>& times() const;
        Time maxTime() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        /* We implement all calculation methods in order to take advantage
           of its own internal structure. This is also the reason why we
           derive directly from YieldTermStructure. */
        Rate zeroYieldImpl(Time) const;
        DiscountFactor discountImpl(Time) const;
        Rate forwardImpl(Time) const;
      private:
        // helper class for bootstrapping
        class FFObjFunction;
        friend class FFObjFunction;
        // methods
        Size referenceNode(Time t) const;
        void checkInstruments();
        void performCalculations() const;
        // data members
        DayCounter dayCounter_;
        std::vector<boost::shared_ptr<RateHelper> > instruments_;
        mutable std::vector<Time> times_;
        mutable std::vector<Date> dates_;
        mutable std::vector<DiscountFactor> discounts_;
        mutable std::vector<Rate> forwards_, zeroYields_;
        Real accuracy_;
    };


    // inline definitions

    inline const std::vector<Date>& PiecewiseFlatForward::dates() const {
        calculate();
        return dates_;
    }

    inline Date PiecewiseFlatForward::maxDate() const {
        calculate();
        return dates_.back();
    }

    inline const std::vector<Time>& PiecewiseFlatForward::times() const {
        calculate();
        return times_;
    }

    inline Time PiecewiseFlatForward::maxTime() const {
        calculate();
        return times_.back();
    }

    inline void PiecewiseFlatForward::update() {
        YieldTermStructure::update();
        LazyObject::update();
    }

}

#endif


#endif
