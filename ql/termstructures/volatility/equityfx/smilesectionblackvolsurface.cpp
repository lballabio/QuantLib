/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/termstructures/volatility/equityfx/smilesectionblackvolsurface.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    SmileSectionBlackVolSurface::SmileSectionBlackVolSurface(
            const Date& referenceDate,
            ext::shared_ptr<SmileSection> smile,
            const DayCounter& dc,
            const Calendar& cal,
            BusinessDayConvention bdc)
    : BlackVolatilityTermStructure(referenceDate, cal, bdc, dc) {
        smiles_.push_back(std::move(smile));
        expiryTimes_.push_back(smiles_.front()->exerciseTime());
        registerWith(smiles_.front());
    }

    SmileSectionBlackVolSurface::SmileSectionBlackVolSurface(
            const Date& referenceDate,
            std::vector<ext::shared_ptr<SmileSection>> smiles,
            const DayCounter& dc,
            const Calendar& cal,
            BusinessDayConvention bdc)
    : BlackVolatilityTermStructure(referenceDate, cal, bdc, dc),
      smiles_(std::move(smiles)) {
        QL_REQUIRE(!smiles_.empty(),
                   "at least one SmileSection is required");

        // Sort by expiry time
        std::sort(smiles_.begin(), smiles_.end(),
                  [](const ext::shared_ptr<SmileSection>& a,
                     const ext::shared_ptr<SmileSection>& b) {
                      return a->exerciseTime() < b->exerciseTime();
                  });

        expiryTimes_.reserve(smiles_.size());
        for (const auto& s : smiles_) {
            expiryTimes_.push_back(s->exerciseTime());
            registerWith(s);
        }
    }

    Date SmileSectionBlackVolSurface::maxDate() const {
        return Date::maxDate();
    }

    Real SmileSectionBlackVolSurface::minStrike() const {
        return QL_MIN_REAL;
    }

    Real SmileSectionBlackVolSurface::maxStrike() const {
        return QL_MAX_REAL;
    }

    void SmileSectionBlackVolSurface::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<SmileSectionBlackVolSurface>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVolatilityTermStructure::accept(v);
    }

    Volatility SmileSectionBlackVolSurface::blackVolImpl(Time t, Real strike) const {
        if (smiles_.size() == 1) {
            // Flat forward variance: vol is time-independent
            return smiles_.front()->volatility(strike);
        }

        // Before first expiry: use first smile
        if (t <= expiryTimes_.front())
            return smiles_.front()->volatility(strike);

        // After last expiry: use last smile
        if (t >= expiryTimes_.back())
            return smiles_.back()->volatility(strike);

        // Find bracketing expiries
        auto it = std::lower_bound(expiryTimes_.begin(), expiryTimes_.end(), t);
        Size i = std::distance(expiryTimes_.begin(), it) - 1;

        Time t1 = expiryTimes_[i];
        Time t2 = expiryTimes_[i + 1];

        Volatility sigma1 = smiles_[i]->volatility(strike);
        Volatility sigma2 = smiles_[i + 1]->volatility(strike);

        // Linear interpolation in total variance space
        Real w1 = sigma1 * sigma1 * t1;
        Real w2 = sigma2 * sigma2 * t2;
        Real alpha = (t - t1) / (t2 - t1);
        Real w = w1 + alpha * (w2 - w1);

        return w > 0.0 && t > 0.0 ? std::sqrt(w / t) : sigma1;
    }

}
