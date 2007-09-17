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

#include <ql/voltermstructures/interestrate/cap/capfloortermvolsurface.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    // floating reference date, floating market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        Natural settlementDays,
                        const Calendar& calendar,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const std::vector<std::vector<Handle<Quote> > >& vols,
                        BusinessDayConvention bdc,
                        const DayCounter& dc)
    : CapFloorVolatilityStructure(settlementDays, calendar, bdc, dc),
      optionTenors_(optionTenors),
      optionTimes_(optionTenors.size()), 
      strikes_(strikes),
      volHandles_(vols),
      volatilities_(vols.size(), vols[0].size())
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        registerWithMarketData();
        for (Size i=0; i<volatilities_.rows(); ++i)
          for (Size j=0; j<volatilities_.columns(); ++j)
              volatilities_[i][j] = volHandles_[i][j]->value();

        interpolate();
    }
    
    // fixed reference date, floating market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        const Date& settlementDate,
                        const Calendar& calendar,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const std::vector<std::vector<Handle<Quote> > >& vols,
                        BusinessDayConvention bdc,
                        const DayCounter& dc)
    : CapFloorVolatilityStructure(settlementDate, calendar, bdc, dc),
      optionTenors_(optionTenors),
      optionTimes_(optionTenors.size()), 
      strikes_(strikes),
      volHandles_(vols),
      volatilities_(vols.size(), vols[0].size())
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        registerWithMarketData();
        for (Size i=0; i<volatilities_.rows(); ++i)
          for (Size j=0; j<volatilities_.columns(); ++j)
              volatilities_[i][j] = volHandles_[i][j]->value();

        interpolate();
    }

    // fixed reference date, fixed market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        const Date& settlementDate,
                        const Calendar& calendar,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const Matrix& vols,
                        BusinessDayConvention bdc,
                        const DayCounter& dc)
    : CapFloorVolatilityStructure(settlementDate, calendar, bdc, dc),
      optionTenors_(optionTenors),
      optionTimes_(optionTenors.size()), 
      strikes_(strikes),
      volHandles_(vols.rows()),
      volatilities_(vols)
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<volatilities_.rows(); ++i) {
            volHandles_[i].resize(volatilities_.columns());
            for (Size j=0; j<volatilities_.columns(); ++j)
                volHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                    SimpleQuote(volatilities_[i][j])));
        }
        registerWithMarketData();

        interpolate();
    }

    // floating reference date, fixed market data
    CapFloorTermVolSurface::CapFloorTermVolSurface(
                        Natural settlementDays,
                        const Calendar& calendar,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Rate>& strikes,
                        const Matrix& vols,
                        BusinessDayConvention bdc,
                        const DayCounter& dc)
    : CapFloorVolatilityStructure(settlementDays, calendar, bdc, dc),
      optionTenors_(optionTenors),
      optionTimes_(optionTenors.size()), 
      strikes_(strikes),
      volHandles_(vols.rows()),
      volatilities_(vols)
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<volatilities_.rows(); ++i) {
            volHandles_[i].resize(volatilities_.columns());
            for (Size j=0; j<volatilities_.columns(); ++j)
                volHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                    SimpleQuote(volatilities_[i][j])));
        }
        registerWithMarketData();

        interpolate();
    }

    void CapFloorTermVolSurface::checkInputs(Size volRows,
                                             Size volsColumns) const {
        QL_REQUIRE(optionTenors_.size()==volRows,
                   "mismatch between number of option tenors (" <<
                   optionTenors_.size() <<
                   ") and number of volatilities (" << volRows << ")");
        QL_REQUIRE(volatilities_.columns()==strikes_.size(),
                   "mismatch between strikes(" << strikes_.size() <<
                   ") and vol columns (" << volatilities_.columns() << ")");        
    }

    void CapFloorTermVolSurface::registerWithMarketData()
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            for (Size j=0; j<volHandles_[0].size(); ++j)
                registerWith(volHandles_[i][j]);
    }

    void CapFloorTermVolSurface::interpolate() const
    {
        interpolation_ = BicubicSpline(strikes_.begin(),  
                                       strikes_.end(),
                                       optionTimes_.begin(),
                                       optionTimes_.end(),
                                       volatilities_);    
    }

    void CapFloorTermVolSurface::performCalculations() const {

        for (Size i=0; i<optionTenors_.size(); ++i) {
            Date endDate = optionDateFromTenor(optionTenors_[i]);
            optionTimes_[i] = timeFromReference(endDate);
        }

        for (Size i=0; i<volatilities_.rows(); ++i)
            for (Size j=0; j<volatilities_.columns(); ++j)
                volatilities_[i][j] = volHandles_[i][j]->value();

        interpolation_.update();
    }

}
