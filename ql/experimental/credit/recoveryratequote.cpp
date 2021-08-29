/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Jose Aparicio

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

#include <ql/experimental/credit/recoveryratequote.hpp>

namespace QuantLib {

    namespace { // file scope

        namespace RecoveryRateQuotePrivate {

            // Conventional recoveries for ISDA seniorities
            constexpr Real IsdaConvRecoveries[] =  {
                0.65,// SECDOM
                0.4, // SNRFOR
                0.2, // SUBLT2
                0.2, // JRSUBUT2
                0.15 // PREFT1
            };
        }

    } // namespace { // file scope

    std::map<Seniority, Real> makeIsdaConvMap() {
        return RecoveryRateQuote::makeIsdaMap(
            RecoveryRateQuotePrivate::IsdaConvRecoveries);
    }

    Real RecoveryRateQuote::conventionalRecovery(Seniority sen) {
        return RecoveryRateQuotePrivate::IsdaConvRecoveries[sen];
    }

    RecoveryRateQuote::RecoveryRateQuote(Real value,
                                         Seniority seniority)
        : seniority_(seniority), recoveryRate_(value) {
            QL_REQUIRE(value == Null<Real>() ||
                (value >= 0. && value <= 1.),
                "Recovery value must be a fractional unit.");
    }

    Real RecoveryRateQuote::setValue(Real value) {
        Real diff = value-recoveryRate_;
        if (diff != 0.0) {
            recoveryRate_ = value;
            notifyObservers();
        }
        return diff;
    }

    void RecoveryRateQuote::reset() {
        setValue(Null<Real>());
        seniority_ = NoSeniority;
    }

}
