/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

#include <ql/quotes/eurodollarfuturesquote.hpp>
#include <ql/pricingengines/blackformula.hpp>

namespace QuantLib {

    EurodollarFuturesImpliedStdDevQuote::EurodollarFuturesImpliedStdDevQuote(
                                const Handle<Quote>& forward,
                                const Handle<Quote>& callPrice,
                                const Handle<Quote>& putPrice,
                                Real strike,
                                Real guess,
                                Real accuracy,
                                Natural maxIter)
    : impliedStdev_(guess), strike_(100.0-strike),
      accuracy_(accuracy), maxIter_(maxIter), forward_(forward),
      callPrice_(callPrice), putPrice_(putPrice) {
        registerWith(forward_);
        registerWith(callPrice_);
        registerWith(putPrice_);
    }

    Real EurodollarFuturesImpliedStdDevQuote::value() const {
        calculate();
        return impliedStdev_;
    }

    bool EurodollarFuturesImpliedStdDevQuote::isValid() const {
        if (forward_.empty() || !forward_->isValid())
            return false;
        Real forwardValue = 100.0-forward_->value();
        if (strike_>forwardValue)
            return !putPrice_.empty() && putPrice_->isValid();
        else
            return !callPrice_.empty() && callPrice_->isValid();
    }

    void EurodollarFuturesImpliedStdDevQuote::performCalculations() const {
        static const Real discount = 1.0;
        static const Real displacement = 0.0;
        Real forwardValue = 100.0-forward_->value();
        if (strike_>forwardValue) {
            impliedStdev_ =
                blackFormulaImpliedStdDev(Option::Call, strike_,
                                          forwardValue, putPrice_->value(),
                                          discount, displacement,
                                          impliedStdev_, accuracy_, maxIter_);
        } else {
            impliedStdev_ =
                blackFormulaImpliedStdDev(Option::Put, strike_,
                                          forwardValue, callPrice_->value(),
                                          discount, displacement,
                                          impliedStdev_, accuracy_, maxIter_);
        }
    }
}
