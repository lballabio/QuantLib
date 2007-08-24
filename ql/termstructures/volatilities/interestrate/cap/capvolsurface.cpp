/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/termstructures/volatilities/interestrate/cap/capvolsurface.hpp>

namespace QuantLib {

    // floating reference date, floating market data
    CapVolatilitySurface::CapVolatilitySurface(
                          Natural settlementDays,
                          const Calendar& calendar,
                          const std::vector<Period>& optionLenghts,
                          const std::vector<Rate>& strikes,
                          const std::vector<std::vector<Handle<Quote> > >& volatilities)
    : CapVolatilityStructure(settlementDays, calendar),
      optionLenghts_(optionLenghts),
      optionTimes_(optionLenghts.size()), 
      strikes_(strikes),
      volHandles_(volatilities),
      volatilities_(volatilities.size(), volatilities[0].size())
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        registerWithMarketData();
        for (Size i=0; i<volatilities_.rows(); ++i)
          for (Size j=0; j<volatilities_.columns(); ++j)
              volatilities_[i][j] = volHandles_[i][j]->value();
        interpolate();
    }
    
    // fixed reference date, floating market data
    CapVolatilitySurface::CapVolatilitySurface(
                                const Date& settlementDate,
                                const std::vector<Period>& optionLenghts,
                                const std::vector<Rate>& strikes,
                                const std::vector<std::vector<Handle<Quote> > >& volatilities)
    : CapVolatilityStructure(settlementDate),
      optionLenghts_(optionLenghts),
      optionTimes_(optionLenghts.size()), 
      strikes_(strikes),
      volHandles_(volatilities),
      volatilities_(volatilities.size(), volatilities[0].size())
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        registerWithMarketData();
        for (Size i=0; i<volatilities_.rows(); ++i)
          for (Size j=0; j<volatilities_.columns(); ++j)
              volatilities_[i][j] = volHandles_[i][j]->value();
        interpolate();
    }

    // fixed reference date, fixed market data
    CapVolatilitySurface::CapVolatilitySurface(
                                const Date& settlementDate,
                                const std::vector<Period>& optionLenghts,
                                const std::vector<Rate>& strikes,
                                const Matrix& volatilities)
    : CapVolatilityStructure(settlementDate),
      optionLenghts_(optionLenghts),
      optionTimes_(optionLenghts.size()), 
      strikes_(strikes),
      volHandles_(volatilities.rows()),
      volatilities_(volatilities)
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<volatilities_.rows(); ++i) {
            volHandles_[i].resize(volatilities.columns());
            for (Size j=0; j<volatilities_.columns(); ++j)
                volHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                    SimpleQuote(volatilities[i][j])));
        }
        registerWithMarketData();
        interpolate();
    }

    // floating reference date, fixed market data
    CapVolatilitySurface::CapVolatilitySurface(
                                Natural settlementDays,
                                const Calendar& calendar,
                                const std::vector<Period>& optionLenghts,
                                const std::vector<Rate>& strikes,
                                const Matrix& volatilities)
    : CapVolatilityStructure(settlementDays,calendar),
      optionLenghts_(optionLenghts),
      strikes_(strikes),
      volHandles_(volatilities.rows()),
      volatilities_(volatilities)
    {
        checkInputs(volatilities_.rows(), volatilities_.columns());
        // fill dummy handles to allow generic handle-based computations later
        for (Size i=0; i<volatilities_.rows(); ++i) {
            volHandles_[i].resize(volatilities.columns());
            for (Size j=0; j<volatilities_.columns(); ++j)
                volHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                    SimpleQuote(volatilities[i][j])));
        }
        registerWithMarketData();
        //volatilities_[0] = volatilities[0];
        //std::copy(volatilities.begin(),volatilities.end(),volatilities_.begin()+1);
        interpolate();
    }

    void CapVolatilitySurface::checkInputs(Size volRows,
                                           Size volsColumns) const {
        QL_REQUIRE(optionLenghts_.size()==volRows,
                   "mismatch between number of cap lenght (" <<
                   optionLenghts_.size() <<
                   ") and number of cap volatilities (" << volRows << ")");
        QL_REQUIRE(volatilities_.columns()==strikes_.size(),
                   "mismatch between strikes(" << strikes_.size() <<
                   ") and vol columns (" << volatilities_.columns() << ")");        
    }

    void CapVolatilitySurface::performCalculations() const {
        for (Size i=0; i<volatilities_.rows(); ++i)
            for (Size j=0; j<volatilities_.columns(); ++j)
                volatilities_[i][j] = volHandles_[i][j]->value();
    }

    void CapVolatilitySurface::registerWithMarketData()
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            for (Size j=0; j<volHandles_[0].size(); j++)
                registerWith(volHandles_[i][j]);
    }

    void CapVolatilitySurface::interpolate() {
        for (Size i=0; i<optionLenghts_.size(); i++) {
            Date endDate = referenceDate() + optionLenghts_[i];
            optionTimes_[i] = timeFromReference(endDate);
        }
        interpolation_ =
            BicubicSpline(
                strikes_.begin(),  
                strikes_.end(),
                optionTimes_.begin(),
                optionTimes_.end(),
                volatilities_);
                //CubicSpline::SecondDerivative,
                //0.0,
                //CubicSpline::SecondDerivative,
                //0.0,
                //false);
            //BilinearInterpolation(
            //    strikes_.begin(), strikes_.end(),
            //    optionTimes_.begin(), optionTimes_.end(),
            //    volatilities_);
        interpolation_.update();
        maxDate_ = referenceDate() + optionLenghts_.back();
    }

}
