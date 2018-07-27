/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006, 2007 StatPro Italia srl
 Copyright (C) 2015, 2016, 2017 Peter Caspers
 Copyright (C) 2017 Paul Giltinan
 Copyright (C) 2017 Werner Kuerzinger

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

#include <ql/pricingengines/swaption/blackswaptionengine.hpp>

namespace QuantLib {

    BlackSwaptionEngine::BlackSwaptionEngine(
                        const Handle<YieldTermStructure> &discountCurve,
                        Volatility vol, const DayCounter &dc,
                        Real displacement, CashAnnuityModel model,
                        Date settlementDate, Date npvDate)
    : detail::BlackStyleSwaptionEngine<detail::Black76Spec>(discountCurve, vol, dc,
                                                            displacement, model,
                                                            settlementDate, npvDate) {}

    BlackSwaptionEngine::BlackSwaptionEngine(
                        const Handle<YieldTermStructure> &discountCurve,
                        const Handle<Quote> &vol,
                        const DayCounter &dc,
                        Real displacement,
                        CashAnnuityModel model,
                        Date settlementDate, Date npvDate)
    : detail::BlackStyleSwaptionEngine<detail::Black76Spec>(discountCurve, vol, dc,
                                                            displacement, model,
                                                            settlementDate, npvDate) {}

    BlackSwaptionEngine::BlackSwaptionEngine(
                        const Handle<YieldTermStructure> &discountCurve,
                        const Handle<SwaptionVolatilityStructure> &vol,
                        CashAnnuityModel model,
                        Date settlementDate, Date npvDate)
    : detail::BlackStyleSwaptionEngine<detail::Black76Spec>(discountCurve, vol,
                                                            model,
                                                            settlementDate, npvDate) {
        QL_REQUIRE(vol->volatilityType() == ShiftedLognormal,
                   "BlackSwaptionEngine requires (shifted) lognormal input "
                   "volatility");
    }


    BachelierSwaptionEngine::BachelierSwaptionEngine(
        const Handle<YieldTermStructure> &discountCurve, Volatility vol,
        const DayCounter &dc, CashAnnuityModel model,
        Date settlementDate, Date npvDate)
    : detail::BlackStyleSwaptionEngine<detail::BachelierSpec>(discountCurve, vol,
                                                              dc, 0., model,
                                                              settlementDate, npvDate) {}

    BachelierSwaptionEngine::BachelierSwaptionEngine(
        const Handle<YieldTermStructure> &discountCurve,
        const Handle<Quote> &vol, const DayCounter &dc, CashAnnuityModel model,
        Date settlementDate, Date npvDate)
    : detail::BlackStyleSwaptionEngine<detail::BachelierSpec>(discountCurve, vol,
                                                              dc, 0., model,
                                                              settlementDate, npvDate) {}

    BachelierSwaptionEngine::BachelierSwaptionEngine(
        const Handle<YieldTermStructure> &discountCurve,
        const Handle<SwaptionVolatilityStructure> &vol, CashAnnuityModel model,
        Date settlementDate, Date npvDate)
    : detail::BlackStyleSwaptionEngine<detail::BachelierSpec>(discountCurve, vol,
                                                              model,
                                                              settlementDate, npvDate) {
        QL_REQUIRE(vol->volatilityType() == Normal,
                   "BachelierSwaptionEngine requires normal input volatility");
    }

} // namespace QuantLib
