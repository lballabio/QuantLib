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

#include <ql/pricingengines/americanpayoffathit.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    AmericanPayoffAtHit::AmericanPayoffAtHit(
         Real spot, DiscountFactor discount, DiscountFactor dividendDiscount,
         Real variance, const std::shared_ptr<StrikedTypePayoff>& payoff)
    : spot_(spot), discount_(discount), dividendDiscount_(dividendDiscount),
      variance_(variance) {

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


        log_H_S_ = std::log(strike_/spot_);

        Real n_d1, n_d2;
        if (variance_>=QL_EPSILON) {
            if (discount_==0.0 && dividendDiscount_==0.0) {
                mu_     = - 0.5;
                lambda_ = 0.5;
            } else if (discount_==0.0) {
                QL_FAIL("null discount not handled yet");
            } else {
                mu_ = std::log(dividendDiscount_/discount_)/variance_ - 0.5;
                lambda_ = std::sqrt(mu_*mu_-2.0*std::log(discount_)/variance_);
            }
            D1_ = log_H_S_/stdDev_ + lambda_*stdDev_;
            D2_ = D1_ - 2.0*lambda_*stdDev_;
            CumulativeNormalDistribution f;
            cum_d1_ = f(D1_);
            cum_d2_ = f(D2_);
            n_d1 = f.derivative(D1_);
            n_d2 = f.derivative(D2_);
        } else {
            // not tested yet
            mu_ = std::log(dividendDiscount_/discount_)/variance_ - 0.5;
            lambda_ = std::sqrt(mu_*mu_-2.0*std::log(discount_)/variance_);
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
                    alpha_     = 1.0-cum_d1_;//  N(-d1)
                    DalphaDd1_ =    -  n_d1;// -n( d1)
                    beta_      = 1.0-cum_d2_;//  N(-d2)
                    DbetaDd2_  =    -  n_d2;// -n( d2)
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
                    alpha_     =     cum_d1_;//  N(d1)
                    DalphaDd1_ =       n_d1;//  n(d1)
                    beta_      =     cum_d2_;//  N(d2)
                    DbetaDd2_  =       n_d2;//  n(d2)
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


        muPlusLambda_  = mu_ + lambda_;
        muMinusLambda_ = mu_ - lambda_;
        inTheMoney_ = (type==Option::Call && strike_<spot_) ||
                      (type==Option::Put  && strike_>spot_);

        if (inTheMoney_) {
            forward_   = 1.0;
            X_         = 1.0;
            DXDstrike_ = 0.0;
        } else {
            forward_   = std::pow(strike_/spot_, muPlusLambda_);
            X_         = std::pow(strike_/spot_, muMinusLambda_);
//            DXDstrike_ = ......;
        }


        // Binary Cash-Or-Nothing payoff?
        std::shared_ptr<CashOrNothingPayoff> coo =
            std::dynamic_pointer_cast<CashOrNothingPayoff>(payoff);
        if (coo != nullptr) {
            K_ = coo->cashPayoff();
            DKDstrike_ = 0.0;
        }

        // Binary Asset-Or-Nothing payoff?
        std::shared_ptr<AssetOrNothingPayoff> aoo =
            std::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff);
        if (aoo != nullptr) {
            if (inTheMoney_) {
                K_ = spot_;
                DKDstrike_ = 0.0;
            } else {
                K_ = aoo->strike();
                DKDstrike_ = 1.0;
            }
        }
    }


    Real AmericanPayoffAtHit::delta() const {
        Real tempDelta = - spot_ * stdDev_;
        Real DalphaDs = DalphaDd1_/tempDelta;
        Real DbetaDs  = DbetaDd2_/tempDelta;

        Real DforwardDs, DXDs;
        if (inTheMoney_) {
            DforwardDs = 0.0;
            DXDs       = 0.0;
        } else {
            DforwardDs = -muPlusLambda_  * forward_ / spot_;
            DXDs       = -muMinusLambda_ * X_       / spot_;
        }

        return K_ * (
              DalphaDs * forward_ + alpha_ * DforwardDs
            + DbetaDs  * X_       + beta_  * DXDs
            );
    }


    Real AmericanPayoffAtHit::gamma() const {
        Real tempDelta = - spot_ * stdDev_;
        Real DalphaDs = DalphaDd1_/tempDelta;
        Real DbetaDs  = DbetaDd2_/tempDelta;
        Real D2alphaDs2 = -DalphaDs/spot_*(1-D1_/stdDev_);
        Real D2betaDs2  = -DbetaDs /spot_*(1-D2_/stdDev_);

        Real DforwardDs, DXDs, D2forwardDs2, D2XDs2;
        if (inTheMoney_) {
            DforwardDs = 0.0;
            DXDs       = 0.0;
            D2forwardDs2 = 0.0;
            D2XDs2       = 0.0;
        } else {
            DforwardDs = -muPlusLambda_  * forward_ / spot_;
            DXDs       = -muMinusLambda_ * X_       / spot_;
            D2forwardDs2 = muPlusLambda_  * forward_ / (spot_*spot_)*(1+muPlusLambda_);
            D2XDs2       = muMinusLambda_ * X_       / (spot_*spot_)*(1+muMinusLambda_);
        }

        return K_ * (
              D2alphaDs2 * forward_   + DalphaDs * DforwardDs
            + DalphaDs   * DforwardDs + alpha_   * D2forwardDs2
            + D2betaDs2  * X_         + DbetaDs  * DXDs
            + DbetaDs    * DXDs       + beta_    * D2XDs2
            );

    }


    Real AmericanPayoffAtHit::rho(Time maturity) const {
        QL_REQUIRE(maturity>=0.0,
                   "negative maturity not allowed");

        // actually D.Dr / T
        Real DalphaDr = -DalphaDd1_/(lambda_*stdDev_) * (1.0 + mu_);
        Real DbetaDr  =  DbetaDd2_ /(lambda_*stdDev_) * (1.0 + mu_);
        Real DforwardDr, DXDr;
        if (inTheMoney_) {
            DforwardDr = 0.0;
            DXDr       = 0.0;
        } else {
            DforwardDr = forward_ * (1.0+(1.0+mu_)/lambda_) * log_H_S_ / variance_;
            DXDr       = X_       * (1.0-(1.0+mu_)/lambda_) * log_H_S_ / variance_;
        }

        return maturity * K_ * (
              DalphaDr * forward_
            + alpha_   * DforwardDr
            + DbetaDr  * X_
            + beta_    * DXDr
            );
    }

}

