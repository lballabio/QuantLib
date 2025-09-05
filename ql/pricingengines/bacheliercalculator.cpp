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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/bacheliercalculator.hpp>

namespace QuantLib {

    class BachelierCalculator::Calculator : public AcyclicVisitor,
                                        public Visitor<Payoff>,
                                        public Visitor<PlainVanillaPayoff>,
                                        public Visitor<CashOrNothingPayoff>,
                                        public Visitor<AssetOrNothingPayoff>,
                                        public Visitor<GapPayoff> {
      private:
        BachelierCalculator& black_;

      public:
        explicit Calculator(BachelierCalculator& black) : black_(black) {}
        void visit(Payoff&) override;
        void visit(PlainVanillaPayoff&) override;
        void visit(CashOrNothingPayoff&) override;
        void visit(AssetOrNothingPayoff&) override;
        void visit(GapPayoff&) override;
    };


    BachelierCalculator::BachelierCalculator(const ext::shared_ptr<StrikedTypePayoff>& p,
                                     Real forward,
                                     Real stdDev,
                                     Real discount)
    : DiffusionCalculator(p, forward, stdDev, discount) {
        initialize(p);
    }

    BachelierCalculator::BachelierCalculator(
        Option::Type optionType, Real strike, Real forward, Real stdDev, Real discount)
    : DiffusionCalculator(optionType, strike, forward, stdDev, discount) {
        initialize(ext::shared_ptr<StrikedTypePayoff>(new PlainVanillaPayoff(optionType, strike)));
    }

    void BachelierCalculator::initialize(const ext::shared_ptr<StrikedTypePayoff>& p) {
        QL_REQUIRE(strike_ >= 0.0, "strike (" << strike_ << ") must be non-negative");
        QL_REQUIRE(forward_ > 0.0, "forward (" << forward_ << ") must be positive");
        // QL_REQUIRE(displacement_>=0.0,
        //            "displacement (" << displacement_ << ") must be non-negative");
        QL_REQUIRE(stdDev_ >= 0.0, "stdDev (" << stdDev_ << ") must be non-negative");
        QL_REQUIRE(discount_ > 0.0, "discount (" << discount_ << ") must be positive");

        if (stdDev_ >= QL_EPSILON) {
            if (close(strike_, 0.0)) {
                d1_ = QL_MAX_REAL;
                d2_ = QL_MAX_REAL;
                cum_d1_ = 1.0;
                cum_d2_ = 1.0;
                n_d1_ = 0.0;
                n_d2_ = 0.0;
            } else {
                d1_ = std::log(forward_ / strike_) / stdDev_ + 0.5 * stdDev_;
                d2_ = d1_ - stdDev_;
                CumulativeNormalDistribution f;
                cum_d1_ = f(d1_);
                cum_d2_ = f(d2_);
                n_d1_ = f.derivative(d1_);
                n_d2_ = f.derivative(d2_);
            }
        } else {
            if (close(forward_, strike_)) {
                d1_ = 0;
                d2_ = 0;
                cum_d1_ = 0.5;
                cum_d2_ = 0.5;
                n_d1_ = M_SQRT_2 * M_1_SQRTPI;
                n_d2_ = M_SQRT_2 * M_1_SQRTPI;
            } else if (forward_ > strike_) {
                d1_ = QL_MAX_REAL;
                d2_ = QL_MAX_REAL;
                cum_d1_ = 1.0;
                cum_d2_ = 1.0;
                n_d1_ = 0.0;
                n_d2_ = 0.0;
            } else {
                d1_ = QL_MIN_REAL;
                d2_ = QL_MIN_REAL;
                cum_d1_ = 0.0;
                cum_d2_ = 0.0;
                n_d1_ = 0.0;
                n_d2_ = 0.0;
            }
        }

        x_ = strike_;
        DxDstrike_ = 1.0;

        // the following one will probably disappear as soon as
        // super-share will be properly handled
        DxDs_ = 0.0;

        // this part is always executed.
        // in case of plain-vanilla payoffs, it is also the only part
        // which is executed.
        switch (p->optionType()) {
            case Option::Call:
                alpha_ = cum_d1_;   //  N(d1)
                DalphaDd1_ = n_d1_; //  n(d1)
                beta_ = -cum_d2_;   // -N(d2)
                DbetaDd2_ = -n_d2_; // -n(d2)
                break;
            case Option::Put:
                alpha_ = -1.0 + cum_d1_; // -N(-d1)
                DalphaDd1_ = n_d1_;      //  n( d1)
                beta_ = 1.0 - cum_d2_;   //  N(-d2)
                DbetaDd2_ = -n_d2_;      // -n( d2)
                break;
            default:
                QL_FAIL("invalid option type");
        }

        // now dispatch on type.

        Calculator calc(*this);
        p->accept(calc);
    }

