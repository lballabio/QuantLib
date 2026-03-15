/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Rich Amaya
 Copyright (C) 2026 Yassine Idyiahia

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/termstructures/volatility/equityfx/piecewiseblackvariancesurface.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/interpolatedsmilesection.hpp>
#include <ql/utilities/null.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace QuantLib {

    PiecewiseBlackVarianceSurface::PiecewiseBlackVarianceSurface(
            const Date& referenceDate,
            const std::vector<Date>& dates,
            std::vector<ext::shared_ptr<SmileSection>> smileSections,
            DayCounter dayCounter)
    : BlackVarianceTermStructure(referenceDate),
      dayCounter_(std::move(dayCounter)),
      smileSections_(std::move(smileSections)) {

        QL_REQUIRE(!dates.empty(),
                   "at least one date is required");
        QL_REQUIRE(dates.size() == smileSections_.size(),
                   "mismatch between " << dates.size() << " dates and "
                   << smileSections_.size() << " smile sections");

        maxDate_ = dates.back();
        times_.resize(dates.size());

        times_[0] = timeFromReference(dates[0]);
        QL_REQUIRE(times_[0] > 0.0,
                   "first date (" << dates[0]
                   << ") must be after reference date ("
                   << referenceDate << ")");

        for (Size i = 1; i < dates.size(); ++i) {
            times_[i] = timeFromReference(dates[i]);
            QL_REQUIRE(times_[i] > times_[i-1],
                       "dates must be sorted and unique, but date "
                       << dates[i] << " (t=" << times_[i]
                       << ") is not after date " << dates[i-1]
                       << " (t=" << times_[i-1] << ")");
        }

        for (Size i = 0; i < smileSections_.size(); ++i) {
            QL_REQUIRE(smileSections_[i],
                       "null smile section at index " << i);
            registerWith(smileSections_[i]);
        }
    }

    PiecewiseBlackVarianceSurface::PiecewiseBlackVarianceSurface(
            const Date& referenceDate,
            const Date& date,
            ext::shared_ptr<SmileSection> smileSection,
            DayCounter dayCounter)
    : PiecewiseBlackVarianceSurface(
          referenceDate,
          std::vector<Date>{date},
          std::vector<ext::shared_ptr<SmileSection>>{std::move(smileSection)},
          std::move(dayCounter)) {}

    Real PiecewiseBlackVarianceSurface::sectionVariance(
            Size i, Real strike) const {
        const auto& s = smileSections_[i];
        QL_REQUIRE(allowsExtrapolation() ||
                   (strike >= s->minStrike() && strike <= s->maxStrike()),
                   "strike (" << strike
                   << ") is outside the range of smile section "
                   << i << " [" << s->minStrike() << ", "
                   << s->maxStrike() << "]");
        return s->variance(strike);
    }

    Real PiecewiseBlackVarianceSurface::blackVarianceImpl(
            Time t, Real strike) const {

        if (t == 0.0)
            return 0.0;

        if (t <= times_.front()) {
            // linear interpolation from (0, 0) to first tenor
            Real var1 = sectionVariance(0, strike);
            return var1 * t / times_.front();
        }

        if (t >= times_.back()) {
            // flat vol extrapolation beyond last tenor
            Real varN = sectionVariance(smileSections_.size() - 1, strike);
            return varN * t / times_.back();
        }

        // find enclosing interval
        auto it = std::upper_bound(times_.begin(), times_.end(), t);
        Size hi = std::distance(times_.begin(), it);
        Size lo = hi - 1;

        Real varLo = sectionVariance(lo, strike);
        Real varHi = sectionVariance(hi, strike);
        Real alpha = (t - times_[lo]) / (times_[hi] - times_[lo]);

        return varLo + (varHi - varLo) * alpha;
    }

    ext::shared_ptr<PiecewiseBlackVarianceSurface>
    PiecewiseBlackVarianceSurface::makeFromGrid(
            const Date& referenceDate,
            const std::vector<Date>& dates,
            const std::vector<Real>& strikes,
            const Matrix& blackVols,
            const DayCounter& dc) {

        QL_REQUIRE(blackVols.rows() == strikes.size(),
                   "mismatch between " << strikes.size() << " strikes and "
                   << blackVols.rows() << " matrix rows");
        QL_REQUIRE(blackVols.columns() == dates.size(),
                   "mismatch between " << dates.size() << " dates and "
                   << blackVols.columns() << " matrix columns");

        std::vector<ext::shared_ptr<SmileSection>> sections(dates.size());

        for (Size j = 0; j < dates.size(); ++j) {
            std::vector<Real> stdDevs(strikes.size());
            Time t = dc.yearFraction(referenceDate, dates[j]);
            QL_REQUIRE(t > 0.0,
                       "date " << dates[j]
                       << " must be after reference date "
                       << referenceDate);
            Real sqrtT = std::sqrt(t);
            for (Size i = 0; i < strikes.size(); ++i)
                stdDevs[i] = blackVols[i][j] * sqrtT;

            sections[j] = ext::make_shared<InterpolatedSmileSection<Linear>>(
                dates[j], strikes, stdDevs, Null<Real>(),
                dc, Linear(), referenceDate);
        }

        return ext::make_shared<PiecewiseBlackVarianceSurface>(
            referenceDate, dates, std::move(sections), dc);
    }

    ext::shared_ptr<SmileSection>
    PiecewiseBlackVarianceSurface::smileSectionImpl(Time t) const {
        auto it = std::lower_bound(times_.begin(), times_.end(), t);
        if (it != times_.end() && close_enough(t, *it))
            return smileSections_[std::distance(times_.begin(), it)];
        return BlackVarianceTermStructure::smileSectionImpl(t);
    }

    void PiecewiseBlackVarianceSurface::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<PiecewiseBlackVarianceSurface>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }

}
