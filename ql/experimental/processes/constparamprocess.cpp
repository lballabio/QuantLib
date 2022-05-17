/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Ruilong Xu

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

#include <ql/experimental/processes/constparamprocess.hpp>

namespace QuantLib {
    void ConstParam<VegaStressedBlackScholesProcess>::fetchValue() const {
        if (fetched_ == false) {
            ext::shared_ptr<FlatForward> flatDividendTS =
                ext::dynamic_pointer_cast<FlatForward>(dividendYield_.currentLink());
            QL_REQUIRE(flatDividendTS != nullptr,
                       "flat dividend yield term structure is required in ConstParam");
            QL_REQUIRE(flatDividendTS->compounding() != SimpleThenCompounded &&
                           flatDividendTS->compounding() != CompoundedThenSimple,
                       "dividend yield term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            ext::shared_ptr<FlatForward> flatRiskFreeTS =
                ext::dynamic_pointer_cast<FlatForward>(riskFreeRate_.currentLink());
            QL_REQUIRE(flatRiskFreeTS != nullptr,
                       "flat risk free rate term structure is required in ConstParam");
            QL_REQUIRE(flatRiskFreeTS->compounding() != SimpleThenCompounded &&
                           flatRiskFreeTS->compounding() != CompoundedThenSimple,
                       "risk free rate term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            ext::shared_ptr<BlackConstantVol> flatBlackVol =
                ext::dynamic_pointer_cast<BlackConstantVol>(blackVolatility_.currentLink());
            QL_REQUIRE(flatBlackVol != nullptr,
                       "flat volatility term structure is required in ConstParam");

            x0Value_ = x0_->value();

            static Real t1 = 0.0, t2 = 1.0;
            dividendYieldValue_ =
                flatDividendTS->forwardRate(t1, t2, Continuous, NoFrequency, true);
            riskFreeRateValue_ = flatRiskFreeTS->forwardRate(t1, t2, Continuous, NoFrequency, true);
            blackVolatilityValue_ = flatBlackVol->blackVol(0.0, x0_->value());

            carryCost_ = riskFreeRateValue_ - dividendYieldValue_;
            fetched_ = true;
        } else {
            return;
        }
    }

    Real ConstParam<VegaStressedBlackScholesProcess>::diffusion(Time t, Real x) const {
        fetchValue();
        if (lowerTimeBorderForStressTest_ <= t && t <= upperTimeBorderForStressTest_ &&
            lowerAssetBorderForStressTest_ <= x && x <= upperAssetBorderForStressTest_) {
            return blackVolatilityValue_ + stressLevel_;
        } else {
            return blackVolatilityValue_;
        }
    }
}

namespace QuantLib {
    void ConstParam<HestonProcess>::fetchValue() const {
        if (fetched_ == false) {
            ext::shared_ptr<FlatForward> flatDividendTS =
                ext::dynamic_pointer_cast<FlatForward>(dividendYield_.currentLink());
            QL_REQUIRE(flatDividendTS != nullptr,
                       "flat dividend yield term structure is required in ConstParam");
            QL_REQUIRE(flatDividendTS->compounding() != SimpleThenCompounded &&
                           flatDividendTS->compounding() != CompoundedThenSimple,
                       "dividend yield term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            ext::shared_ptr<FlatForward> flatRiskFreeTS =
                ext::dynamic_pointer_cast<FlatForward>(riskFreeRate_.currentLink());
            QL_REQUIRE(flatRiskFreeTS != nullptr,
                       "flat risk free rate term structure is required in ConstParam");
            QL_REQUIRE(flatRiskFreeTS->compounding() != SimpleThenCompounded &&
                           flatRiskFreeTS->compounding() != CompoundedThenSimple,
                       "risk free rate term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            static Real t1 = 0.0, t2 = 1.0;
            dividendYieldValue_ = flatDividendTS->forwardRate(t1, t2, Continuous);
            riskFreeRateValue_ = flatRiskFreeTS->forwardRate(t1, t2, Continuous);

            carryCost_ = riskFreeRateValue_ - dividendYieldValue_;
            fetched_ = true;
        } else {
            return;
        }
    }

    void ConstParam<BatesProcess>::fetchValue() const {
        if (fetched_ == false) {
            ext::shared_ptr<FlatForward> flatDividendTS =
                ext::dynamic_pointer_cast<FlatForward>(dividendYield_.currentLink());
            QL_REQUIRE(flatDividendTS != nullptr,
                       "flat dividend yield term structure is required in ConstParam");
            QL_REQUIRE(flatDividendTS->compounding() != SimpleThenCompounded &&
                           flatDividendTS->compounding() != CompoundedThenSimple,
                       "dividend yield term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            ext::shared_ptr<FlatForward> flatRiskFreeTS =
                ext::dynamic_pointer_cast<FlatForward>(riskFreeRate_.currentLink());
            QL_REQUIRE(flatRiskFreeTS != nullptr,
                       "flat risk free rate term structure is required in ConstParam");
            QL_REQUIRE(flatRiskFreeTS->compounding() != SimpleThenCompounded &&
                           flatRiskFreeTS->compounding() != CompoundedThenSimple,
                       "risk free rate term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            static Real t1 = 0.0, t2 = 1.0;
            dividendYieldValue_ = flatDividendTS->forwardRate(t1, t2, Continuous);
            riskFreeRateValue_ = flatRiskFreeTS->forwardRate(t1, t2, Continuous);

            carryCost_ = riskFreeRateValue_ - dividendYieldValue_;
            fetched_ = true;
        } else {
            return;
        }
    }
}

namespace QuantLib {
    void ConstParam<GJRGARCHProcess>::fetchValue() const {
        if (fetched_ == false) {
            ext::shared_ptr<FlatForward> flatDividendTS =
                ext::dynamic_pointer_cast<FlatForward>(dividendYield_.currentLink());
            QL_REQUIRE(flatDividendTS != nullptr,
                       "flat dividend yield term structure is required in ConstParam");
            QL_REQUIRE(flatDividendTS->compounding() != SimpleThenCompounded &&
                           flatDividendTS->compounding() != CompoundedThenSimple,
                       "dividend yield term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            ext::shared_ptr<FlatForward> flatRiskFreeTS =
                ext::dynamic_pointer_cast<FlatForward>(riskFreeRate_.currentLink());
            QL_REQUIRE(flatRiskFreeTS != nullptr,
                       "flat risk free rate term structure is required in ConstParam");
            QL_REQUIRE(flatRiskFreeTS->compounding() != SimpleThenCompounded &&
                           flatRiskFreeTS->compounding() != CompoundedThenSimple,
                       "risk free rate term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            static Real t1 = 0.0, t2 = 1.0;
            dividendYieldValue_ = flatDividendTS->forwardRate(t1, t2, Continuous);
            riskFreeRateValue_ = flatRiskFreeTS->forwardRate(t1, t2, Continuous);

            carryCost_ = riskFreeRateValue_ - dividendYieldValue_;
            fetched_ = true;
        } else {
            return;
        }
    }
}