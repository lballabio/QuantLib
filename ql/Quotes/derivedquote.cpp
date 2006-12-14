/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2006 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Quotes/derivedquote.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/ShortRateModels/OneFactorModels/hullwhite.hpp>


namespace QuantLib {

    ForwardValueQuote::ForwardValueQuote(
                            const boost::shared_ptr<Index>& index,
                            const Date& fixingDate)
    : index_(index), fixingDate_(fixingDate) {
        registerWith(index_);
    }

    ImpliedStdDevQuote::ImpliedStdDevQuote(Option::Type optionType,
                                           const Handle<Quote>& forward,
                                           const Handle<Quote>& price,
                                           Real strike,
                                           Real guess,
                                           Real accuracy)
    : impliedVolatility_(guess), optionType_(optionType), strike_(strike),
      accuracy_(accuracy), forward_(forward), price_(price) {
        registerWith(forward_);
        registerWith(price_);
    }

    Real ImpliedStdDevQuote::value() const {
        static const Real discount_ = 1.0;
        Real blackPrice = price_->value();
        try {
            impliedVolatility_ = blackImpliedStdDev(optionType_, strike_,
                forward_->value(), blackPrice, discount_, impliedVolatility_,
                accuracy_);
        } catch(QuantLib::Error&) {
            impliedVolatility_ = 0.0;
        }
        return impliedVolatility_;
    }

    EurodollarFuturesImpliedStdDevQuote::EurodollarFuturesImpliedStdDevQuote(
                                const Handle<Quote>& forward,
                                const Handle<Quote>& callPrice,
                                const Handle<Quote>& putPrice,
                                Real strike,
                                Real guess,
                                Real accuracy)
    : impliedVolatility_(guess), strike_(100.0-strike),
      accuracy_(accuracy), forward_(forward),
      callPrice_(callPrice), putPrice_(putPrice) {
        registerWith(forward_);
        registerWith(callPrice_);
        registerWith(putPrice_);
    }

    Real EurodollarFuturesImpliedStdDevQuote::value() const {
        static const Real discount_ = 1.0;
        Real forwardValue = 100.0-forward_->value();
        if (strike_>forwardValue) {
            impliedVolatility_ = blackImpliedStdDev(Option::Call, strike_,
                forwardValue, putPrice_->value(), discount_,
                impliedVolatility_, accuracy_);
        } else {
            impliedVolatility_ = blackImpliedStdDev(Option::Put, strike_,
                forwardValue, callPrice_->value(), discount_,
                impliedVolatility_, accuracy_);
        }
        return impliedVolatility_;
    }

    
    FuturesConvAdjustmentQuote::FuturesConvAdjustmentQuote(
                               const boost::shared_ptr<InterestRateIndex>& index,
                               const Date& futuresDate, 
                               const Handle<Quote>& futuresQuote,
                               const Handle<Quote>& volatility,
                               const Handle<Quote>& meanReversion)
    : index_(index), futuresDate_(futuresDate), futuresQuote_(futuresQuote),
      volatility_(volatility), meanReversion_(meanReversion) {
        registerWith(index_);
        registerWith(futuresQuote_);
        registerWith(volatility_);
        registerWith(meanReversion_);
    }

    Real FuturesConvAdjustmentQuote::value() const {
        
        DayCounter dc = index_->dayCounter();
        Date settlementDate = Settings::instance().evaluationDate();
        Time startTime = dc.yearFraction(settlementDate, futuresDate_);

        Date indexMaturityDate = index_->maturityDate(futuresDate_);
        Time indexMaturity = dc.yearFraction(settlementDate, indexMaturityDate);

        Real convexity = HullWhite::convexityBias(futuresQuote_->value(),
                                                 startTime,
                                                 indexMaturity,
                                                 volatility_->value(),
                                                 meanReversion_->value());
        return convexity;
    }
}
