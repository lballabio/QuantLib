/*/ -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Dimitri Reiswich

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

#include <ql/quotes/deltavolquote.hpp>
#include <utility>

namespace QuantLib {

    DeltaVolQuote::DeltaVolQuote(Real delta, Handle<Quote> vol, Time maturity, DeltaType deltaType)
    : delta_(delta), vol_(std::move(vol)), deltaType_(deltaType), maturity_(maturity),
      atmType_(DeltaVolQuote::AtmNull) {

        registerWith(vol_); // observe vol
    }

    DeltaVolQuote::DeltaVolQuote(Handle<Quote> vol,
                                 DeltaType deltaType,
                                 Time maturity,
                                 AtmType atmType)
    : vol_(std::move(vol)), deltaType_(deltaType), maturity_(maturity), atmType_(atmType) {

        registerWith(vol_);
    }

    Real DeltaVolQuote::value() const {
        return vol_->value();
    }

    Real DeltaVolQuote::delta() const {
        return delta_;
    }

    Time DeltaVolQuote::maturity() const {
        return maturity_;
    }

    bool DeltaVolQuote::isValid() const {
        return !vol_.empty() && vol_->isValid();
    }

    void DeltaVolQuote::update() {
        notifyObservers(); // let observers know, that something has changed
    }

    DeltaVolQuote::AtmType DeltaVolQuote::atmType() const {
        return atmType_;
    }

    DeltaVolQuote::DeltaType DeltaVolQuote::deltaType() const {
        return deltaType_;
    }

}
