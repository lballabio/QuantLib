/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Katiuscia Manzoni

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

#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    // floating reference date, floating market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        Natural settlementDays,
                        const Calendar& calendar,
                        BusinessDayConvention bdc,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const std::vector<std::vector<Handle<Quote> > >& vols,
                        const DayCounter& dc)
    : CapFloorTermVolatilityStructure(settlementDays, calendar, bdc, dc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      nStrikes_(strikes.size()),
      strikes_(strikes),
      volHandles_(vols),
      vols_(vols.size(), vols[0].size())
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        for (Size i=0; i<nOptionTenors_; ++i)
            QL_REQUIRE(volHandles_[i].size()==nStrikes_,
                       io::ordinal(i+1) << " row of vol handles has size " <<
                       volHandles_[i].size() << " instead of " << nStrikes_);
        registerWithMarketData();
        for (Size i=0; i<vols_.rows(); ++i)
            for (Size j=0; j<vols_.columns(); ++j)
                vols_[i][j] = volHandles_[i][j]->value();
        interpolate();
    }

    // fixed reference date, floating market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        const Date& settlementDate,
                        const Calendar& calendar,
                        BusinessDayConvention bdc,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const std::vector<std::vector<Handle<Quote> > >& vols,
                        const DayCounter& dc)
    : CapFloorTermVolatilityStructure(settlementDate, calendar, bdc, dc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      nStrikes_(strikes.size()),
      strikes_(strikes),
      volHandles_(vols),
      vols_(vols.size(), vols[0].size())
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        for (Size i=0; i<nOptionTenors_; ++i)
            QL_REQUIRE(volHandles_[i].size()==nStrikes_,
                       io::ordinal(i+1) << " row of vol handles has size " <<
                       volHandles_[i].size() << " instead of " << nStrikes_);
        registerWithMarketData();
        for (Size i=0; i<vols_.rows(); ++i)
            for (Size j=0; j<vols_.columns(); ++j)
                vols_[i][j] = volHandles_[i][j]->value();
        interpolate();
    }

    // fixed reference date, fixed market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        const Date& settlementDate,
                        const Calendar& calendar,
                        BusinessDayConvention bdc,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const Matrix& vols,
                        const DayCounter& dc)
    : CapFloorTermVolatilityStructure(settlementDate, calendar, bdc, dc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      nStrikes_(strikes.size()),
      strikes_(strikes),
      volHandles_(vols.rows()),
      vols_(vols)
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<nOptionTenors_; ++i) {
            volHandles_[i].resize(nStrikes_);
            for (Size j=0; j<nStrikes_; ++j)
                volHandles_[i][j] = Handle<Quote>(std::shared_ptr<Quote>(new
                    SimpleQuote(vols_[i][j])));
        }
        interpolate();
    }

    // floating reference date, fixed market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        Natural settlementDays,
                        const Calendar& calendar,
                        BusinessDayConvention bdc,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const Matrix& vols,
                        const DayCounter& dc)
    : CapFloorTermVolatilityStructure(settlementDays, calendar, bdc, dc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      nStrikes_(strikes.size()),
      strikes_(strikes),
      volHandles_(vols.rows()),
      vols_(vols)
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<nOptionTenors_; ++i) {
            volHandles_[i].resize(nStrikes_);
            for (Size j=0; j<nStrikes_; ++j)
                volHandles_[i][j] = Handle<Quote>(std::shared_ptr<Quote>(new
                    SimpleQuote(vols_[i][j])));
        }
        interpolate();
    }

    void CapFloorTermVolSurface::checkInputs() const {

        QL_REQUIRE(!optionTenors_.empty(), "empty option tenor vector");
        QL_REQUIRE(nOptionTenors_==vols_.rows(),
                   "mismatch between number of option tenors (" <<
                   nOptionTenors_ << ") and number of volatility rows (" <<
                   vols_.rows() << ")");
        QL_REQUIRE(optionTenors_[0]>0*Days,
                   "negative first option tenor: " << optionTenors_[0]);
        for (Size i=1; i<nOptionTenors_; ++i)
            QL_REQUIRE(optionTenors_[i]>optionTenors_[i-1],
                       "non increasing option tenor: " << io::ordinal(i) <<
                       " is " << optionTenors_[i-1] << ", " <<
                       io::ordinal(i+1) << " is " << optionTenors_[i]);

        QL_REQUIRE(nStrikes_==vols_.columns(),
                   "mismatch between strikes(" << strikes_.size() <<
                   ") and vol columns (" << vols_.columns() << ")");
        for (Size j=1; j<nStrikes_; ++j)
            QL_REQUIRE(strikes_[j-1]<strikes_[j],
                       "non increasing strikes: " << io::ordinal(j) <<
                       " is " << io::rate(strikes_[j-1]) << ", " <<
                       io::ordinal(j+1) << " is " << io::rate(strikes_[j]));
    }

    void CapFloorTermVolSurface::registerWithMarketData()
    {
        for (Size i=0; i<nOptionTenors_; ++i)
            for (Size j=0; j<nStrikes_; ++j)
                registerWith(volHandles_[i][j]);
    }

    void CapFloorTermVolSurface::interpolate()
    {
        interpolation_ = BicubicSpline(strikes_.begin(),
                                       strikes_.end(),
                                       optionTimes_.begin(),
                                       optionTimes_.end(),
                                       vols_);
    }

    void CapFloorTermVolSurface::update()
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

    void CapFloorTermVolSurface::initializeOptionDatesAndTimes() const
    {
        for (Size i=0; i<nOptionTenors_; ++i) {
            optionDates_[i] = optionDateFromTenor(optionTenors_[i]);
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        }
    }

    void CapFloorTermVolSurface::performCalculations() const
    {
        // check if date recalculation must be called here

        for (Size i=0; i<nOptionTenors_; ++i)
            for (Size j=0; j<nStrikes_; ++j)
                vols_[i][j] = volHandles_[i][j]->value();

        interpolation_.update();
    }

}
