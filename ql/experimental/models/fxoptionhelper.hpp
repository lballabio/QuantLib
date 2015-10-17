
/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file fxoptionhelper.hpp
    \brief calibration helper for fx options
           the discounting curve for the option value is the domestic yield
*/

#ifndef quantlib_calibrationhelper_fxoption_hpp
#define quantlib_calibrationhelper_fxoption_hpp

#include <ql/models/calibrationhelper.hpp>
#include <ql/instruments/vanillaoption.hpp>

namespace QuantLib {

class FxOptionHelper : public CalibrationHelper {
  public:
    /*! the fx spot is interpreted as of today (or discounted spot)
        if strike is null, an (fwd-) atm option is constructed */
    FxOptionHelper(const Period &maturity, const Calendar &calendar,
                   const Real strike, const Handle<Quote> fxSpot,
                   const Handle<Quote> volatility,
                   const Handle<YieldTermStructure> &domesticYield,
                   const Handle<YieldTermStructure> &foreignYield,
                   CalibrationHelper::CalibrationErrorType errorType =
                       CalibrationHelper::RelativePriceError);
    FxOptionHelper(const Date &exerciseDate, const Real strike,
                   const Handle<Quote> fxSpot, const Handle<Quote> volatility,
                   const Handle<YieldTermStructure> &domesticYield,
                   const Handle<YieldTermStructure> &foreignYield,
                   CalibrationHelper::CalibrationErrorType errorType =
                       CalibrationHelper::RelativePriceError);
    void addTimesTo(std::list<Time> &) const {}
    void performCalculations() const;
    Real modelValue() const;
    Real blackPrice(Real volatility) const;
    boost::shared_ptr<VanillaOption> option() const { return option_; }

  private:
    const bool hasMaturity_;
    Period maturity_;
    mutable Date exerciseDate_;
    Calendar calendar_;
    const Real strike_;
    const Handle<Quote> fxSpot_;
    const Handle<YieldTermStructure> foreignYield_;
    mutable Real tau_;
    mutable Real atm_;
    mutable Option::Type type_;
    mutable boost::shared_ptr<VanillaOption> option_;
    mutable Real effStrike_;
};

} // namespace QuantLib

#endif
