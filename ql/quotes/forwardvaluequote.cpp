/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

#include <ql/quotes/forwardvaluequote.hpp>
#include <utility>

namespace QuantLib {

    ForwardValueQuote::ForwardValueQuote(ext::shared_ptr<Index> index, const Date& fixingDate)
    : index_(std::move(index)), fixingDate_(fixingDate) {
        registerWith(index_);
    }

    Real ForwardValueQuote::value() const {
        return index_->fixing(fixingDate_);
    }

    bool ForwardValueQuote::isValid() const {
        // not sure this is the best approach...
        return true;
    }

    void ForwardValueQuote::update() {
        notifyObservers();
    }

}

