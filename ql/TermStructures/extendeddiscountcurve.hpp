/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd).

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

/*! \file extendeddiscountcurve.hpp
    \brief discount factor structure with detailed compound-forward calculation
*/

#ifndef quantlib_extended_discount_curve_h
#define quantlib_extended_discount_curve_h

#include <ql/TermStructures/discountcurve.hpp>
#include <map>

namespace QuantLib {

    class CompoundForward;

    //! Term structure based on loglinear interpolation of discount factors
    /*! Loglinear interpolation guarantees piecewise constant forward rates.

        Rates are assumed to be annual continuos compounding.
    */
    class ExtendedDiscountCurve : public DiscountCurve {
      public:
        // constructor
        ExtendedDiscountCurve(const std::vector<Date>& dates,
                              const std::vector<DiscountFactor>& dfs,
                              const Calendar& calendar,
                              const BusinessDayConvention conv,
                              const DayCounter& dayCounter);
        Calendar calendar() const { return calendar_; };
        BusinessDayConvention businessDayConvention() const {
            return conv_;
        }
        void update();
        Rate compoundForward(const Date& d1,
                             Integer f,
                             bool extrapolate = false) const;
        Rate compoundForward(Time t1,
                             Integer f,
                             bool extrapolate = false) const;
      protected:
        /*! Returns the forward rate at a specified compound frequency
	    for the given date calculating it from the zero yield.
        */
        Rate compoundForwardImpl(Time, Integer) const;
        /*! Returns the zero yield rate for the given date calculating it
            from the discount.
        */
        Rate zeroYieldImpl(Time) const;
        void calibrateNodes() const;
        boost::shared_ptr<CompoundForward> reversebootstrap(Integer) const;
        boost::shared_ptr<CompoundForward> forwardCurve(Integer) const;
      private:
        Calendar calendar_;
        BusinessDayConvention conv_;
        mutable std::map<Integer,boost::shared_ptr<CompoundForward> >
                                                         forwardCurveMap_;
    };

    // inline definitions

    inline Rate ExtendedDiscountCurve::compoundForward(const Date& d,
                                                       Integer f,
                                                       bool extrapolate)
                                                                      const {
        Time t = timeFromReference(d);
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
        return compoundForwardImpl(timeFromReference(d),f);
    }

    inline Rate ExtendedDiscountCurve::compoundForward(Time t, Integer f,
                                                       bool extrapolate)
                                                                      const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                   << maxTime() << ")");
        return compoundForwardImpl(t,f);
    }

    inline void ExtendedDiscountCurve::update() {
        forwardCurveMap_.clear();
        DiscountCurve::update();
    }

}


#endif
