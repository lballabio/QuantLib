/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

#include <ql/pricingengines/americanpayoffatexpiry.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    AmericanPayoffAtExpiry::AmericanPayoffAtExpiry(
         Real spot, DiscountFactor discount, DiscountFactor dividendDiscount,
         Real variance, const boost::shared_ptr<StrikedTypePayoff>& payoff)
    : spot_(spot), discount_(discount), dividendDiscount_(dividendDiscount),
      variance_(variance) {

        QL_REQUIRE(spot_>0.0,
                   "positive spot_ value required");

        forward_ = spot_ * dividendDiscount_ / discount_;

        QL_REQUIRE(discount_>0.0,
                   "positive discount required");

        QL_REQUIRE(dividendDiscount_>0.0,
                   "positive dividend discount_ required");

        QL_REQUIRE(variance_>=0.0,
                   "negative variance_ not allowed");

        stdDev_ = std::sqrt(variance_);

        Option::Type type   = payoff->optionType();
        strike_ = payoff->strike();


        mu_ = std::log(dividendDiscount_/discount_)/variance_ - 0.5;

        // binary cash-or-nothing payoff?
        boost::shared_ptr<CashOrNothingPayoff> coo =
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(payoff);
        if (coo) {
            K_ = coo->cashPayoff();
            DKDstrike_ = 0.0;
        }

        // binary asset-or-nothing payoff?
        boost::shared_ptr<AssetOrNothingPayoff> aoo =
            boost::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff);
        if (aoo) {
            K_ = forward_;
            DKDstrike_ = 0.0;
            mu_ += 1.0;
        }


        log_H_S_ = std::log(strike_/spot_);

        Real n_d1, n_d2;
        if (variance_>=QL_EPSILON) {
            D1_ = log_H_S_/stdDev_ + mu_*stdDev_;
            D2_ = D1_ - 2.0*mu_*stdDev_;
            CumulativeNormalDistribution f;
            cum_d1_ = f(D1_);
            cum_d2_ = f(D2_);
            n_d1 = f.derivative(D1_);
            n_d2 = f.derivative(D2_);
        } else {
            if (log_H_S_>0) {
                cum_d1_= 1.0;
                cum_d2_= 1.0;
            } else {
                cum_d1_= 0.0;
                cum_d2_= 0.0;
            }
            n_d1 = 0.0;
            n_d2 = 0.0;
        }


        switch (type) {
            // up-and-in cash-(at-hit)-or-nothing option
            // a.k.a. american call with cash-or-nothing payoff
            case Option::Call:
                if (strike_>spot_) {
                    alpha_     = 1.0-cum_d2_;//  N(-d2)
                    DalphaDd1_ =    -  n_d2; // -n( d2)
                    beta_      = 1.0-cum_d1_;//  N(-d1)
                    DbetaDd2_  =    -  n_d1; // -n( d1)
                } else {
                    alpha_     = 0.5;
                    DalphaDd1_ = 0.0;
                    beta_      = 0.5;
                    DbetaDd2_  = 0.0;
                }
                break;
            // down-and-in cash-(at-hit)-or-nothing option
            // a.k.a. american put with cash-or-nothing payoff
            case Option::Put:
                if (strike_<spot_) {
                    alpha_     =     cum_d2_;//  N(d2)
                    DalphaDd1_ =       n_d2; //  n(d2)
                    beta_      =     cum_d1_;//  N(d1)
                    DbetaDd2_  =       n_d1; //  n(d1)
                } else {
                    alpha_     = 0.5;
                    DalphaDd1_ = 0.0;
                    beta_      = 0.5;
                    DbetaDd2_  = 0.0;
                }
                break;
            default:
                QL_FAIL("invalid option type");
         }


        inTheMoney_ = (type==Option::Call && strike_<spot_) ||
                      (type==Option::Put  && strike_>spot_);
        if (inTheMoney_) {
            Y_         = 1.0;
            X_         = 1.0;
            DYDstrike_ = 0.0;
            DXDstrike_ = 0.0;
        } else {
            Y_ = 1.0;
            X_ = std::pow(Real(strike_/spot_), Real(2.0*mu_));
//            DXDstrike_ = ......;
        }

    }

}

