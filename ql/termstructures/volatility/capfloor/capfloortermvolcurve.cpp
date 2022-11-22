/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

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

#include <ql/termstructures/volatility/capfloor/capfloortermvolcurve.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    // floating reference date, floating market data
    CapFloorTermVolCurve::CapFloorTermVolCurve(
                        Natural settlementDays,
                        const Calendar& calendar,
                        BusinessDayConvention bdc,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Handle<Quote> >& vols,
                        const DayCounter& dc)
    : CapFloorTermVolatilityStructure(settlementDays, calendar, bdc, dc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      volHandles_(vols),
      vols_(vols.size()) // do not initialize with nOptionTenors_
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        registerWithMarketData();
        interpolate();
    }

    // fixed reference date, floating market data
    CapFloorTermVolCurve::CapFloorTermVolCurve(
                            const Date& settlementDate,
                            const Calendar& calendar,
                            BusinessDayConvention bdc,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Handle<Quote> >& vols,
                            const DayCounter& dayCounter)
    : CapFloorTermVolatilityStructure(settlementDate, calendar, bdc, dayCounter),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      volHandles_(vols),
      vols_(vols.size()) // do not initialize with nOptionTenors_
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        registerWithMarketData();
        interpolate();
    }

    // fixed reference date, fixed market data
    CapFloorTermVolCurve::CapFloorTermVolCurve(
                                const Date& settlementDate,
                                const Calendar& calendar,
                                BusinessDayConvention bdc,
                                const std::vector<Period>& optionTenors,
                                const std::vector<Volatility>& vols,
                                const DayCounter& dayCounter)
    : CapFloorTermVolatilityStructure(settlementDate, calendar, bdc, dayCounter),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      volHandles_(vols.size()), // do not initialize with nOptionTenors_
      vols_(vols)
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<nOptionTenors_; ++i)
            volHandles_[i] = Handle<Quote>(ext::shared_ptr<Quote>(new
                SimpleQuote(vols_[i])));
        interpolate();
    }

    // floating reference date, fixed market data
    CapFloorTermVolCurve::CapFloorTermVolCurve(
                                Natural settlementDays,
                                const Calendar& calendar,
                                BusinessDayConvention bdc,
                                const std::vector<Period>& optionTenors,
                                const std::vector<Volatility>& vols,
                                const DayCounter& dayCounter)
    : CapFloorTermVolatilityStructure(settlementDays, calendar, bdc, dayCounter),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      volHandles_(vols.size()), // do not initialize with nOptionTenors_
      vols_(vols)
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<nOptionTenors_; ++i)
            volHandles_[i] = Handle<Quote>(ext::shared_ptr<Quote>(new
                SimpleQuote(vols_[i])));
        interpolate();
    }

    void CapFloorTermVolCurve::checkInputs() const
    {
        QL_REQUIRE(!optionTenors_.empty(), "empty option tenor vector");
        QL_REQUIRE(nOptionTenors_==vols_.size(),
                   "mismatch between number of option tenors (" <<
                   nOptionTenors_ << ") and number of volatilities (" <<
                   vols_.size() << ")");
        QL_REQUIRE(optionTenors_[0]>0*Days,
                   "negative first option tenor: " << optionTenors_[0]);
        for (Size i=1; i<nOptionTenors_; ++i)
            QL_REQUIRE(optionTenors_[i]>optionTenors_[i-1],
                       "non increasing option tenor: " << io::ordinal(i) <<
                       " is " << optionTenors_[i-1] << ", " <<
                       io::ordinal(i+1) << " is " << optionTenors_[i]);
    }

    void CapFloorTermVolCurve::registerWithMarketData()
    {
        for (auto& volHandle : volHandles_)
            registerWith(volHandle);
    }

    void CapFloorTermVolCurve::interpolate()
    {
        interpolation_ = CubicInterpolation(
                                    optionTimes_.begin(), optionTimes_.end(),
                                    vols_.begin(),
                                    CubicInterpolation::Spline, false,
                                    CubicInterpolation::SecondDerivative, 0.0,
                                    CubicInterpolation::SecondDerivative, 0.0);
    }

    void CapFloorTermVolCurve::update()
    {
        // recalculate dates if necessary...
        if (moving_) {
            Date d = Settings::instance().evaluationDate();
            if (evaluationDate_ != d) {
                evaluationDate_ = d;
                initializeOptionDatesAndTimes();
            }
        }
        CapFloorTermVolatilityStructure::update();
        LazyObject::update();
    }

    void CapFloorTermVolCurve::initializeOptionDatesAndTimes() const
    {
        for (Size i=0; i<nOptionTenors_; ++i) {
            optionDates_[i] = optionDateFromTenor(optionTenors_[i]);
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        }
    }

    void CapFloorTermVolCurve::performCalculations() const
    {
        // check if date recalculation must be called here

        for (Size i=0; i<vols_.size(); ++i)
            vols_[i] = volHandles_[i]->value();

        interpolation_.update();
    }

}
