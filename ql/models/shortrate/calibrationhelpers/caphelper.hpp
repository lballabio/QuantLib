/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file caphelper.hpp
    \brief CapHelper calibration helper
*/

#ifndef quantlib_cap_calibration_helper_hpp
#define quantlib_cap_calibration_helper_hpp

#include <ql/models/calibrationhelper.hpp>
#include <ql/instruments/capfloor.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

namespace QuantLib {

    //! calibration helper for ATM cap

    class CapHelper : public BlackCalibrationHelper {
      public:
        CapHelper(const Period& length,
                  const Handle<Quote>& volatility,
                  std::shared_ptr<IborIndex> index,
                  // data for ATM swap-rate calculation
                  Frequency fixedLegFrequency,
                  DayCounter fixedLegDayCounter,
                  bool includeFirstSwaplet,
                  Handle<YieldTermStructure> termStructure,
                  BlackCalibrationHelper::CalibrationErrorType errorType =
                      BlackCalibrationHelper::RelativePriceError,
                  VolatilityType type = ShiftedLognormal,
                  Real shift = 0.0);
        void addTimesTo(std::list<Time>& times) const override;
        Real modelValue() const override;
        Real blackPrice(Volatility volatility) const override;

      private:
        void performCalculations() const override;
        mutable std::shared_ptr<Cap> cap_;
        const Period length_;
        const std::shared_ptr<IborIndex> index_;
        const Handle<YieldTermStructure> termStructure_;
        const Frequency fixedLegFrequency_;
        const DayCounter fixedLegDayCounter_;
        const bool includeFirstSwaplet_;
    };

}

#endif