    void BachelierCalculator::Calculator::visit(Payoff& p) {
        QL_FAIL("unsupported payoff type: " << p.name());
    }

    void BachelierCalculator::Calculator::visit(PlainVanillaPayoff&) {}

    void BachelierCalculator::Calculator::visit(CashOrNothingPayoff& payoff) {
        black_.alpha_ = black_.DalphaDd1_ = 0.0;
        black_.x_ = payoff.cashPayoff();
        black_.DxDstrike_ = 0.0;
        switch (payoff.optionType()) {
            case Option::Call:
                black_.beta_ = black_.cum_d2_;
                black_.DbetaDd2_ = black_.n_d2_;
                break;
            case Option::Put:
                black_.beta_ = 1.0 - black_.cum_d2_;
                black_.DbetaDd2_ = -black_.n_d2_;
                break;
            default:
                QL_FAIL("invalid option type");
        }
    }

    void BachelierCalculator::Calculator::visit(AssetOrNothingPayoff& payoff) {
        black_.beta_ = black_.DbetaDd2_ = 0.0;
        switch (payoff.optionType()) {
            case Option::Call:
                black_.alpha_ = black_.cum_d1_;
                black_.DalphaDd1_ = black_.n_d1_;
                break;
            case Option::Put:
                black_.alpha_ = 1.0 - black_.cum_d1_;
                black_.DalphaDd1_ = -black_.n_d1_;
                break;
            default:
                QL_FAIL("invalid option type");
        }
    }

    void BachelierCalculator::Calculator::visit(GapPayoff& payoff) {
        black_.x_ = payoff.secondStrike();
        black_.DxDstrike_ = 0.0;
    }

    Real BachelierCalculator::value() const {
        Real result = discount_ * (forward_ * alpha_ + x_ * beta_);
        return result;
    }

    Real BachelierCalculator::delta(Real spot) const {

        QL_REQUIRE(spot > 0.0, "positive spot value required: " << spot << " not allowed");

        Real DforwardDs = forward_ / spot;

        Real temp = stdDev_ * spot;
        Real DalphaDs = DalphaDd1_ / temp;
        Real DbetaDs = DbetaDd2_ / temp;
        Real temp2 = DalphaDs * forward_ + alpha_ * DforwardDs + DbetaDs * x_ + beta_ * DxDs_;

        return discount_ * temp2;
    }

