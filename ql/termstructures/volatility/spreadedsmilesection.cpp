/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/quote.hpp>
#include <ql/termstructures/volatility/spreadedsmilesection.hpp>
#include <utility>

namespace QuantLib {

    SpreadedSmileSection::SpreadedSmileSection(ext::shared_ptr<SmileSection> underlyingSection,
                                               Handle<Quote> spread)
    : underlyingSection_(std::move(underlyingSection)), spread_(std::move(spread)) {
        registerWith(underlyingSection_);
        registerWith(spread_);
    }

    Volatility SpreadedSmileSection::volatilityImpl(Rate k) const {
        return underlyingSection_->volatility(k) + spread_->value();
    }

}
