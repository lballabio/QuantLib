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

#include <ql/quotes/impliedstddevquote.hpp>
#include <ql/pricingengines/blackformula.hpp>

namespace QuantLib {

    ImpliedStdDevQuote::ImpliedStdDevQuote(Option::Type optionType,
                                           const Handle<Quote>& forward,
                                           const Handle<Quote>& price,
                                           Real strike,
                                           Real guess,
                                           Real accuracy)
    : impliedStdev_(guess), optionType_(optionType), strike_(strike),
      accuracy_(accuracy), forward_(forward), price_(price) {
        registerWith(forward_);
        registerWith(price_);
    }

    Real ImpliedStdDevQuote::value() const {
        calculate();
        return impliedStdev_;
    }

    void ImpliedStdDevQuote::performCalculations() const {
        static const Real discount_ = 1.0;
        Real blackPrice = price_->value();
        try {
            impliedStdev_ =
                blackFormulaImpliedStdDev(optionType_, strike_,
                                          forward_->value(), blackPrice,
                                          discount_, impliedStdev_,
                                          accuracy_);
        } catch(QuantLib::Error&) {
            impliedStdev_ = 0.0;
        }
    }
}
