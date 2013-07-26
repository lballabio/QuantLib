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

/*! \file swaptionhelper.hpp
    \brief Swaption calibration helper
*/

#ifndef quantlib_swaption_calibration_helper_hpp
#define quantlib_swaption_calibration_helper_hpp

#include <ql/models/calibrationhelper.hpp>
#include <ql/instruments/swaption.hpp>

namespace QuantLib {

    //! calibration helper for ATM swaption

    class SwaptionHelper : public CalibrationHelper {
      public:
        SwaptionHelper(const Period& maturity,
                       const Period& length,
                       const Handle<Quote>& volatility,
                       const boost::shared_ptr<IborIndex>& index,
                       const Period& fixedLegTenor,
                       const DayCounter& fixedLegDayCounter,
                       const DayCounter& floatingLegDayCounter,
                       const Handle<YieldTermStructure>& termStructure,
                       CalibrationHelper::CalibrationErrorType errorType
                                      = CalibrationHelper::RelativePriceError,
                       const Real strike = Null<Real>(),
                       const Real nominal = 1.0);

        SwaptionHelper(const Date& exerciseDate,
                       const Period& length,
                       const Handle<Quote>& volatility,
                       const boost::shared_ptr<IborIndex>& index,
                       const Period& fixedLegTenor,
                       const DayCounter& fixedLegDayCounter,
                       const DayCounter& floatingLegDayCounter,
                       const Handle<YieldTermStructure>& termStructure,
                       CalibrationHelper::CalibrationErrorType errorType
                                      = CalibrationHelper::RelativePriceError,
                       const Real strike = Null<Real>(),
                       const Real nominal = 1.0);

        SwaptionHelper(const Date& exerciseDate,
                       const Date& endDate,
                       const Handle<Quote>& volatility,
                       const boost::shared_ptr<IborIndex>& index,
                       const Period& fixedLegTenor,
                       const DayCounter& fixedLegDayCounter,
                       const DayCounter& floatingLegDayCounter,
                       const Handle<YieldTermStructure>& termStructure,
                       CalibrationHelper::CalibrationErrorType errorType
                                      = CalibrationHelper::RelativePriceError,
                       const Real strike = Null<Real>(),
                       const Real nominal = 1.0);

        virtual void addTimesTo(std::list<Time>& times) const;
        virtual Real modelValue() const;
        virtual Real blackPrice(Volatility volatility) const;

        boost::shared_ptr<VanillaSwap> underlyingSwap() const { calculate(); return swap_; }
        boost::shared_ptr<Swaption> swaption() const { calculate(); return swaption_; }

      private:
        void performCalculations() const;
        mutable Date exerciseDate_, endDate_;
        const Period maturity_, length_, fixedLegTenor_;
        const boost::shared_ptr<IborIndex> index_;
        const DayCounter fixedLegDayCounter_, floatingLegDayCounter_;
        const Real strike_, nominal_;
        mutable Rate exerciseRate_;
        mutable boost::shared_ptr<VanillaSwap> swap_;
        mutable boost::shared_ptr<Swaption> swaption_;
    };

}

#endif
