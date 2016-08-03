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
         Real variance, const boost::shared_ptr<StrikedTypePayoff>& payoff,
         bool knock_in)
    : spot_(spot), discount_(discount), dividendDiscount_(dividendDiscount),
      variance_(variance), knock_in_(knock_in) {

        QL_REQUIRE(spot_>0.0,
                   "positive spot value required");

        QL_REQUIRE(discount_>0.0,
                   "positive discount required");

        QL_REQUIRE(dividendDiscount_>0.0,
                   "positive dividend discount required");

        QL_REQUIRE(variance_>=0.0,
                   "negative variance not allowed");

        stdDev_ = std::sqrt(variance_);

        Option::Type type   = payoff->optionType();
        strike_ = payoff->strike();
        forward_ = spot_ * dividendDiscount_ / discount_;

        mu_ = std::log(dividendDiscount_/discount_)/variance_ - 0.5;

        // binary cash-or-nothing payoff?
        boost::shared_ptr<CashOrNothingPayoff> coo =
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(payoff);
        if (coo) {
            K_ = coo->cashPayoff();
        }

        // binary asset-or-nothing payoff?
        boost::shared_ptr<AssetOrNothingPayoff> aoo =
            boost::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff);
        if (aoo) {
            K_ = forward_;
            mu_ += 1.0;
        }


        log_H_S_ = std::log(strike_/spot_);
        Real log_S_H_ = std::log(spot_/strike_);

        double eta;
        double phi;
        switch (type) {
            case Option::Call:
                if (knock_in_) {
                   // up-and-in cash-(at-expiry)-or-nothing option
                   // a.k.a. american call with cash-or-nothing payoff
                   eta = -1.0;
                   phi =  1.0;
                } else {
                   // up-and-out cash-(at-expiry)-or-nothing option
				   eta = -1.0;
				   phi = -1.0;
                }
                break;
            case Option::Put:
               if (knock_in_) {
                   // down-and-in cash-(at-expiry)-or-nothing option
                   // a.k.a. american put with cash-or-nothing payoff
                   eta =  1.0;
                   phi = -1.0;
                } else {
                   // down-and-out cash-(at-expiry)-or-nothing option
				   eta =  1.0;
				   phi =  1.0;
                }
                break;
            default:
                QL_FAIL("invalid option type");
         }


        if (variance_>=QL_EPSILON) {
            D1_ = phi*(log_S_H_/stdDev_ + mu_*stdDev_);
            D2_ = eta*(log_H_S_/stdDev_ + mu_*stdDev_);

            CumulativeNormalDistribution f;
            cum_d1_ = f(D1_);
            cum_d2_ = f(D2_);
            n_d1_ = f.derivative(D1_);
            n_d2_ = f.derivative(D2_);
        } else {
            if (log_S_H_ * phi >0)
                cum_d1_= 1.0;
            else
                cum_d1_= 0.0;
            if (log_H_S_ * eta >0)
                cum_d2_= 1.0;
            else
                cum_d2_= 0.0;
            n_d1_ = 0.0;
            n_d2_ = 0.0;
        }


        switch (type) {
            case Option::Call:
                if (strike_<=spot_) {
                    if (knock_in_) {
                        // up-and-in cash-(at-expiry)-or-nothing option
                        // a.k.a. american call with cash-or-nothing payoff
                        cum_d1_     = 0.5;
                        cum_d2_     = 0.5;
                    } else {
                        // up-and-out cash-(at-expiry)-or-nothing option
                        // already knocked out
                        cum_d1_     = 0.0;
                        cum_d2_     = 0.0;
                    }
                    n_d1_       = 0.0;
                    n_d2_       = 0.0;
                }
                break;
            case Option::Put:
				if (strike_>=spot_) {
                    if (knock_in_) {
					    // down-and-in cash-(at-expiry)-or-nothing option
					    // a.k.a. american put with cash-or-nothing payoff
                        cum_d1_     = 0.5;
                        cum_d2_     = 0.5;
                    } else {
					    // down-and-out cash-(at-expiry)-or-nothing option
                        // already knocked out
                        cum_d1_     = 0.0;
                        cum_d2_     = 0.0;
                    }
                    n_d1_       = 0.0;
                    n_d2_       = 0.0;
                }
                break;
            default:
                QL_FAIL("invalid option type");
         }


        inTheMoney_ = (type==Option::Call && strike_<spot_) ||
                      (type==Option::Put  && strike_>spot_);
        if (inTheMoney_) {
            X_ = 1.0;
            Y_ = 1.0;
        } else {
            X_ = 1.0;
            if (cum_d2_ == 0.0)
                Y_ = 0.0; // check needed on some extreme cases
            else
                Y_ = std::pow(Real(strike_/spot_), Real(2.0*mu_));
        }
        if (!knock_in_)
           Y_ *= -1.0; 
    }

}

