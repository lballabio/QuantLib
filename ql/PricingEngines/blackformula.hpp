
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file blackformula.hpp
    \brief Black formula
*/

#ifndef quantlib_blackformula_h
#define quantlib_blackformula_h

#include <ql/Instruments/payoffs.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    class BlackFormula {
    public:
        BlackFormula(double forward,
                     double discount,
                     double variance,
                     const boost::shared_ptr<StrikedTypePayoff>& payoff);
        double value() const;
        double delta(double spot) const;
        double elasticity(double spot) const;
        double gamma(double spot) const;
        double deltaForward() const;
        double elasticityForward() const;
        double gammaForward() const;
        double theta(double spot, double maturity) const;
        double thetaPerDay(double spot, double maturity) const;
        double vega(double maturity) const;
        double rho(double maturity) const;
        double dividendRho(double maturity) const;
        /*! Probability of being in the money in the bond martingale measure.
            It is a risk-neutral probability, not the real world probability.
        */
        double itmCashProbability() const;
        /*! Probability of being in the money in the asset martingale measure.
            It is a risk-neutral probability, not the real world probability.
        */
        double itmAssetProbability() const;
        double strikeSensitivity() const;
    private:
        double forward_, discount_, variance_;
        double stdDev_, strike_;
        double D1_, D2_, alpha_, beta_, DalphaDd1_, DbetaDd2_;
        double cum_d1_, cum_d2_;
        double X_, DXDs_, DXDstrike_;
    };


    inline BlackFormula::BlackFormula(
                           double forward, double discount,
                           double variance, 
                           const boost::shared_ptr<StrikedTypePayoff>& payoff)
    : forward_(forward), discount_(discount), variance_(variance) {

        QL_REQUIRE(forward>0.0,
                   "BlackFormula::BlackFormula : "
                   "positive forward value required: " +
                   DoubleFormatter::toString(forward) +
                   " not allowed");

        QL_REQUIRE(discount>0.0,
                   "BlackFormula::BlackFormula : "
                   "positive discount required " +
                   DoubleFormatter::toString(discount) +
                   " not allowed");

        QL_REQUIRE(variance>=0.0,
                   "BlackFormula::BlackFormula : "
                   "non-negative variance required " +
                   DoubleFormatter::toString(variance) +
                   " not allowed");

        stdDev_ = QL_SQRT(variance);
        strike_ = payoff->strike();

        double n_d1, n_d2;
        if (variance>=QL_EPSILON) {
            if (strike_==0.0) {
                n_d1 = 0.0;
                n_d2 = 0.0;
                cum_d1_ = 1.0;
                cum_d2_= 1.0;
            } else {
                D1_ = (QL_LOG(forward/strike_) + 0.5*variance) / stdDev_;
                D2_ = D1_-stdDev_;
                CumulativeNormalDistribution f;
                cum_d1_ = f(D1_);
                cum_d2_= f(D2_);
                n_d1 = f.derivative(D1_);
                n_d2 = f.derivative(D2_);
            }
        } else {
            if (forward>strike_) {
                cum_d1_ = 1.0;
                cum_d2_= 1.0;
            } else {
                cum_d1_ = 0.0;
                cum_d2_= 0.0;
            }
            n_d1 = 0.0;
            n_d2 = 0.0;
        }

        X_ = strike_;
        DXDstrike_ = 1.0;

        // the following one will probably disappear as soon as
        // super-share will be properly handled
        DXDs_ = 0.0;

        // Plain Vanilla Payoff
        switch (payoff->optionType()) {
          case Option::Call:
            alpha_     =  cum_d1_;//  N(d1)
            DalphaDd1_ =    n_d1; //  n(d1)
            beta_      = -cum_d2_;// -N(d2)
            DbetaDd2_  = -  n_d2; // -n(d2)
            break;
          case Option::Put:
            alpha_     = -1.0+cum_d1_;// -N(-d1)
            DalphaDd1_ =        n_d1; //  n( d1)
            beta_      =  1.0-cum_d2_;//  N(-d2)
            DbetaDd2_  =     -  n_d2; // -n( d2)
            break;
          case Option::Straddle:
            // incorporating the linear effect of call + put
            alpha_     = -1.0 + 2.0*cum_d1_;//  N(d1) - N(-d1)
            DalphaDd1_ =        2.0*  n_d1; //  n(d1) + n( d1)
            beta_      =  1.0 - 2.0*cum_d2_;// -N(d2) + N(-d2)
            DbetaDd2_  =      - 2.0*  n_d2; // -n(d2) - n( d2)
            break;
          default:
            QL_FAIL("BlackFormula::BlackFormula : "
                    "invalid option type");
        }

        // binary cash-or-nothing payoff?
        boost::shared_ptr<CashOrNothingPayoff> coo =
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(payoff);
        if (coo) {
            // ok, the payoff is binary cash-or-nothing
            alpha_ = DalphaDd1_ = 0.0;
            X_ = coo->cashPayoff();
            DXDstrike_ = 0.0;
            switch (payoff->optionType()) {
              case Option::Call:
                beta_     = cum_d2_;// N(d2)
                DbetaDd2_ =   n_d2; // n(d2)
                break;
              case Option::Put:
                beta_     = 1.0-cum_d2_;//  N(-d2)
                DbetaDd2_ =    -  n_d2; // -n( d2)
                break;
              case Option::Straddle:
                // incorporating the linear effect of call + put
                beta_     = 1.0; // N(d2) + N(-d2) = 1.0
                DbetaDd2_ = 0.0; // n(d2) - n( d2) = 0.0
                break;
              default:
                QL_FAIL("BlackFormula::BlackFormula : "
                        "invalid option type");
            }
        }

        // binary asset-or-nothing payoff?
        boost::shared_ptr<AssetOrNothingPayoff> aoo =
            boost::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff);
        if (aoo) {
            // ok, the payoff is binary asset-or-nothing
            beta_ = DbetaDd2_ = 0.0;
            switch (payoff->optionType()) {
              case Option::Call:
                alpha_     =  cum_d1_;//  N(d1)
                DalphaDd1_ =    n_d1; //  n(d1)
                break;
              case Option::Put:
                alpha_     = 1.0-cum_d1_;//  N(-d1)
                DalphaDd1_ =    -  n_d1; // -n( d1)
                break;
              case Option::Straddle:
                // incorporating the linear effect of call + put
                alpha_     = 1.0; //  N(d1) + N(-d1) = 1.0
                DalphaDd1_ = 0.0; //  n(d1) - n( d1) = 0.0
                break;
              default:
                QL_FAIL("BlackFormula::BlackFormula : "
                        "invalid option type");
            }
        }

        // binary gap payoff?
        boost::shared_ptr<GapPayoff> gap =
            boost::dynamic_pointer_cast<GapPayoff>(payoff);
        if (gap) {
            // ok, the payoff is binary gap
            X_ = gap->strikePayoff();
            DXDstrike_ = 0.0;
        }

        // binary super-share payoff?
        boost::shared_ptr<SuperSharePayoff> ss =
            boost::dynamic_pointer_cast<SuperSharePayoff>(payoff);
        if (ss) {
            // ok, the payoff is binary super-share
            QL_FAIL("Binary super-share payoff not handled yet");
        }
    }




    inline double BlackFormula::value() const {
        double result = discount_ * (forward_ * alpha_ + X_ * beta_);
        // numerical inaccuracies can yield a negative answer
        if (result<0.0 && -1e-16 < result)
            result = 0.0;
        return result;
    }

    inline double BlackFormula::delta(double spot) const {

        QL_REQUIRE(spot>0.0,
            "BlackFormula::delta : "
            "positive spot value required");

        double DforwardDs = forward_ / spot;

        double temp = stdDev_*spot;
        double DalphaDs = DalphaDd1_/temp;
        double DbetaDs  = DbetaDd2_/temp;
        double temp2 = DalphaDs * forward_ + alpha_ * DforwardDs
                      +DbetaDs  * X_       + beta_  * DXDs_;

        return discount_ * temp2;
    }

    inline double BlackFormula::deltaForward() const {

        double temp = stdDev_*forward_;
        double DalphaDforward = DalphaDd1_/temp;
        double DbetaDforward  = DbetaDd2_/temp;
        double temp2 = DalphaDforward * forward_ + alpha_
                      +DbetaDforward  * X_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    //! Sensitivity in percent to a percent movement in the underlying price
    inline double BlackFormula::elasticity(double spot) const {
        double val = value();
        double del = delta(spot);
        if (val>QL_EPSILON)
            return del/val*spot;
        else if (QL_FABS(del)<QL_EPSILON)
            return 0.0;
        else if (del>0.0)
            return QL_MAX_DOUBLE;
        else
            return QL_MIN_DOUBLE;
    }

    //! Sensitivity in percent to a percent movement in the forward price
    inline double BlackFormula::elasticityForward() const {
        double val = value();
        double del = deltaForward();
        if (val>QL_EPSILON)
            return del/val*forward_;
        else if (QL_FABS(del)<QL_EPSILON)
            return 0.0;
        else if (del>0.0)
            return QL_MAX_DOUBLE;
        else
            return QL_MIN_DOUBLE;
    }

    inline double BlackFormula::gamma(double spot) const {

        QL_REQUIRE(spot>0.0,
            "BlackFormula::gamma : "
            "positive spot value required");

        double DforwardDs = forward_ / spot;

        double temp = stdDev_*spot;
        double DalphaDs = DalphaDd1_/temp;
        double DbetaDs  = DbetaDd2_/temp;

        double D2alphaDs2 = - DalphaDs/spot*(1+D1_/stdDev_);
        double D2betaDs2  = - DbetaDs /spot*(1+D2_/stdDev_);

        double temp2 = D2alphaDs2 * forward_ + 2.0 * DalphaDs * DforwardDs
                      +D2betaDs2  * X_       + 2.0 * DbetaDs  * DXDs_;

        return  discount_ * temp2;
    }

    inline double BlackFormula::gammaForward() const {

        double temp = stdDev_*forward_;
        double DalphaDforward = DalphaDd1_/temp;
        double DbetaDforward  = DbetaDd2_/temp;

        double D2alphaDforward2 = - DalphaDforward/forward_*(1+D1_/stdDev_);
        double D2betaDforward2  = - DbetaDforward /forward_*(1+D2_/stdDev_);

        double temp2 = D2alphaDforward2 * forward_ + 2.0 * DalphaDforward
                      +D2betaDforward2  * X_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    inline double BlackFormula::theta(double spot, double maturity) const {

        if (maturity>0.0) {
//            vol = stdDev_ / QL_SQRT(maturity);
//            rate = -QL_LOG(discount_)/maturity;
//            dividendRate = -QL_LOG(forward_ / spot * discount_)/maturity;
//            return rate*value() - (rate-dividendRate)*spot*delta(spot)
//                - 0.5*vol*vol*spot*spot*gamma(spot);
            return -( QL_LOG(discount_)            * value()
                     +QL_LOG(forward_/spot) * spot * delta(spot)
                     +0.5*variance_ * spot  * spot * gamma(spot))/maturity;
        } else if (maturity==0.0) {
            // should be r*value(), where r is the short rate
            // but the term structure is not available here
            QL_FAIL("BlackFormula::theta : "
                    "null maturity not handled");
        } else {
            QL_FAIL("BlackFormula::theta : "
                    "negative maturity not allowed");
        }


    }

    inline double BlackFormula::thetaPerDay(double spot, double maturity) const {
        return theta(spot, maturity)/365.0;
    }

    inline double BlackFormula::rho(double maturity) const {
        QL_REQUIRE(maturity>=0.0,
            "BlackFormula::rho : "
            "negative maturity not allowed");

        // actually DalphaDr / T
        double DalphaDr = DalphaDd1_/stdDev_;
        double DbetaDr  = DbetaDd2_/stdDev_;
        double temp = DalphaDr * forward_ + alpha_ * forward_ + DbetaDr * X_;

        return maturity * (discount_ * temp - value());
    }

    inline double BlackFormula::dividendRho(double maturity) const {
        QL_REQUIRE(maturity>=0.0,
            "BlackFormula::dividendRho : "
            "negative maturity not allowed");

        // actually DalphaDq / T
        double DalphaDq = -DalphaDd1_/stdDev_;
        double DbetaDq  = -DbetaDd2_/stdDev_;

        double temp = DalphaDq * forward_ - alpha_ * forward_ + DbetaDq * X_;

        return maturity * discount_ * temp;
            ;
    }

    inline double BlackFormula::vega(double maturity) const {
        QL_REQUIRE(maturity>=0.0,
            "BlackFormula::vega : "
            "negative maturity not allowed");

        double temp = QL_LOG(strike_/forward_)/variance_;
        // actually DalphaDsigma / SQRT(T)
        double DalphaDsigma = DalphaDd1_*(temp+0.5);
        double DbetaDsigma  = DbetaDd2_ *(temp-0.5);

        double temp2 = DalphaDsigma * forward_ + DbetaDsigma * X_;

        return discount_ * QL_SQRT(maturity) * temp2;

    }

    inline double BlackFormula::strikeSensitivity() const {

        double temp = stdDev_*strike_;
        double DalphaDstrike = -DalphaDd1_/temp;
        double DbetaDstrike  = -DbetaDd2_/temp;

        double temp2 =
            DalphaDstrike * forward_ + DbetaDstrike * X_ + beta_ * DXDstrike_;

        return discount_ * temp2;
    }

    inline double BlackFormula::itmCashProbability() const {
        return cum_d2_;
    }

    inline double BlackFormula::itmAssetProbability() const {
        return cum_d1_;
    }
}


#endif
