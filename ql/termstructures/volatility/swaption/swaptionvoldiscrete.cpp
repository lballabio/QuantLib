/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

#include <ql/termstructures/volatility/swaption/swaptionvoldiscrete.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    SwaptionVolatilityDiscrete::SwaptionVolatilityDiscrete(
                                    const std::vector<Period>& optionTenors,
                                    const std::vector<Period>& swapTenors,
                                    Natural settlementDays,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(settlementDays, cal, dc, bdc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      optionDatesAsReal_(nOptionTenors_),
      nSwapTenors_(swapTenors.size()),
      swapTenors_(swapTenors),
      swapLengths_(nSwapTenors_) {

        checkOptionTenors();
        initializeOptionDatesAndTimes();

        checkSwapTenors();

        optionInterpolator_= LinearInterpolation(optionTimes_.begin(),
                                                 optionTimes_.end(),
                                                 optionDatesAsReal_.begin());
        optionInterpolator_.update();
        optionInterpolator_.enableExtrapolation();

        registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
    }

    SwaptionVolatilityDiscrete::SwaptionVolatilityDiscrete(
        const std::vector<Period>& optionTenors,
        const std::vector<Period>& swapTenors,
        const Date& referenceDate,
        const Calendar& cal,
        const DayCounter& dc,
        BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(referenceDate, cal, dc, bdc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      optionDatesAsReal_(nOptionTenors_),
      nSwapTenors_(swapTenors.size()),
      swapTenors_(swapTenors),
      swapLengths_(nSwapTenors_) {

        checkOptionTenors();
        initializeOptionDatesAndTimes();

        checkSwapTenors();

        optionInterpolator_= LinearInterpolation(optionTimes_.begin(),
                                                 optionTimes_.end(),
                                                 optionDatesAsReal_.begin());
        optionInterpolator_.update();
        optionInterpolator_.enableExtrapolation();
    }

    SwaptionVolatilityDiscrete::SwaptionVolatilityDiscrete(
        const std::vector<Date>& optionDates,
        const std::vector<Period>& swapTenors,
        const Date& referenceDate,
        const Calendar& cal,
        const DayCounter& dc,
        BusinessDayConvention bdc)
    : SwaptionVolatilityStructure(referenceDate, cal, dc, bdc),
      nOptionTenors_(optionDates.size()),
      optionTenors_(nOptionTenors_),
      optionDates_(optionDates),
      optionTimes_(nOptionTenors_),
      optionDatesAsReal_(nOptionTenors_),
      nSwapTenors_(swapTenors.size()),
      swapTenors_(swapTenors),
      swapLengths_(nSwapTenors_) {

        checkOptionDates();
        initializeOptionTimes();

        checkSwapTenors();

        optionInterpolator_= LinearInterpolation(optionTimes_.begin(),
                                                 optionTimes_.end(),
                                                 optionDatesAsReal_.begin());
        optionInterpolator_.update();
        optionInterpolator_.enableExtrapolation();
    }

    void SwaptionVolatilityDiscrete::checkOptionDates() const {
        QL_REQUIRE(optionDates_[0]>=referenceDate(),
                   "first option date (" << optionDates_[0] <<
                   ") is after reference date (" << referenceDate() << ")");
        for (Size i=1; i<nOptionTenors_; ++i) {
            QL_REQUIRE(optionDates_[i]>optionDates_[i-1],
                       "non increasing option dates: " << io::ordinal(i-1) <<
                       " is " << optionDates_[i-1] << ", " << io::ordinal(i) <<
                       " is " << optionDates_[i]);
        }
    }

    void SwaptionVolatilityDiscrete::checkOptionTenors() const {
        Date rollingExDate = optionDateFromTenor(optionTenors_[0]);
        QL_REQUIRE(rollingExDate>=referenceDate(),
                   "first option tenor is negative ("
                   << optionTenors_[0] << ")");
        for (Size i=1; i<nOptionTenors_; ++i) {
            QL_REQUIRE(optionDateFromTenor(optionTenors_[i])>rollingExDate,
                       "non increasing option tenor: " << io::ordinal(i-1) <<
                       " is " << optionTenors_[i-1] << ", " << io::ordinal(i) <<
                       " is " << optionTenors_[i]);
            rollingExDate = optionDateFromTenor(optionTenors_[i]);
        }
    }

    void SwaptionVolatilityDiscrete::checkSwapTenors() const {
        Date startDate = referenceDate();
        /* better than an any option date, for coherence between swaption atm vol matrix
           and swaption vol cube:
           otherwise, if both swaption atm vol matrix and swaption vol cube are created
           and they have different first option date, the swap tenors of the structures may
           differ
        */
        // Date startDate = optionDates_[0]; // as good as any
        Date endDate = startDate + swapTenors_[0];
        QL_REQUIRE(endDate>startDate, "first swap tenor is negative ("  << swapTenors_[0] << ")");
        for (Size i=1; i<nSwapTenors_; ++i) {
            QL_REQUIRE((startDate+swapTenors_[i])>endDate,
                       "non increasing swap tenor: " << io::ordinal(i-1) <<
                       " is " << swapTenors_[i-1] << ", " << io::ordinal(i) <<
                       " is " << swapTenors_[i]);
            Date endDate = startDate + swapTenors_[i];
        }

    }

    void SwaptionVolatilityDiscrete::initializeOptionDatesAndTimes() const {
        for (Size i=0; i<nOptionTenors_; ++i) {
            optionDates_[i] = optionDateFromTenor(optionTenors_[i]);
            optionDatesAsReal_[i] =
                static_cast<Real>(optionDates_[i].serialNumber());
        }
        initializeOptionTimes();
    }

    void SwaptionVolatilityDiscrete::initializeOptionTimes() const {
        for (Size i=0; i<nOptionTenors_; ++i)
            optionTimes_[i] = timeFromReference(optionDates_[i]);
            //optionTimes_[i] = dayCounter().yearFraction(referenceDate(), optionDates_[i]);

        Date startDate = referenceDate();
        //Date startDate = optionDates_[0]; // as good as any
        for (Size i=0; i<nSwapTenors_; ++i) {
            Date endDate = startDate + swapTenors_[i];
            swapLengths_[i] = dayCounter().yearFraction(startDate, endDate);
        }
    }

    void SwaptionVolatilityDiscrete::performCalculations() const {
         // check if date recalculation could be avoided here
         if (moving_)
            initializeOptionDatesAndTimes();
    }

    void SwaptionVolatilityDiscrete::update() {
        // recalculate dates if necessary...
        if (moving_) {
            Date d = Settings::instance().evaluationDate();
            if (evaluationDate_ != d) {
                evaluationDate_ = d;
                initializeOptionDatesAndTimes();
            }
        }
        TermStructure::update();
        LazyObject::update();
    }

}
