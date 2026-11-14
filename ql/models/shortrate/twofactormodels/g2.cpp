/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 Mike Parker
 Copyright (C) 2021 Magnus Mencke

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        // (1 - exp(-x))/x
        Real expDecayRatio(Real x) {
            return -std::expm1(-x)/x;
        }

        // The integral of s^2 * f(u*s) * f(v*s) between 0 and 1, where f is expDecayRatio.
        // Its closed form (1 - f(u) - f(v) + f(u+v))/(u*v) subtracts terms that are O(1)
        // to get a result that is O(u*v) and then divides by u*v, so it loses all
        // precision when u and v are small and is only used when that is not the case.
        // G2::V combines this function at (a*t,a*t), (b*t,b*t) and (a*t,b*t).
        Real varianceKernel(Real u, Real v) {
            if (u < v)
                std::swap(u, v);

            if (u + v <= 2.0) {
                // series sum_(k>=2) (-1)^k*P_k/(k+1)!, where P_k = ((u+v)^k - u^k - v^k)/(u*v),
                // so that P_2 = 2 and P_(k+1) = (u+v)*P_k + u^(k-1) + v^(k-1)
                const Real s = u + v;
                Real P = 2.0, uPower = u, vPower = v, factorial = 6.0, sign = 1.0, sum = 0.0;
                for (Size k = 2; k < 60; ++k) {
                    const Real term = sign*P/factorial;
                    sum += term;
                    if (std::fabs(term) < 0.01*QL_EPSILON*std::fabs(sum))
                        break;
                    P = s*P + uPower + vPower;
                    uPower *= u;
                    vPower *= v;
                    factorial *= k + 2;
                    sign = -sign;
                }
                return sum;
            }

            if (v >= 0.5)
                return (1.0 - expDecayRatio(u) - expDecayRatio(v) + expDecayRatio(u+v))/(u*v);

            // v is small and u is not. Write 1 - f(v) as v*omega(v), where
            // omega(v) = (v + expm1(-v))/v^2 = 1/2 - v/6 + v^2/24 - ..., and
            // f(u) - f(u+v) = (u*exp(-u)*expm1(-v) - v*expm1(-u))/(u*(u+v))
            Real omega = 0.0, term = 0.5;
            for (Size k = 2; k < 40; ++k) {
                omega += term;
                term *= -v/(k + 1);
                if (std::fabs(term) < 0.01*QL_EPSILON*std::fabs(omega))
                    break;
            }
            const Real difference =
                (u*std::exp(-u)*std::expm1(-v) - v*std::expm1(-u))/(u*(u+v));
            return (v*omega - difference)/(u*v);
        }

    }

    G2::G2(const Handle<YieldTermStructure>& termStructure,
           Real a, Real sigma, Real b, Real eta, Real rho)
    : TwoFactorModel(5), TermStructureConsistentModel(termStructure),
      a_(arguments_[0]), sigma_(arguments_[1]), b_(arguments_[2]),
      eta_(arguments_[3]), rho_(arguments_[4]) {

        a_     = ConstantParameter(a,     PositiveConstraint());
        sigma_ = ConstantParameter(sigma, PositiveConstraint());
        b_     = ConstantParameter(b,     PositiveConstraint());
        eta_   = ConstantParameter(eta,   PositiveConstraint());
        rho_   = ConstantParameter(rho,   BoundaryConstraint(-1.0, 1.0));

        G2::generateArguments();

        registerWith(termStructure);
    }

    ext::shared_ptr<TwoFactorModel::ShortRateDynamics> G2::dynamics() const {
        return ext::make_shared<Dynamics>(phi_, a(), sigma(), b(), eta(), rho());
    }

    void G2::generateArguments() {

        phi_ = FittingParameter(termStructure(),
            a(), sigma(), b(), eta(), rho());
    }

    Real G2::sigmaP(Time t, Time s) const {
        Real temp = 1.0 - std::exp(-(a()+b())*t);
        Real temp1 = 1.0 - std::exp(-a()*(s-t));
        Real temp2 = 1.0 - std::exp(-b()*(s-t));
        Real a3 = a()*a()*a();
        Real b3 = b()*b()*b();
        Real sigma2 = sigma()*sigma();
        Real eta2 = eta()*eta();
        Real value =
            0.5*sigma2*temp1*temp1*(1.0 - std::exp(-2.0*a()*t))/a3 +
            0.5*eta2*temp2*temp2*(1.0 - std::exp(-2.0*b()*t))/b3 +
            2.0*rho()*sigma()*eta()/(a()*b()*(a()+b()))*
            temp1*temp2*temp;
        return std::sqrt(value);
    }

    Real G2::discountBond(Time t, Time T, Real x, Real y) const {
        return A(t,T) * std::exp(-B(a(),(T-t))*x-B(b(),(T-t))*y);
    }

    Real G2::discountBondOption(Option::Type type, Real strike, Time maturity,
        Time bondMaturity) const {

        Real v = sigmaP(maturity, bondMaturity);
        Real f = termStructure()->discount(bondMaturity);
        Real k = termStructure()->discount(maturity)*strike;

        return blackFormula(type, k, f, v);
    }

    Real G2::V(Time t) const {
        // the variance of the integral of the two factors between 0 and t, that is, the
        // integral of sigma^2*B(a,s)^2 + eta^2*B(b,s)^2 + 2*rho*sigma*eta*B(a,s)*B(b,s)
        Real u = a()*t;
        Real v = b()*t;
        Real valuex = sigma()*sigma()*varianceKernel(u, u);
        Real valuey = eta()*eta()*varianceKernel(v, v);
        Real value = 2.0*rho()*sigma()*eta()*varianceKernel(u, v);
        return t*t*t*(valuex + valuey + value);
    }

    Real G2::A(Time t, Time T) const {
        return termStructure()->discount(T)/termStructure()->discount(t)*
            std::exp(0.5*(V(T-t) - V(T) + V(t)));
    }

    Real G2::B(Real x, Time t) const {
        return -std::expm1(-x*t)/x;
    }

    class G2::SwaptionPricingFunction {
      public:
        SwaptionPricingFunction(Real a,
                                Real sigma,
                                Real b,
                                Real eta,
                                Real rho,
                                Real w,
                                Real start,
                                std::vector<Time> payTimes,
                                Rate fixedRate,
                                const G2& model)
        : a_(a), sigma_(sigma), b_(b), eta_(eta), rho_(rho), w_(w), T_(start),
          t_(std::move(payTimes)), rate_(fixedRate), size_(t_.size()), A_(size_), Ba_(size_),
          Bb_(size_) {


            sigmax_ = sigma_*std::sqrt(0.5*(1.0-std::exp(-2.0*a_*T_))/a_);
            sigmay_ =   eta_*std::sqrt(0.5*(1.0-std::exp(-2.0*b_*T_))/b_);
            rhoxy_ = rho_*eta_*sigma_*(1.0 - std::exp(-(a_+b_)*T_))/
                ((a_+b_)*sigmax_*sigmay_);

            Real temp = sigma_*sigma_/(a_*a_);
            mux_ = -((temp+rho_*sigma_*eta_/(a_*b_))*(1.0 - std::exp(-a*T_)) -
                     0.5*temp*(1.0 - std::exp(-2.0*a_*T_)) -
                     rho_*sigma_*eta_/(b_*(a_+b_))*
                     (1.0- std::exp(-(b_+a_)*T_)));

            temp = eta_*eta_/(b_*b_);
            muy_ = -((temp+rho_*sigma_*eta_/(a_*b_))*(1.0 - std::exp(-b*T_)) -
                     0.5*temp*(1.0 - std::exp(-2.0*b_*T_)) -
                     rho_*sigma_*eta_/(a_*(a_+b_))*
                     (1.0- std::exp(-(b_+a_)*T_)));

            for (Size i=0; i<size_; i++) {
                A_[i] = model.A(T_, t_[i]);
                Ba_[i] = model.B(a_, t_[i]-T_);
                Bb_[i] = model.B(b_, t_[i]-T_);
            }
        }

        Real mux() const { return mux_; }
        Real sigmax() const { return sigmax_; }
        Real operator()(Real x) const {
            CumulativeNormalDistribution phi;
            Real temp = (x - mux_)/sigmax_;
            Real txy = std::sqrt(1.0 - rhoxy_*rhoxy_);

            Array lambda(size_);
            Size i;
            for (i=0; i<size_; i++) {
                Real tau = (i==0 ? t_[0] - T_ : t_[i] - t_[i-1]);
                Real c = (i==size_-1 ? Real(1.0+rate_*tau) : rate_*tau);
                lambda[i] = c*A_[i]*std::exp(-Ba_[i]*x);
            }

            SolvingFunction function(lambda, Bb_) ;
            Brent s1d;
            s1d.setMaxEvaluations(1000);
            Real searchBound = std::max(10.0*sigmay_, 1.0);
            Real yb = s1d.solve(function, 1e-6, 0.00, -searchBound, searchBound);

            Real h1 = (yb - muy_)/(sigmay_*txy) -
                rhoxy_*(x  - mux_)/(sigmax_*txy);
            Real value = phi(-w_*h1);


            for (i=0; i<size_; i++) {
                Real h2 = h1 +
                    Bb_[i]*sigmay_*std::sqrt(1.0-rhoxy_*rhoxy_);
                Real kappa = - Bb_[i] *
                    (muy_ - 0.5*txy*txy*sigmay_*sigmay_*Bb_[i] +
                     rhoxy_*sigmay_*(x-mux_)/sigmax_);
                value -= lambda[i] *std::exp(kappa)*phi(-w_*h2);
            }

            return std::exp(-0.5*temp*temp)*value/
                (sigmax_*std::sqrt(2.0*M_PI));
        }


      private:
        class SolvingFunction {
          public:
            SolvingFunction(const Array& lambda, const Array& Bb)
            : lambda_(lambda), Bb_(Bb) {}
            Real operator()(Real y) const {
                Real value = 1.0;
                for (Size i=0; i<lambda_.size(); i++) {
                    value -= lambda_[i]*std::exp(-Bb_[i]*y);
                }
                return value;
            }
          private:
            const Array& lambda_;
            const Array& Bb_;
        };

        Real a_, sigma_, b_, eta_, rho_, w_;
        Time T_;
        std::vector<Time> t_;
        Rate rate_;
        Size size_;
        Array A_, Ba_, Bb_;
        Real mux_, muy_, sigmax_, sigmay_, rhoxy_;
    };

    Real G2::swaption(const Swaption::arguments& arguments,
                      Rate fixedRate, Real range, Size intervals) const {

        QL_REQUIRE(arguments.nominal != Null<Real>(),
                   "non-constant nominals are not supported yet");

        auto settlement = termStructure()->referenceDate();
        auto dayCounter = termStructure()->dayCounter();
        Time start = dayCounter.yearFraction(settlement,
                                             arguments.floatingResetDates[0]);
        Real w = (arguments.type==Swap::Payer ? 1 : -1 );

        std::vector<Time> fixedPayTimes(arguments.fixedPayDates.size());
        for (Size i=0; i<fixedPayTimes.size(); ++i)
            fixedPayTimes[i] =
                dayCounter.yearFraction(settlement,
                                        arguments.fixedPayDates[i]);

        SwaptionPricingFunction function(a(), sigma(), b(), eta(), rho(),
                                         w, start,
                                         fixedPayTimes,
                                         fixedRate, (*this));

        Real upper = function.mux() + range*function.sigmax();
        Real lower = function.mux() - range*function.sigmax();
        SegmentIntegral integrator(intervals);
        return arguments.nominal * w * termStructure()->discount(start) *
            integrator(function, lower, upper);
    }

}
