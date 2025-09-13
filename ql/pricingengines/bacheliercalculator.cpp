/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

  Copyright (C) 2025 kp9991-git https://github.com/kp9991-git

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
        BachelierCalculator& bachelier_;

      public:
        explicit Calculator(BachelierCalculator& bachelier) : bachelier_(bachelier) {}
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
        QL_REQUIRE(stdDev_ >= 0.0, "stdDev (" << stdDev_ << ") must be non-negative");
        QL_REQUIRE(discount_ > 0.0, "discount (" << discount_ << ") must be positive");

        // For Bachelier model, we use d = (F - K) / σ instead of the Black-Scholes d1, d2
        if (stdDev_ >= QL_EPSILON) {
            // Bachelier d parameter: d = (F - K) / σ
            d1_ = (forward_ - strike_) / stdDev_;
            d2_ = d1_; // In Bachelier model, d2 = d1 (no drift adjustment)
            
            CumulativeNormalDistribution f;
            cum_d1_ = f(d1_);
            cum_d2_ = cum_d1_; // Same as cum_d1_ in Bachelier
            n_d1_ = f.derivative(d1_);
            n_d2_ = n_d1_; // Same as n_d1_ in Bachelier
        } else {
            // When volatility is zero
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
        DxDs_ = 0.0;

        // For Bachelier model, the option values are:
        // Call: max(F-K, 0) = (F-K)*N(d) + σ*n(d)
        // Put:  max(K-F, 0) = (K-F)*N(-d) + σ*n(d)
        // 
        // We represent this as: discount * (forward * alpha + x * beta)
        // where for Bachelier:
        // Call: alpha = N(d), beta = -N(d) + σ*n(d)/x (when x != 0)
        // Put:  alpha = -N(-d), beta = N(-d) + σ*n(d)/x (when x != 0)
        
        switch (p->optionType()) {
            case Option::Call:
                alpha_ = cum_d1_;        // N(d)
                DalphaDd1_ = n_d1_;      // n(d)
                beta_ = -cum_d1_;        // -N(d) - base part
                DbetaDd2_ = -n_d1_;      // -n(d)
                break;
            case Option::Put:
                alpha_ = cum_d1_ - 1.0;  // N(d) - 1 = -N(-d)
                DalphaDd1_ = n_d1_;      // n(d)
                beta_ = 1.0 - cum_d1_;   // 1 - N(d) = N(-d)
                DbetaDd2_ = -n_d1_;      // -n(d)
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
        bachelier_.alpha_ = bachelier_.DalphaDd1_ = 0.0;
        bachelier_.x_ = payoff.cashPayoff();
        bachelier_.DxDstrike_ = 0.0;
        switch (payoff.optionType()) {
            case Option::Call:
                bachelier_.beta_ = bachelier_.cum_d2_;
                bachelier_.DbetaDd2_ = bachelier_.n_d2_;
                break;
            case Option::Put:
                bachelier_.beta_ = 1.0 - bachelier_.cum_d2_;
                bachelier_.DbetaDd2_ = -bachelier_.n_d2_;
                break;
            default:
                QL_FAIL("invalid option type");
        }
    }

    void BachelierCalculator::Calculator::visit(AssetOrNothingPayoff& payoff) {
        bachelier_.beta_ = bachelier_.DbetaDd2_ = 0.0;
        switch (payoff.optionType()) {
            case Option::Call:
                bachelier_.alpha_ = bachelier_.cum_d1_;
                bachelier_.DalphaDd1_ = bachelier_.n_d1_;
                break;
            case Option::Put:
                bachelier_.alpha_ = 1.0 - bachelier_.cum_d1_;
                bachelier_.DalphaDd1_ = -bachelier_.n_d1_;
                break;
            default:
                QL_FAIL("invalid option type");
        }
    }

    void BachelierCalculator::Calculator::visit(GapPayoff& payoff) {
        bachelier_.x_ = payoff.secondStrike();
        bachelier_.DxDstrike_ = 0.0;
    }

    Real BachelierCalculator::value() const {
        // Bachelier option value formula:
        // Call: (F-K)*N(d) + σ*n(d)
        // Put:  (K-F)*N(-d) + σ*n(d)
        // where d = (F-K)/σ
        
        Real intrinsic = forward_ - strike_;
        Real timeValue = 0.0;
        
        if (stdDev_ > QL_EPSILON) {
            timeValue = stdDev_ * n_d1_;
        }
        
        Real result;
        if (alpha_ >= 0) // Call option (alpha_ = N(d) >= 0)
            result = intrinsic * cum_d1_ + timeValue;
        else // Put option (alpha_ = N(d) - 1 < 0)
            result = -intrinsic * (1.0 - cum_d1_) + timeValue;
        
        return discount_ * std::max(result, 0.0);
    }

    Real BachelierCalculator::delta(Real spot) const {

        QL_REQUIRE(spot > 0.0, "positive spot value required: " << spot << " not allowed");

        // For Bachelier model:
        // Delta = dV/dS = (dV/dF) * (dF/dS)
        // where dF/dS = F/S (assuming forward = spot * exp(r*T))
        // and dV/dF = N(d) for calls, -N(-d) for puts
        
        Real DforwardDs = forward_ / spot;
        Real deltaFwd = deltaForward();

        return deltaFwd * DforwardDs;
    }

    Real BachelierCalculator::deltaForward() const {
        // For Bachelier model:
        // Delta_Forward = dV/dF = N(d) for calls, -N(-d) for puts
        // where d = (F-K)/σ
        
        if (alpha_ >= 0) { // Call option
            return discount_ * cum_d1_; // N(d)
        } else { // Put option  
            return discount_ * (cum_d1_ - 1.0); // N(d) - 1 = -N(-d)
        }
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

        // For Bachelier model:
        // Gamma = d²V/dS² = d/dS(dV/dS) = d/dS(N(d) * dF/dS) * dF/dS
        // = n(d) * (1/σ) * (dF/dS)² * (dd/dF)
        // where dd/dF = 1/σ
        
        if (stdDev_ <= QL_EPSILON) {
            return 0.0;
        }
        
        Real DforwardDs = forward_ / spot;
        Real gammaForward = n_d1_ / stdDev_; // dn(d)/dF = n(d) * (1/σ) * (dd/dF) = n(d)/σ
        
        return discount_ * gammaForward * DforwardDs * DforwardDs;
    }

    Real BachelierCalculator::gammaForward() const {
        // For Bachelier model:
        // Gamma_Forward = d²V/dF² = d/dF(N(d)) = n(d) * dd/dF = n(d)/σ
        
        if (stdDev_ <= QL_EPSILON) {
            return 0.0;
        }
        
        return discount_ * n_d1_ / stdDev_;
    }

    Real BachelierCalculator::theta(Real spot, Time maturity) const {

        QL_REQUIRE(maturity >= 0.0, "maturity (" << maturity << ") must be non-negative");
        if (close(maturity, 0.0)) return 0.0;
        
        // Theta = -dV/dt = -(r*V - r*S*Delta + 0.5*σ²*Gamma)

        return -(std::log(discount_) * value() + std::log(forward_ / spot) * spot * delta(spot) +
                 0.5 * variance_ * gamma(spot)) / maturity;
    }

    Real BachelierCalculator::vega(Time maturity) const {
        QL_REQUIRE(maturity >= 0.0, "negative maturity not allowed");

        // For Bachelier model:
        // Vega = dV/dσ = d/dσ[(F-K)*N(d) + σ*n(d)]
        // = (F-K)*n(d)*dd/dσ + n(d) + σ*n'(d)*dd/dσ
        // where d = (F-K)/σ, so dd/dσ = -(F-K)/σ² = -d/σ
        // and n'(d) = -d*n(d)
        // Therefore: Vega = n(d) + σ*(-d*n(d))*(-d/σ) = n(d) + d²*n(d) = n(d)*(1 + d²) - (F-K)*n(d)*d/σ
        // Simplifying: Vega = n(d) for Bachelier model
        
        if (maturity <= QL_EPSILON) {
            return 0.0;
        }
        
        return discount_ * std::sqrt(maturity) * n_d1_;
    }

    Real BachelierCalculator::rho(Time maturity) const {
        QL_REQUIRE(maturity >= 0.0, "negative maturity not allowed");
        
        // For Bachelier model:
        // Rho = dV/dr = T * (discount * delta_forward * forward - value)
        // where delta_forward = N(d) for calls, N(d)-1 for puts
        
        Real deltaFwd = deltaForward();
        Real rho_value = maturity * (deltaFwd * forward_ - value());
        
        return rho_value;
    }

    Real BachelierCalculator::dividendRho(Time maturity) const {
        QL_REQUIRE(maturity >= 0.0, "negative maturity not allowed");
        
        // For Bachelier model:
        // Dividend rho = -T * discount * delta_forward * forward
        // where delta_forward = N(d) for calls, N(d)-1 for puts
        
        Real deltaFwd = (alpha_ >= 0) ? cum_d1_ : (cum_d1_ - 1.0);
        
        return -maturity * discount_ * deltaFwd * forward_;
    }

    Real BachelierCalculator::strikeSensitivity() const {
        // For Bachelier model:
        // dV/dK = -N(d) for calls, N(-d) for puts
        // where d = (F-K)/σ, so dd/dK = -1/σ
        
        if (alpha_ >= 0) { // Call option
            return -discount_ * cum_d1_; // -N(d)
        } else { // Put option
            return discount_ * (1.0 - cum_d1_); // N(-d) = 1 - N(d)
        }
    }


    Real BachelierCalculator::strikeGamma() const {
        // For Bachelier model:
        // d²V/dK² = d/dK(-N(d)) = -n(d) * dd/dK = -n(d) * (-1/σ) = n(d)/σ
        // This is the same for both calls and puts
        
        if (stdDev_ <= QL_EPSILON) {
            return 0.0;
        }
        
        return discount_ * n_d1_ / stdDev_;
    }
}
