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

/*! \file proxyfxtarfengine.hpp
    \brief Proxy engine for FX Tarf
*/

#ifndef quantlib_pricingengines_proxy_fxtarf_hpp
#define quantlib_pricingengines_proxy_fxtarf_hpp

#include <ql/experimental/fx/fxtarfengine.hpp>

namespace QuantLib {

class ProxyFxTarfEngine : public FxTarfEngine {
  public:
    ProxyFxTarfEngine(
        boost::shared_ptr<ProxyInstrument::ProxyDescription> proxy,
        Handle<Quote> exchangeRate, Handle<YieldTermStructure> discount,
        bool interpolate = true)
        : FxTarfEngine(discount), exchangeRate_(exchangeRate),
          interpolate_(interpolate) {
        exchangeRateSet_ = !exchangeRate_.empty();
        if(exchangeRateSet_)
            registerWith(exchangeRate_);
        proxy_ = boost::dynamic_pointer_cast<FxTarf::Proxy>(proxy);
        QL_REQUIRE(proxy_, "no FxTarf::Proxy given");
    }

    void calculate() const;

  private:
    boost::shared_ptr<FxTarf::Proxy> proxy_;
    mutable Handle<Quote> exchangeRate_;
    bool interpolate_, exchangeRateSet_;
};

} // namespace QuantLib

#endif
