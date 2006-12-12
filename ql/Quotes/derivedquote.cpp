/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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
        Real blackprice = price_->value();
        // ugly fix to avoid having a non sensical guess, to be improved ...
        Real guess;
        if (impliedVolatility_>.001 && impliedVolatility_<.4)
            guess = impliedVolatility_;
        else
            guess = .15;
        try {
        impliedVolatility_ = blackImpliedStdDev(optionType_, strike_,
            forwardValue(), blackprice, discount_, guess, accuracy_);
        } catch(QuantLib::Error&) {
            QL_FAIL("ImpliedStdDevQuote::value:"
                    "\niv computation failure at option strike " << strike_ 
                    << ", option price is " << blackprice);
        }
        return impliedVolatility_;
    }

    EurodollarFuturesImpliedStdDevQuote::EurodollarFuturesImpliedStdDevQuote(
                                Option::Type optionType,
                                const Handle<Quote>& forward,
                                const Handle<Quote>& price,
                                Real strike,
                                Real guess,
                                Real accuracy)
    : ImpliedStdDevQuote(optionType==Option::Call ? Option::Put : Option::Call,
                         forward, price, 100.0-strike, guess, accuracy) {}

}
