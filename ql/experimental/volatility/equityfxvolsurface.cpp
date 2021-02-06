/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/experimental/volatility/equityfxvolsurface.hpp>

namespace QuantLib {

    EquityFXVolSurface::EquityFXVolSurface(BusinessDayConvention bdc,
                                           const DayCounter& dc)
    : BlackVolSurface(bdc, dc) {}

    EquityFXVolSurface::EquityFXVolSurface(const Date& refDate,
                                           const Calendar& cal,
                                           BusinessDayConvention bdc,
                                           const DayCounter& dc)
    : BlackVolSurface(refDate, cal, bdc, dc) {}

    EquityFXVolSurface::EquityFXVolSurface(Natural settlDays,
                                           const Calendar& cal,
                                           BusinessDayConvention bdc,
                                           const DayCounter& dc)
    : BlackVolSurface(settlDays, cal, bdc, dc) {}

    Volatility EquityFXVolSurface::atmForwardVol(const Date& date1,
                                                 const Date& date2,
                                                 bool extrapolate) const {
        QL_REQUIRE(date1<date2, "wrong dates");
        Time t1 = timeFromReference(date1);
        Time t2 = timeFromReference(date2);
        return atmForwardVol(t1, t2, extrapolate);
    }

    Volatility EquityFXVolSurface::atmForwardVol(Time time1,
                                                 Time time2,
                                                 bool extrapolate) const {
        Real fwdVariance = atmForwardVariance(time1, time2, extrapolate);
        Time t = time2-time1;
        return std::sqrt(fwdVariance/t);
    }

    Real EquityFXVolSurface::atmForwardVariance(const Date& date1,
                                                const Date& date2,
                                                bool extrapolate) const {
        QL_REQUIRE(date1<date2, "wrong dates");
        Time t1 = timeFromReference(date1);
        Time t2 = timeFromReference(date2);
        return atmForwardVariance(t1, t2, extrapolate);
    }

    Real EquityFXVolSurface::atmForwardVariance(Time time1,
                                                Time time2,
                                                bool extrapolate) const {
        QL_REQUIRE(time1<time2, "wrong times");
        Real var1 = atmVariance(time1, extrapolate);
        Real var2 = atmVariance(time2, extrapolate);
        QL_ENSURE(var1<var2, "non-increasing variances");
        return var2-var1;
    }

    void EquityFXVolSurface::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<EquityFXVolSurface>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            QL_FAIL("not a EquityFXVolSurface term structure visitor");
    }

}
