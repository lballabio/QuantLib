/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
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

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <utility>


namespace QuantLib {

    namespace {
        Date time2Date(const Date referenceDate, const DayCounter& dc, Time t) {
            t-=1e4*QL_EPSILON; // add a small buffer for rounding errors
            Date d(referenceDate);
            while(dc.yearFraction(referenceDate, d+=Period(1, Years)) < t);
            d-=Period(1, Years);
            while(dc.yearFraction(referenceDate, d+=Period(1, Months)) < t);
            d-=Period(1, Months);
            while(dc.yearFraction(referenceDate, d++) < t);
            return d;
        }
    }

    FixedLocalVolSurface::FixedLocalVolSurface(const Date& referenceDate,
                                               const std::vector<Date>& dates,
                                               const std::vector<Real>& strikes,
                                               ext::shared_ptr<Matrix> localVolMatrix,
                                               const DayCounter& dayCounter,
                                               Extrapolation lowerExtrapolation,
                                               Extrapolation upperExtrapolation)
    : LocalVolTermStructure(referenceDate, NullCalendar(), Following, dayCounter),
      maxDate_(dates.back()), localVolMatrix_(std::move(localVolMatrix)),
      strikes_(dates.size(), ext::make_shared<std::vector<Real> >(strikes)),
      localVolInterpol_(dates.size()), lowerExtrapolation_(lowerExtrapolation),
      upperExtrapolation_(upperExtrapolation) {

        QL_REQUIRE(dates[0]>=referenceDate,
                   "cannot have dates[0] < referenceDate");

        times_ = std::vector<Time>(dates.size());
        for (Size j=0; j<times_.size(); j++)
            times_[j] = timeFromReference(dates[j]);

        checkSurface();
        setInterpolation<Linear>();
    }

    FixedLocalVolSurface::FixedLocalVolSurface(const Date& referenceDate,
                                               const std::vector<Time>& times,
                                               const std::vector<Real>& strikes,
                                               ext::shared_ptr<Matrix> localVolMatrix,
                                               const DayCounter& dayCounter,
                                               Extrapolation lowerExtrapolation,
                                               Extrapolation upperExtrapolation)
    : LocalVolTermStructure(referenceDate, NullCalendar(), Following, dayCounter),
      maxDate_(time2Date(referenceDate, dayCounter, times.back())), times_(times),
      localVolMatrix_(std::move(localVolMatrix)),
      strikes_(times.size(), ext::make_shared<std::vector<Real> >(strikes)),
      localVolInterpol_(times.size()), lowerExtrapolation_(lowerExtrapolation),
      upperExtrapolation_(upperExtrapolation) {

        QL_REQUIRE(times_[0]>=0, "cannot have times[0] < 0");

        checkSurface();
        setInterpolation<Linear>();
    }

    FixedLocalVolSurface::FixedLocalVolSurface(
        const Date& referenceDate,
        const std::vector<Time>& times,
        const std::vector<ext::shared_ptr<std::vector<Real> > >& strikes,
        ext::shared_ptr<Matrix> localVolMatrix,
        const DayCounter& dayCounter,
        Extrapolation lowerExtrapolation,
        Extrapolation upperExtrapolation)
    : LocalVolTermStructure(referenceDate, NullCalendar(), Following, dayCounter),
      maxDate_(time2Date(referenceDate, dayCounter, times.back())), times_(times),
      localVolMatrix_(std::move(localVolMatrix)), strikes_(strikes),
      localVolInterpol_(times.size()), lowerExtrapolation_(lowerExtrapolation),
      upperExtrapolation_(upperExtrapolation) {

        QL_REQUIRE(times_[0]>=0, "cannot have times[0] < 0");
        QL_REQUIRE(times.size() == strikes.size(),
             "need strikes for every time step");
        checkSurface();
        setInterpolation<Linear>();
    }


    void FixedLocalVolSurface::checkSurface() {
        QL_REQUIRE(times_.size()==localVolMatrix_->columns(),
                   "mismatch between date vector and vol matrix colums");
        for (const auto& strike : strikes_) {
            QL_REQUIRE(strike->size() == localVolMatrix_->rows(),
                       "mismatch between money-strike vector and "
                       "vol matrix rows");
        }

        for (Size j=1; j<times_.size(); j++) {
            QL_REQUIRE(times_[j]>times_[j-1],
                       "dates must be sorted unique!");
        }

        for (const auto& strike : strikes_)
            for (Size j = 1; j < strike->size(); j++) {
                QL_REQUIRE((*strike)[j] >= (*strike)[j - 1], "strikes must be sorted");
            }
    }

    Date FixedLocalVolSurface::maxDate() const {
        return maxDate_;
    }
    Time FixedLocalVolSurface::maxTime() const {
        return times_.back();
    }
    Real FixedLocalVolSurface::minStrike() const {
        return strikes_.back()->front();
    }
    Real FixedLocalVolSurface::maxStrike() const {
        return strikes_.back()->back();
    }

    Volatility FixedLocalVolSurface::localVolImpl(Time t, Real strike) const {
        t = std::min(times_.back(), std::max(t, times_.front()));

        const Size idx = std::distance(times_.begin(),
            std::lower_bound(times_.begin(), times_.end(), t));

        if (close_enough(t, times_[idx])) {
            if (strikes_[idx]->front() < strikes_[idx]->back())
                return localVolInterpol_[idx](strike, true);
            else
                return (*localVolMatrix_)[localVolMatrix_->rows()/2][idx];
        }
        else {
            Real earlierStrike = strike, laterStrike = strike;
            if (lowerExtrapolation_ == ConstantExtrapolation) {
                if (strike < strikes_[idx-1]->front())
                    earlierStrike = strikes_[idx-1]->front();
                if (strike < strikes_[idx]->front())
                    laterStrike = strikes_[idx]->front();
            }

            if (upperExtrapolation_ == ConstantExtrapolation) {
                if (strike > strikes_[idx-1]->back())
                    earlierStrike = strikes_[idx-1]->back();
                if (strike > strikes_[idx]->back())
                    laterStrike = strikes_[idx]->back();
            }

            const Real earlyVol =
                (strikes_[idx-1]->front() < strikes_[idx-1]->back())
                ? localVolInterpol_[idx-1](earlierStrike, true)
                : (*localVolMatrix_)[localVolMatrix_->rows()/2][idx-1];
            const Real laterVol = localVolInterpol_[idx](laterStrike, true);

            return earlyVol
                    + (laterVol-earlyVol)/(times_[idx]-times_[idx-1])
                      *(t-times_[idx-1]);
        }
    }

}
