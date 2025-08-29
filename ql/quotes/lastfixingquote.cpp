/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2014 Ferdinando Ametrano

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

#include <ql/quotes/lastfixingquote.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    LastFixingQuote::LastFixingQuote(ext::shared_ptr<Index> index) : index_(std::move(index)) {
        registerWith(index_);
    }

    Real LastFixingQuote::value() const {
        QL_ENSURE(isValid(),
                  index_->name() << " has no fixing");
        return index_->fixing(referenceDate());
    }

    bool LastFixingQuote::isValid() const {
        return !index_->timeSeries().empty();
    }

    Date LastFixingQuote::referenceDate() const {
        return std::min<Date>(index_->timeSeries().lastDate(),
                              Settings::instance().evaluationDate());
    }
}
