/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
 Copyright (C) 2015 Peter Caspers

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

/*! \file hestonmodelhelper.hpp
    \brief Heston-model calibration helper
*/

#ifndef quantlib_heston_option_helper_hpp
#define quantlib_heston_option_helper_hpp

#include <ql/models/calibrationhelper.hpp>
#include <ql/instruments/vanillaoption.hpp>

namespace QuantLib {

    //! calibration helper for Heston model
    class HestonModelHelper : public BlackCalibrationHelper {
      public:
        HestonModelHelper(const Period& maturity,
                          Calendar calendar,
                          Real s0,
                          Real strikePrice,
                          const Handle<Quote>& volatility,
                          const Handle<YieldTermStructure>& riskFreeRate,
                          const Handle<YieldTermStructure>& dividendYield,
                          BlackCalibrationHelper::CalibrationErrorType errorType =
                              BlackCalibrationHelper::RelativePriceError);

        HestonModelHelper(const Period& maturity,
                          Calendar calendar,
                          const Handle<Quote>& s0,
                          Real strikePrice,
                          const Handle<Quote>& volatility,
                          const Handle<YieldTermStructure>& riskFreeRate,
                          const Handle<YieldTermStructure>& dividendYield,
                          BlackCalibrationHelper::CalibrationErrorType errorType =
                              BlackCalibrationHelper::RelativePriceError);

        void addTimesTo(std::list<Time>&) const override {}
        void performCalculations() const override;
        Real modelValue() const override;
        Real blackPrice(Real volatility) const override;
        Time maturity() const  { calculate(); return tau_; }
      private:
        const Period maturity_;
        const Calendar calendar_;
        const Handle<Quote> s0_;
        const Real strikePrice_;
        const Handle<YieldTermStructure> riskFreeRate_;
        const Handle<YieldTermStructure> dividendYield_;
        mutable Date exerciseDate_;
        mutable Time tau_;
        mutable Option::Type type_;
        mutable ext::shared_ptr<VanillaOption> option_;
    };

}


#endif



#ifndef id_831dea48921315e5d6432aff16e176fb
#define id_831dea48921315e5d6432aff16e176fb
inline bool test_831dea48921315e5d6432aff16e176fb(const int* i) {
    return i != nullptr;
}
#endif
