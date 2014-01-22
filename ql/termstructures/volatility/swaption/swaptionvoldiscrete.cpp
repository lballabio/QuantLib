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
                                    BusinessDayConvention bdc,
                                    const DayCounter& dc)
    : SwaptionVolatilityStructure(settlementDays, cal, bdc, dc),
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
        initializeSwapLengths();

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
                                    BusinessDayConvention bdc,
                                    const DayCounter& dc)
    : SwaptionVolatilityStructure(referenceDate, cal, bdc, dc),
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
        initializeSwapLengths();

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
                                        BusinessDayConvention bdc,
                                        const DayCounter& dc)
    : SwaptionVolatilityStructure(referenceDate, cal, bdc, dc),
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
        initializeSwapLengths();

        optionInterpolator_= LinearInterpolation(optionTimes_.begin(),
                                                 optionTimes_.end(),
                                                 optionDatesAsReal_.begin());
        optionInterpolator_.update();
        optionInterpolator_.enableExtrapolation();
    }

    void SwaptionVolatilityDiscrete::checkOptionDates() const {
        QL_REQUIRE(optionDates_[0]>referenceDate(),
                   "first option date (" << optionDates_[0] <<
                   ") must be greater than reference date (" <<
                   referenceDate() << ")");
        for (Size i=1; i<nOptionTenors_; ++i) {
            QL_REQUIRE(optionDates_[i]>optionDates_[i-1],
                       "non increasing option dates: " << io::ordinal(i) <<
                       " is " << optionDates_[i-1] << ", " << io::ordinal(i+1) <<
                       " is " << optionDates_[i]);
        }
    }

    void SwaptionVolatilityDiscrete::checkOptionTenors() const {
        QL_REQUIRE(optionTenors_[0]>0*Days,
                   "first option tenor is negative (" <<
                   optionTenors_[0] << ")");
        for (Size i=1; i<nOptionTenors_; ++i)
            QL_REQUIRE(optionTenors_[i]>optionTenors_[i-1],
                       "non increasing option tenor: " << io::ordinal(i) <<
                       " is " << optionTenors_[i-1] << ", " << io::ordinal(i+1) <<
                       " is " << optionTenors_[i]);
    }

    void SwaptionVolatilityDiscrete::checkSwapTenors() const {
        QL_REQUIRE(swapTenors_[0]>0*Days,
                   "first swap tenor is negative (" <<
                   swapTenors_[0] << ")");
        for (Size i=1; i<nSwapTenors_; ++i)
            QL_REQUIRE(swapTenors_[i]>swapTenors_[i-1],
                       "non increasing swap tenor: " << io::ordinal(i) <<
                       " is " << swapTenors_[i-1] << ", " << io::ordinal(i+1) <<
                       " is " << swapTenors_[i]);
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
    }

    void SwaptionVolatilityDiscrete::initializeSwapLengths() const {
        for (Size i=0; i<nSwapTenors_; ++i) 
            swapLengths_[i] = swapLength(swapTenors_[i]);
    }

    void SwaptionVolatilityDiscrete::performCalculations() const {
        // recalculate dates if necessary...
        if (moving_) {
            Date d = Settings::instance().evaluationDate();
            if (evaluationDate_ != d) {
                evaluationDate_ = d;
                initializeOptionDatesAndTimes();
                initializeSwapLengths();
                optionInterpolator_.update();
            }
        }
    }

    void SwaptionVolatilityDiscrete::update() {
        TermStructure::update();
        LazyObject::update();
    }

}
