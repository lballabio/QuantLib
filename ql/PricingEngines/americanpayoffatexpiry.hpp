
/*
 Copyright (C) 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file americanpayoffatexpiry.hpp
    \brief Analytical formulae for american exercise with payoff at expiry
*/

#ifndef quantlib_americanpayoffatexpiry_h
#define quantlib_americanpayoffatexpiry_h

#include <ql/Instruments/payoffs.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    /*! Analytic formula for American exercise payoff at-expiry options

        \todo calculate greeks
    */
    class AmericanPayoffAtExpiry {
    public:
        AmericanPayoffAtExpiry(double spot,
                               double discount,
                               double dividendDiscount,
                               double variance,
                               const Handle<StrikedTypePayoff>& payoff);
        double value() const;
    private:
        double spot_, discount_, dividendDiscount_, variance_;
        double forward_, stdDev_;

        double strike_, K_, DKDstrike_;

        double mu_, log_H_S_;

        double D1_, D2_, cum_d1_, cum_d2_;

        double alpha_, beta_, DalphaDd1_, DbetaDd2_;

        bool inTheMoney_;
        double Y_, DYDstrike_, X_, DXDstrike_;
    };


    inline AmericanPayoffAtExpiry::AmericanPayoffAtExpiry(double spot,
        double discount, double dividendDiscount, double variance,
        const Handle<StrikedTypePayoff>& payoff)
    : spot_(spot), discount_(discount), dividendDiscount_(dividendDiscount),
      variance_(variance) {

        QL_REQUIRE(spot_>0.0,
            "AmericanPayoffAtExpiry::AmericanPayoffAtExpiry : "
            "positive spot_ value required");

        forward_ = spot_ * dividendDiscount_ / discount_;

        QL_REQUIRE(discount_>0.0,
            "AmericanPayoffAtExpiry::AmericanPayoffAtExpiry : "
            "positive discount required");

        QL_REQUIRE(dividendDiscount_>0.0,
            "AmericanPayoffAtExpiry::AmericanPayoffAtExpiry : "
            "positive dividend discount_ required");

        QL_REQUIRE(variance_>=0.0,
            "AmericanPayoffAtExpiry::AmericanPayoffAtExpiry : "
            "negative variance_ not allowed");

        stdDev_ = QL_SQRT(variance_);

        Option::Type type   = payoff->optionType();
        strike_ = payoff->strike();


        mu_ = QL_LOG(dividendDiscount_/discount_)/variance_ - 0.5;

        // binary cash-or-nothing payoff?
        Handle<CashOrNothingPayoff> coo =
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(payoff);
        if (!IsNull(coo)) {
            K_ = coo->cashPayoff();
            DKDstrike_ = 0.0;
        }

        // binary asset-or-nothing payoff?
        Handle<AssetOrNothingPayoff> aoo =
            boost::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff);
        if (!IsNull(aoo)) {
            K_ = forward_;
            DKDstrike_ = 0.0;
            mu_ += 1.0;
        }


        log_H_S_ = QL_LOG (strike_/spot_);

        double n_d1, n_d2;
        double cum_d1_, cum_d2_;
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
            case Option::Straddle:
                // incorporating the linear effect of call + put
                alpha_     = 1.0; //  N(-d2) + N(d2)
                DalphaDd1_ = 0.0; // -n( d2) + n(d2)
                beta_      = 1.0; //  N(-d1) + N(d1)
                DbetaDd2_  = 0.0; // -n( d1) + n(d1)
                break;
            default:
                QL_FAIL("AnalyticAmericanEngine::calculate() :"
                        "invalid option type");
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
            X_ = QL_POW(strike_/spot_, 2.0*mu_);
//            DXDstrike_ = ......;
        }

    }


    inline double AmericanPayoffAtExpiry::value() const {
        return discount_ * K_ * (Y_ * alpha_ + X_ * beta_);
    }

}


#endif
