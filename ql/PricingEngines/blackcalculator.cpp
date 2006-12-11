/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2006 Ferdinando Ametrano
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

#include <ql/PricingEngines/blackcalculator.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    class BlackCalculator::Calculator : public AcyclicVisitor,
                                        public Visitor<Payoff>,
                                        public Visitor<PlainVanillaPayoff>,
                                        public Visitor<CashOrNothingPayoff>,
                                        public Visitor<AssetOrNothingPayoff>,
                                        public Visitor<GapPayoff> {
      private:
        BlackCalculator& black_;
      public:
        Calculator(BlackCalculator& black) : black_(black) {}
        void visit(Payoff&);
        void visit(PlainVanillaPayoff&);
        void visit(CashOrNothingPayoff&);
        void visit(AssetOrNothingPayoff&);
        void visit(GapPayoff&);
    };


    #ifndef QL_DISABLE_DEPRECATED
    BlackCalculator::BlackCalculator(Real forward, DiscountFactor discount,
        Real variance, const boost::shared_ptr<StrikedTypePayoff>& payoff)
    : strike_(payoff->strike()), forward_(forward),
      stdDev_(std::sqrt(variance)), discount_(discount),
      variance_(variance) {

        QL_REQUIRE(forward>0.0, "positive forward value required: " <<
                   forward << " not allowed");

        QL_REQUIRE(discount>0.0, "positive discount required: " <<
                   discount << " not allowed");

        QL_REQUIRE(variance>=0.0, "non-negative variance required: " <<
                   variance << " not allowed");

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
    #endif

    BlackCalculator::BlackCalculator(
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        Real forward, Real stdDev, Real discount)
    : strike_(payoff->strike()), forward_(forward), stdDev_(stdDev),
      discount_(discount), variance_(stdDev*stdDev) {

        QL_REQUIRE(forward>0.0, "positive forward value required: " <<
                   forward << " not allowed");

        QL_REQUIRE(stdDev>=0.0, "non-negative standard deviation required: "
                   << stdDev << " not allowed");

        QL_REQUIRE(discount>0.0, "positive discount required: " <<
                   discount << " not allowed");

        if (stdDev_>=QL_EPSILON) {
            if (strike_==0.0) {
                n_d1_ = 0.0;
                n_d2_ = 0.0;
                cum_d1_ = 1.0;
                cum_d2_= 1.0;
            } else {
                D1_ = std::log(forward/strike_)/stdDev_ + 0.5*stdDev_;
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

    void BlackCalculator::Calculator::visit(Payoff& p) {
        QL_FAIL("unsupported payoff type: " << p.name());
    }

    void BlackCalculator::Calculator::visit(PlainVanillaPayoff&) {}

    void BlackCalculator::Calculator::visit(CashOrNothingPayoff& payoff) {
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

    void BlackCalculator::Calculator::visit(AssetOrNothingPayoff& payoff) {
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

    void BlackCalculator::Calculator::visit(GapPayoff& payoff) {
        black_.X_ = payoff.secondStrike();
        black_.DXDstrike_ = 0.0;
    }

    Real BlackCalculator::value() const {
        Real result = discount_ * (forward_ * alpha_ + X_ * beta_);
        return result;
    }

    Real BlackCalculator::delta(Real spot) const {

        QL_REQUIRE(spot > 0.0, "positive spot value required: " <<
                   spot << " not allowed");

        Real DforwardDs = forward_ / spot;

        Real temp = stdDev_*spot;
        Real DalphaDs = DalphaDd1_/temp;
        Real DbetaDs  = DbetaDd2_/temp;
        Real temp2 = DalphaDs * forward_ + alpha_ * DforwardDs
                      +DbetaDs  * X_       + beta_  * DXDs_;

        return discount_ * temp2;
    }

    Real BlackCalculator::deltaForward() const {

        Real temp = stdDev_*forward_;
        Real DalphaDforward = DalphaDd1_/temp;
        Real DbetaDforward  = DbetaDd2_/temp;
        Real temp2 = DalphaDforward * forward_ + alpha_
                      +DbetaDforward  * X_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    Real BlackCalculator::elasticity(Real spot) const {
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

    Real BlackCalculator::elasticityForward() const {
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

    Real BlackCalculator::gamma(Real spot) const {

        QL_REQUIRE(spot > 0.0, "positive spot value required: " <<
                   spot << " not allowed");

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

    Real BlackCalculator::gammaForward() const {

        Real temp = stdDev_*forward_;
        Real DalphaDforward = DalphaDd1_/temp;
        Real DbetaDforward  = DbetaDd2_/temp;

        Real D2alphaDforward2 = - DalphaDforward/forward_*(1+D1_/stdDev_);
        Real D2betaDforward2  = - DbetaDforward /forward_*(1+D2_/stdDev_);

        Real temp2 = D2alphaDforward2 * forward_ + 2.0 * DalphaDforward
                      +D2betaDforward2  * X_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    Real BlackCalculator::theta(Real spot, Time maturity) const {

        if (maturity==0.0) return 0.0;
        QL_REQUIRE(maturity>0.0, "non negative maturity required: " <<
                   maturity << " not allowed");
        //vol = stdDev_ / std::sqrt(maturity);
        //rate = -std::log(discount_)/maturity;
        //dividendRate = -std::log(forward_ / spot * discount_)/maturity;
        //return rate*value() - (rate-dividendRate)*spot*delta(spot)
        //    - 0.5*vol*vol*spot*spot*gamma(spot);
        return -( std::log(discount_)            * value()
                 +std::log(forward_/spot) * spot * delta(spot)
                 +0.5*variance_ * spot  * spot * gamma(spot))/maturity;
    }

    Real BlackCalculator::thetaPerDay(Real spot, Time maturity) const {
        return theta(spot, maturity)/365.0;
    }

    Real BlackCalculator::vega(Time maturity) const {
        QL_REQUIRE(maturity>=0.0,
                   "negative maturity not allowed");

        Real temp = std::log(strike_/forward_)/variance_;
        // actually DalphaDsigma / SQRT(T)
        Real DalphaDsigma = DalphaDd1_*(temp+0.5);
        Real DbetaDsigma  = DbetaDd2_ *(temp-0.5);

        Real temp2 = DalphaDsigma * forward_ + DbetaDsigma * X_;

        return discount_ * std::sqrt(maturity) * temp2;

    }

    Real BlackCalculator::rho(Time maturity) const {
        QL_REQUIRE(maturity>=0.0,
                   "negative maturity not allowed");

        // actually DalphaDr / T
        Real DalphaDr = DalphaDd1_/stdDev_;
        Real DbetaDr  = DbetaDd2_/stdDev_;
        Real temp = DalphaDr * forward_ + alpha_ * forward_ + DbetaDr * X_;

        return maturity * (discount_ * temp - value());
    }

    Real BlackCalculator::dividendRho(Time maturity) const {
        QL_REQUIRE(maturity>=0.0,
                   "negative maturity not allowed");

        // actually DalphaDq / T
        Real DalphaDq = -DalphaDd1_/stdDev_;
        Real DbetaDq  = -DbetaDd2_/stdDev_;

        Real temp = DalphaDq * forward_ - alpha_ * forward_ + DbetaDq * X_;

        return maturity * discount_ * temp;
    }

    Real BlackCalculator::itmCashProbability() const {
        return cum_d2_;
    }

    Real BlackCalculator::itmAssetProbability() const {
        return cum_d1_;
    }

    Real BlackCalculator::strikeSensitivity() const {

        Real temp = stdDev_*strike_;
        Real DalphaDstrike = -DalphaDd1_/temp;
        Real DbetaDstrike  = -DbetaDd2_/temp;

        Real temp2 =
            DalphaDstrike * forward_ + DbetaDstrike * X_ + beta_ * DXDstrike_;

        return discount_ * temp2;
    }

    Real BlackCalculator::alpha() const {
        return alpha_;
    }

    Real BlackCalculator::beta() const {
        return beta_;
    }

}
