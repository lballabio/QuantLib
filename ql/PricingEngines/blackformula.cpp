/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    class BlackFormula::Calculator : public AcyclicVisitor,
                                     public Visitor<Payoff>,
                                     public Visitor<PlainVanillaPayoff>,
                                     public Visitor<CashOrNothingPayoff>,
                                     public Visitor<AssetOrNothingPayoff>,
                                     public Visitor<GapPayoff>,
                                     public Visitor<SuperSharePayoff> {
      private:
        BlackFormula& black_;
      public:
        Calculator(BlackFormula& black) : black_(black) {}
        void visit(Payoff&);
        void visit(PlainVanillaPayoff&);
        void visit(CashOrNothingPayoff&);
        void visit(AssetOrNothingPayoff&);
        void visit(GapPayoff&);
        void visit(SuperSharePayoff&);
    };


    BlackFormula::BlackFormula(
                           Real forward, DiscountFactor discount,
                           Real variance,
                           const boost::shared_ptr<StrikedTypePayoff>& payoff)
    : forward_(forward), discount_(discount), variance_(variance) {

        QL_REQUIRE(forward>0.0,
                   "positive forward value required: "
                   << forward << " not allowed");

        QL_REQUIRE(discount>0.0,
                   "positive discount required: "
                   << discount << " not allowed");

        QL_REQUIRE(variance>=0.0,
                   "non-negative variance required: "
                   << variance << " not allowed");

        stdDev_ = std::sqrt(variance);
        strike_ = payoff->strike();

        if (variance>=QL_EPSILON) {
            if (strike_==0.0) {
                n_d1_ = 0.0;
                n_d2_ = 0.0;
                cum_d1_ = 1.0;
                cum_d2_= 1.0;
            } else {
                D1_ = (std::log(forward/strike_) + 0.5*variance) / stdDev_;
                D2_ = D1_-stdDev_;
                CumulativeNormalDistribution f;
                cum_d1_ = f(D1_);
                cum_d2_= f(D2_);
                n_d1_ = f.derivative(D1_);
                n_d2_ = f.derivative(D2_);
            }
        } else {
            if (forward>strike_) {
                cum_d1_ = 1.0;
                cum_d2_= 1.0;
            } else {
                cum_d1_ = 0.0;
                cum_d2_= 0.0;
            }
            n_d1_ = 0.0;
            n_d2_ = 0.0;
        }

        X_ = strike_;
        DXDstrike_ = 1.0;

        // the following one will probably disappear as soon as
        // super-share will be properly handled
        DXDs_ = 0.0;

        // this part is always executed.
        // in case of plain-vanilla payoffs, it is also the only part
        // which is executed.
        switch (payoff->optionType()) {
          case Option::Call:
            alpha_     =  cum_d1_;//  N(d1)
            DalphaDd1_ =    n_d1_;//  n(d1)
            beta_      = -cum_d2_;// -N(d2)
            DbetaDd2_  = -  n_d2_;// -n(d2)
            break;
          case Option::Put:
            alpha_     = -1.0+cum_d1_;// -N(-d1)
            DalphaDd1_ =        n_d1_;//  n( d1)
            beta_      =  1.0-cum_d2_;//  N(-d2)
            DbetaDd2_  =     -  n_d2_;// -n( d2)
            break;
          default:
            QL_FAIL("invalid option type");
        }

        // now dispatch on type.

        Calculator calc(*this);
        payoff->accept(calc);
    }

    void BlackFormula::Calculator::visit(Payoff&) {
        QL_FAIL("unsupported payoff type");
    }

    void BlackFormula::Calculator::visit(PlainVanillaPayoff&) {}

    void BlackFormula::Calculator::visit(CashOrNothingPayoff& payoff) {
        black_.alpha_ = black_.DalphaDd1_ = 0.0;
        black_.X_ = payoff.cashPayoff();
        black_.DXDstrike_ = 0.0;
        switch (payoff.optionType()) {
          case Option::Call:
            black_.beta_     = black_.cum_d2_;
            black_.DbetaDd2_ = black_.n_d2_;
            break;
          case Option::Put:
            black_.beta_     = 1.0-black_.cum_d2_;
            black_.DbetaDd2_ =    -black_.n_d2_;
            break;
          default:
            QL_FAIL("invalid option type");
        }
    }

    void BlackFormula::Calculator::visit(AssetOrNothingPayoff& payoff) {
        black_.beta_ = black_.DbetaDd2_ = 0.0;
        switch (payoff.optionType()) {
          case Option::Call:
            black_.alpha_     = black_.cum_d1_;
            black_.DalphaDd1_ = black_.n_d1_;
            break;
          case Option::Put:
            black_.alpha_     = 1.0-black_.cum_d1_;
            black_.DalphaDd1_ = -black_.n_d1_;
            break;
          default:
            QL_FAIL("invalid option type");
        }
    }

    void BlackFormula::Calculator::visit(GapPayoff& payoff) {
        black_.X_ = payoff.strikePayoff();
        black_.DXDstrike_ = 0.0;
    }

    void BlackFormula::Calculator::visit(SuperSharePayoff& payoff) {
        QL_FAIL("binary super-share payoff not handled yet");
    }

    Real BlackFormula::value() const {
        Real result = discount_ * (forward_ * alpha_ + X_ * beta_);
        // numerical inaccuracies can yield a negative answer
        if (result<0.0 && -1e-16 < result)
            result = 0.0;
        return result;
    }

    Real BlackFormula::delta(Real spot) const {

        QL_REQUIRE(spot > 0.0,
                   "positive spot value required");

        Real DforwardDs = forward_ / spot;

        Real temp = stdDev_*spot;
        Real DalphaDs = DalphaDd1_/temp;
        Real DbetaDs  = DbetaDd2_/temp;
        Real temp2 = DalphaDs * forward_ + alpha_ * DforwardDs
                      +DbetaDs  * X_       + beta_  * DXDs_;

        return discount_ * temp2;
    }

    Real BlackFormula::deltaForward() const {

        Real temp = stdDev_*forward_;
        Real DalphaDforward = DalphaDd1_/temp;
        Real DbetaDforward  = DbetaDd2_/temp;
        Real temp2 = DalphaDforward * forward_ + alpha_
                      +DbetaDforward  * X_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    Real BlackFormula::elasticity(Real spot) const {
        Real val = value();
        Real del = delta(spot);
        if (val>QL_EPSILON)
            return del/val*spot;
        else if (std::fabs(del)<QL_EPSILON)
            return 0.0;
        else if (del>0.0)
            return QL_MAX_REAL;
        else
            return QL_MIN_REAL;
    }

    Real BlackFormula::elasticityForward() const {
        Real val = value();
        Real del = deltaForward();
        if (val>QL_EPSILON)
            return del/val*forward_;
        else if (std::fabs(del)<QL_EPSILON)
            return 0.0;
        else if (del>0.0)
            return QL_MAX_REAL;
        else
            return QL_MIN_REAL;
    }

    Real BlackFormula::gamma(Real spot) const {

        QL_REQUIRE(spot>0.0,
                   "positive spot value required");

        Real DforwardDs = forward_ / spot;

        Real temp = stdDev_*spot;
        Real DalphaDs = DalphaDd1_/temp;
        Real DbetaDs  = DbetaDd2_/temp;

        Real D2alphaDs2 = - DalphaDs/spot*(1+D1_/stdDev_);
        Real D2betaDs2  = - DbetaDs /spot*(1+D2_/stdDev_);

        Real temp2 = D2alphaDs2 * forward_ + 2.0 * DalphaDs * DforwardDs
                      +D2betaDs2  * X_       + 2.0 * DbetaDs  * DXDs_;

        return  discount_ * temp2;
    }

    Real BlackFormula::gammaForward() const {

        Real temp = stdDev_*forward_;
        Real DalphaDforward = DalphaDd1_/temp;
        Real DbetaDforward  = DbetaDd2_/temp;

        Real D2alphaDforward2 = - DalphaDforward/forward_*(1+D1_/stdDev_);
        Real D2betaDforward2  = - DbetaDforward /forward_*(1+D2_/stdDev_);

        Real temp2 = D2alphaDforward2 * forward_ + 2.0 * DalphaDforward
                      +D2betaDforward2  * X_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    Real BlackFormula::theta(Real spot, Time maturity) const {

        if (maturity>0.0) {
//            vol = stdDev_ / std::sqrt(maturity);
//            rate = -std::log(discount_)/maturity;
//            dividendRate = -std::log(forward_ / spot * discount_)/maturity;
//            return rate*value() - (rate-dividendRate)*spot*delta(spot)
//                - 0.5*vol*vol*spot*spot*gamma(spot);
            return -( std::log(discount_)            * value()
                     +std::log(forward_/spot) * spot * delta(spot)
                     +0.5*variance_ * spot  * spot * gamma(spot))/maturity;
        } else if (maturity==0.0) {
            // should be r*value(), where r is the short rate
            // but the term structure is not available here
            QL_FAIL("null maturity not handled");
        } else {
            QL_FAIL("negative maturity not allowed");
        }
    }

    Real BlackFormula::thetaPerDay(Real spot, Time maturity) const {
        return theta(spot, maturity)/365.0;
    }

    Real BlackFormula::vega(Time maturity) const {
        QL_REQUIRE(maturity>=0.0,
                   "negative maturity not allowed");

        Real temp = std::log(strike_/forward_)/variance_;
        // actually DalphaDsigma / SQRT(T)
        Real DalphaDsigma = DalphaDd1_*(temp+0.5);
        Real DbetaDsigma  = DbetaDd2_ *(temp-0.5);

        Real temp2 = DalphaDsigma * forward_ + DbetaDsigma * X_;

        return discount_ * std::sqrt(maturity) * temp2;

    }

    Real BlackFormula::rho(Time maturity) const {
        QL_REQUIRE(maturity>=0.0,
                   "negative maturity not allowed");

        // actually DalphaDr / T
        Real DalphaDr = DalphaDd1_/stdDev_;
        Real DbetaDr  = DbetaDd2_/stdDev_;
        Real temp = DalphaDr * forward_ + alpha_ * forward_ + DbetaDr * X_;

        return maturity * (discount_ * temp - value());
    }

    Real BlackFormula::dividendRho(Time maturity) const {
        QL_REQUIRE(maturity>=0.0,
                   "negative maturity not allowed");

        // actually DalphaDq / T
        Real DalphaDq = -DalphaDd1_/stdDev_;
        Real DbetaDq  = -DbetaDd2_/stdDev_;

        Real temp = DalphaDq * forward_ - alpha_ * forward_ + DbetaDq * X_;

        return maturity * discount_ * temp;
    }

    Real BlackFormula::itmCashProbability() const {
        return cum_d2_;
    }

    Real BlackFormula::itmAssetProbability() const {
        return cum_d1_;
    }

    Real BlackFormula::strikeSensitivity() const {

        Real temp = stdDev_*strike_;
        Real DalphaDstrike = -DalphaDd1_/temp;
        Real DbetaDstrike  = -DbetaDd2_/temp;

        Real temp2 =
            DalphaDstrike * forward_ + DbetaDstrike * X_ + beta_ * DXDstrike_;

        return discount_ * temp2;
    }

    Real BlackFormula::alpha() const {
        return alpha_;
    }

    Real BlackFormula::beta() const {
        return beta_;
    }

}

