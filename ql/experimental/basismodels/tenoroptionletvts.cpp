/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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

/*! \file tenoroptionletvts.cpp
    \brief caplet volatility term structure based on volatility transformation
*/

#include <ql/exercise.hpp>
#include <ql/experimental/basismodels/tenoroptionletvts.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/math/rounding.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/time/schedule.hpp>
#include <utility>


namespace QuantLib {

    TenorOptionletVTS::TenorOptionletVTS(const Handle<OptionletVolatilityStructure>& baseVTS,
                                         std::shared_ptr<IborIndex> baseIndex,
                                         std::shared_ptr<IborIndex> targIndex,
                                         std::shared_ptr<CorrelationStructure> correlation)
    : OptionletVolatilityStructure(baseVTS->referenceDate(),
                                   baseVTS->calendar(),
                                   baseVTS->businessDayConvention(),
                                   baseVTS->dayCounter()),
      baseVTS_(baseVTS), baseIndex_(std::move(baseIndex)), targIndex_(std::move(targIndex)),
      correlation_(std::move(correlation)) {
        QL_REQUIRE(baseIndex_->tenor().frequency() % targIndex_->tenor().frequency() == 0,
                   "Base index frequency must be a multiple of target tenor frequency");
    }


    TenorOptionletVTS::TenorOptionletSmileSection::TenorOptionletSmileSection(
        const TenorOptionletVTS& volTS, const Time optionTime)
    : SmileSection(optionTime, volTS.baseVTS_->dayCounter(), Normal, 0.0),
      correlation_(volTS.correlation_) {
        // we assume that long (target) tenor is a multiple of short (base) tenor
        // first we need the long tenor start and end date
        Real oneDayAsYear =
            volTS.dayCounter().yearFraction(volTS.referenceDate(), volTS.referenceDate() + 1);
        Date exerciseDate =
            volTS.referenceDate() + ((BigInteger)ClosestRounding(0)(optionTime / oneDayAsYear));
        Date effectiveDate = volTS.baseIndex_->fixingCalendar().advance(
            exerciseDate, volTS.baseIndex_->fixingDays() * Days);
        Date maturityDate = volTS.baseIndex_->fixingCalendar().advance(
            effectiveDate, volTS.targIndex_->tenor(), Unadjusted, false);
        // now we can set up the short tenor schedule
        Schedule baseFloatSchedule(effectiveDate, maturityDate, volTS.baseIndex_->tenor(),
                                   volTS.baseIndex_->fixingCalendar(), ModifiedFollowing,
                                   Unadjusted, DateGeneration::Backward, false);
        // set up scalar attributes
        fraRateTarg_ = volTS.targIndex_->fixing(exerciseDate);
        Time yfTarg = volTS.targIndex_->dayCounter().yearFraction(effectiveDate, maturityDate);
        for (Size k = 0; k < baseFloatSchedule.dates().size() - 1; ++k) {
            Date startDate = baseFloatSchedule.dates()[k];
            Date fixingDate = volTS.baseIndex_->fixingCalendar().advance(
                startDate, (-1 * volTS.baseIndex_->fixingDays()) * Days);
            Time yearFrac = volTS.baseIndex_->dayCounter().yearFraction(
                baseFloatSchedule.dates()[k], baseFloatSchedule.dates()[k + 1]);
            // set up vector attributes
            baseSmileSection_.push_back(volTS.baseVTS_->smileSection(fixingDate, true));
            startTimeBase_.push_back(
                volTS.dayCounter().yearFraction(volTS.referenceDate(), startDate));
            fraRateBase_.push_back(volTS.baseIndex_->fixing(fixingDate));
            v_.push_back(yearFrac / yfTarg * (1.0 + yfTarg * fraRateTarg_) /
                         (1.0 + yearFrac * fraRateBase_[k]));
        }
    }

    Volatility TenorOptionletVTS::TenorOptionletSmileSection::volatilityImpl(Rate strike) const {
        Real sum_v = 0.0;
        for (Real k : v_)
            sum_v += k;
        std::vector<Real> volBase(v_.size());
        for (Size k = 0; k < fraRateBase_.size(); ++k) {
            Real strike_k = (strike - (fraRateTarg_ - sum_v * fraRateBase_[k])) / sum_v;
            volBase[k] = baseSmileSection_[k]->volatility(strike_k, Normal, 0.0);
        }
        Real var = 0.0;
        for (Size i = 0; i < volBase.size(); ++i) {
            var += v_[i] * v_[i] * volBase[i] * volBase[i];
            for (Size j = i + 1; j < volBase.size(); ++j) {
                Real corr = (*correlation_)(startTimeBase_[i], startTimeBase_[j]);
                var += 2.0 * corr * v_[i] * v_[j] * volBase[i] * volBase[j];
            }
        }
        Real vol = sqrt(var);
        return vol;
    }


}
