/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

#include <iostream>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>

namespace QuantLib {

	namespace {
		Date time2Date(const Date referenceDate, const DayCounter& dc, Time t) {
			Date d(referenceDate);
			while(dc.yearFraction(referenceDate, d+=Period(1, Years)) < t);
			d-=Period(1, Years);
			while(dc.yearFraction(referenceDate, d+=Period(1, Months)) < t);
			d-=Period(1, Months);
			while(dc.yearFraction(referenceDate, d++) < t);
			return d;
		}
	}

	FixedLocalVolSurface::FixedLocalVolSurface(
		const Date& referenceDate,
		const std::vector<Date>& dates,
		const std::vector<Real>& strikes,
		const Matrix& localVolMatrix,
		const DayCounter& dayCounter,
		Extrapolation lowerExtrapolation,
		Extrapolation upperExtrapolation)
	: LocalVolTermStructure(referenceDate, NullCalendar(),
							Following, dayCounter),
	  maxDate_(dates.back()),
	  minDate_(dates.front()),
	  strikes_(strikes),
	  localVolMatrix_(localVolMatrix),
      lowerExtrapolation_(lowerExtrapolation),
	  upperExtrapolation_(upperExtrapolation) {

        QL_REQUIRE(dates[0]>=referenceDate,
                   "cannot have dates[0] < referenceDate");

        times_ = std::vector<Time>(dates.size());
        for (Size j=0; j<times_.size(); j++)
            times_[j] = timeFromReference(dates[j]);

        checkSurface();
//        setInterpolation<Bicubic>();
        setInterpolation<Bilinear>();
	}

	FixedLocalVolSurface::FixedLocalVolSurface(
    	const Date& referenceDate,
		const std::vector<Time>& times,
		const std::vector<Real>& strikes,
		const Matrix& localVolMatrix,
		const DayCounter& dayCounter,
		Extrapolation lowerExtrapolation,
		Extrapolation upperExtrapolation)
	: LocalVolTermStructure(referenceDate, NullCalendar(),
							Following, dayCounter),
	  maxDate_(time2Date(referenceDate, dayCounter, times.back())),
	  minDate_(time2Date(referenceDate, dayCounter, times.front())),
	  strikes_(strikes),
	  localVolMatrix_(localVolMatrix),
	  times_(times),
      lowerExtrapolation_(lowerExtrapolation),
	  upperExtrapolation_(upperExtrapolation) {

        QL_REQUIRE(times_[0]>=0, "cannot have times[0] < 0");

        checkSurface();
        setInterpolation<Bicubic>();
	}

	void FixedLocalVolSurface::checkSurface() {
        QL_REQUIRE(times_.size()==localVolMatrix_.columns(),
                   "mismatch between date vector and vol matrix colums");
        QL_REQUIRE(strikes_.size()==localVolMatrix_.rows(),
                   "mismatch between money-strike vector and vol matrix rows");

        for (Size j=1; j<times_.size(); j++) {
            QL_REQUIRE(times_[j]>times_[j-1],
                       "dates must be sorted unique!");
        }
        for (Size j=1; j<strikes_.size(); j++) {
            QL_REQUIRE(strikes_[j]>strikes_[j-1],
                       "strikes must be sorted unique!");
        }
	}

    Date FixedLocalVolSurface::maxDate() const {
    	return maxDate_;
    }
    Date FixedLocalVolSurface::minDate() const {
    	return minDate_;
    }
    Time FixedLocalVolSurface::minTime() const {
    	return times_.front();
    }
    Time FixedLocalVolSurface::maxTime() const {
    	return times_.back();
    }
    Real FixedLocalVolSurface::minStrike() const {
    	return strikes_.front();
    }
    Real FixedLocalVolSurface::maxStrike() const {
    	return strikes_.back();
    }

    Volatility FixedLocalVolSurface::localVolImpl(Time t, Real strike) const {

        if (strike < strikes_.front()
            && lowerExtrapolation_ == ConstantExtrapolation)
            strike = strikes_.front();
        if (strike > strikes_.back()
            && upperExtrapolation_ == ConstantExtrapolation)
            strike = strikes_.back();

        t = std::min(times_.back(), std::max(t, times_.front()));

        return localVolSurface_(t, strike, true);
    }
}