    Real BachelierCalculator::deltaForward() const {

        Real temp = stdDev_ * forward_;
        Real DalphaDforward = DalphaDd1_ / temp;
        Real DbetaDforward = DbetaDd2_ / temp;
        Real temp2 = DalphaDforward * forward_ + alpha_ + DbetaDforward * x_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    Real BachelierCalculator::elasticity(Real spot) const {
        Real val = value();
        Real del = delta(spot);
        if (val > QL_EPSILON)
            return del / val * spot;
        else if (std::fabs(del) < QL_EPSILON)
            return 0.0;
        else if (del > 0.0)
            return QL_MAX_REAL;
        else
            return QL_MIN_REAL;
    }

    Real BachelierCalculator::elasticityForward() const {
        Real val = value();
        Real del = deltaForward();
        if (val > QL_EPSILON)
            return del / val * forward_;
        else if (std::fabs(del) < QL_EPSILON)
            return 0.0;
        else if (del > 0.0)
            return QL_MAX_REAL;
        else
            return QL_MIN_REAL;
    }

    Real BachelierCalculator::gamma(Real spot) const {

        QL_REQUIRE(spot > 0.0, "positive spot value required: " << spot << " not allowed");

        Real DforwardDs = forward_ / spot;

        Real temp = stdDev_ * spot;
        Real DalphaDs = DalphaDd1_ / temp;
        Real DbetaDs = DbetaDd2_ / temp;

        Real D2alphaDs2 = -DalphaDs / spot * (1 + d1_ / stdDev_);
        Real D2betaDs2 = -DbetaDs / spot * (1 + d2_ / stdDev_);

        Real temp2 = D2alphaDs2 * forward_ + 2.0 * DalphaDs * DforwardDs + D2betaDs2 * x_ +
                     2.0 * DbetaDs * DxDs_;

        return discount_ * temp2;
    }

    Real BachelierCalculator::gammaForward() const {

        Real temp = stdDev_ * forward_;
        Real DalphaDforward = DalphaDd1_ / temp;
        Real DbetaDforward = DbetaDd2_ / temp;

        Real D2alphaDforward2 = -DalphaDforward / forward_ * (1 + d1_ / stdDev_);
        Real D2betaDforward2 = -DbetaDforward / forward_ * (1 + d2_ / stdDev_);

        Real temp2 = D2alphaDforward2 * forward_ + 2.0 * DalphaDforward +
                     D2betaDforward2 * x_; // DXDforward = 0.0

        return discount_ * temp2;
    }

    Real BachelierCalculator::theta(Real spot, Time maturity) const {

        QL_REQUIRE(maturity >= 0.0, "maturity (" << maturity << ") must be non-negative");
        if (close(maturity, 0.0))
            return 0.0;
        return -(std::log(discount_) * value() + std::log(forward_ / spot) * spot * delta(spot) +
                 0.5 * variance_ * spot * spot * gamma(spot)) /
               maturity;
    }

    Real BachelierCalculator::vega(Time maturity) const {
        QL_REQUIRE(maturity >= 0.0, "negative maturity not allowed");

        Real temp = std::log(strike_ / forward_) / variance_;
        // actually DalphaDsigma / SQRT(T)
        Real DalphaDsigma = DalphaDd1_ * (temp + 0.5);
        Real DbetaDsigma = DbetaDd2_ * (temp - 0.5);

        Real temp2 = DalphaDsigma * forward_ + DbetaDsigma * x_;

        return discount_ * std::sqrt(maturity) * temp2;
    }

    Real BachelierCalculator::rho(Time maturity) const {
        QL_REQUIRE(maturity >= 0.0, "negative maturity not allowed");

        // actually DalphaDr / T
        Real DalphaDr = DalphaDd1_ / stdDev_;
        Real DbetaDr = DbetaDd2_ / stdDev_;
        Real temp = DalphaDr * forward_ + alpha_ * forward_ + DbetaDr * x_;

        return maturity * (discount_ * temp - value());
    }

    Real BachelierCalculator::dividendRho(Time maturity) const {
        QL_REQUIRE(maturity >= 0.0, "negative maturity not allowed");

        // actually DalphaDq / T
        Real DalphaDq = -DalphaDd1_ / stdDev_;
        Real DbetaDq = -DbetaDd2_ / stdDev_;

        Real temp = DalphaDq * forward_ - alpha_ * forward_ + DbetaDq * x_;

        return maturity * discount_ * temp;
    }

    Real BachelierCalculator::strikeSensitivity() const {

        Real temp = stdDev_ * strike_;
        Real DalphaDstrike = -DalphaDd1_ / temp;
        Real DbetaDstrike = -DbetaDd2_ / temp;

        Real temp2 = DalphaDstrike * forward_ + DbetaDstrike * x_ + beta_ * DxDstrike_;

        return discount_ * temp2;
    }


    Real BachelierCalculator::strikeGamma() const {

        Real temp = stdDev_ * strike_;
        Real DalphaDstrike = -DalphaDd1_ / temp;
        Real DbetaDstrike = -DbetaDd2_ / temp;

        Real D2alphaD2strike = -DalphaDstrike / strike_ * (1 - d1_ / stdDev_);
        Real D2betaD2strike = -DbetaDstrike / strike_ * (1 - d2_ / stdDev_);

        Real temp2 =
            D2alphaD2strike * forward_ + D2betaD2strike * x_ + 2.0 * DbetaDstrike * DxDstrike_;

        return discount_ * temp2;
    }
}
