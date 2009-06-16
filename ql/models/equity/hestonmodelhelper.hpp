/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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
    class HestonModelHelper : public CalibrationHelper {
      public:
        // constructor for ATM option
        HestonModelHelper(const Period& maturity,
                          const Calendar& calendar,
                          const Real s0,
                          const Real strikePrice,
                          const Handle<Quote>& volatility,
                          const Handle<YieldTermStructure>& riskFreeRate,
                          const Handle<YieldTermStructure>& dividendYield,
                          CalibrationHelper::CalibrationErrorType errorType
                                    = CalibrationHelper::RelativePriceError);

        void addTimesTo(std::list<Time>&) const {}
        Real modelValue() const;
        Real blackPrice(Real volatility) const;
        Time maturity() const  { return tau_; }
      private:
        Handle<YieldTermStructure> dividendYield_;
        boost::shared_ptr<VanillaOption> option_;
        const Date exerciseDate_;
        const Time tau_;
        const Real s0_;
        const Real strikePrice_;
    };

}


#endif

