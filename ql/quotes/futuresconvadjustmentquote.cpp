/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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

#include <ql/quotes/futuresconvadjustmentquote.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/time/imm.hpp>

namespace QuantLib {

    FuturesConvAdjustmentQuote::FuturesConvAdjustmentQuote(
                               const boost::shared_ptr<IborIndex>& index,
                               const Date& futuresDate,
                               const Handle<Quote>& futuresQuote,
                               const Handle<Quote>& volatility,
                               const Handle<Quote>& meanReversion)
    : dc_(index->dayCounter()),
      futuresDate_(futuresDate),
      indexMaturityDate_(index->maturityDate(futuresDate_)),
      futuresQuote_(futuresQuote),
      volatility_(volatility), meanReversion_(meanReversion) {

        registerWith(futuresQuote_);
        registerWith(volatility_);
        registerWith(meanReversion_);
    }

    FuturesConvAdjustmentQuote::FuturesConvAdjustmentQuote(
                               const boost::shared_ptr<IborIndex>& index,
                               const std::string& immCode,
                               const Handle<Quote>& futuresQuote,
                               const Handle<Quote>& volatility,
                               const Handle<Quote>& meanReversion)
    : dc_(index->dayCounter()),
      futuresDate_(IMM::date(immCode)),
      indexMaturityDate_(index->maturityDate(futuresDate_)),
      futuresQuote_(futuresQuote),
      volatility_(volatility), meanReversion_(meanReversion) {

        registerWith(futuresQuote_);
        registerWith(volatility_);
        registerWith(meanReversion_);
    }

    Real FuturesConvAdjustmentQuote::value() const {

        Date settlementDate = Settings::instance().evaluationDate();
        Time startTime = dc_.yearFraction(settlementDate, futuresDate_);
        Time indexMaturity = dc_.yearFraction(settlementDate,
                                              indexMaturityDate_);
        return HullWhite::convexityBias(futuresQuote_->value(),
                                        startTime,
                                        indexMaturity,
                                        volatility_->value(),
                                        meanReversion_->value());
    }

    bool FuturesConvAdjustmentQuote::isValid() const {
        return !futuresQuote_.empty() && !volatility_.empty() &&
               !meanReversion_.empty() && futuresQuote_->isValid() &&
               volatility_->isValid() && meanReversion_->isValid();
    }
}
