
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 Mike Parker

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

#include <ql/ShortRateModels/TwoFactorModels/g2.hpp>
#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/Math/segmentintegral.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    G2::G2(const Handle<TermStructure>& termStructure,
           Real a, Real sigma, Real b, Real eta, Real rho)
    : TwoFactorModel(5), TermStructureConsistentModel(termStructure),
      a_(arguments_[0]), sigma_(arguments_[1]), b_(arguments_[2]),
      eta_(arguments_[3]), rho_(arguments_[4]) {

        a_     = ConstantParameter(a,     PositiveConstraint());
        sigma_ = ConstantParameter(sigma, PositiveConstraint());
        b_     = ConstantParameter(b,     PositiveConstraint());
        eta_   = ConstantParameter(eta,   PositiveConstraint());
        rho_   = ConstantParameter(rho,   BoundaryConstraint(-1.0, 1.0));

        generateArguments();
    }

    boost::shared_ptr<TwoFactorModel::ShortRateDynamics> G2::dynamics() const {
        return boost::shared_ptr<ShortRateDynamics>(new
            Dynamics(phi_, a(), sigma(), b(), eta(), rho()));
    }

    void G2::generateArguments() {
        
        phi_ = FittingParameter(termStructure(),
            a(), sigma(), b(), eta(), rho());
    }

    Real G2::sigmaP(Time t, Time s) const {
        Real temp = 1.0 - QL_EXP(-(a()+b())*t);
        Real temp1 = 1.0 - QL_EXP(-a()*(s-t));
        Real temp2 = 1.0 - QL_EXP(-b()*(s-t));
        Real a3 = a()*a()*a();
        Real b3 = b()*b()*b();
        Real sigma2 = sigma()*sigma();
        Real eta2 = eta()*eta();
        Real value =
            0.5*sigma2*temp1*temp1*(1.0 - QL_EXP(-2.0*a()*t))/a3 +
            0.5*eta2*temp2*temp2*(1.0 - QL_EXP(-2.0*b()*t))/b3 +
            2.0*rho()*sigma()*eta()/(a()*b()*(a()+b()))*
            temp1*temp2*temp;
        return QL_SQRT(value);
    }

    Real G2::discountBondOption(Option::Type type, Real strike, Time maturity,
        Time bondMaturity) const {

        Real v = sigmaP(maturity, bondMaturity);
        Real f = termStructure()->discount(bondMaturity);
        Real k = termStructure()->discount(maturity)*strike;

        Real w = (type==Option::Call)? 1.0 : -1.0;

        return BlackModel::formula(f, k, v, w);
    }

    Real G2::V(Time t) const {
        Real expat = QL_EXP(-a()*t);
        Real expbt = QL_EXP(-b()*t);
        Real cx = sigma()/a();
        Real cy = eta()/b();
        Real valuex = cx*cx*(t + (2.0*expat-0.5*expat*expat-1.5)/a());
        Real valuey = cy*cy*(t + (2.0*expbt-0.5*expbt*expbt-1.5)/b());
        Real value = 2.0*rho()*cx*cy* (t + (expat - 1.0)/a() 
                                         + (expbt - 1.0)/b() 
                                         - (expat*expbt-1.0)/(a()+b()));
        return valuex + valuey + value;
    }

    Real G2::A(Time t, Time T) const {
        return termStructure()->discount(T)/termStructure()->discount(t)*
            QL_EXP(0.5*(V(T-t) - V(T) + V(t)));
    }

    Real G2::B(Real x, Time t) const {
        return (1.0 - QL_EXP(-x*t))/x;
    }

    class G2::SwaptionPricingFunction {
      public:
        SwaptionPricingFunction(Real a, Real sigma,
                                Real b, Real eta, Real rho, 
                                Real w, Real start, 
                                const std::vector<Time>& payTimes, 
                                Rate fixedRate, const G2& model)
        : a_(a), sigma_(sigma), b_(b), eta_(eta), rho_(rho), w_(w), 
          T_(start), t_(payTimes), rate_(fixedRate), size_(t_.size()), 
          A_(size_), Ba_(size_), Bb_(size_) {


            sigmax_ = sigma_*QL_SQRT(0.5*(1.0-QL_EXP(-2.0*a_*T_))/a_);
            sigmay_ =   eta_*QL_SQRT(0.5*(1.0-QL_EXP(-2.0*b_*T_))/b_);
            rhoxy_ = rho_*eta_*sigma_*(1.0 - QL_EXP(-(a_+b_)*T_))/
                ((a_+b_)*sigmax_*sigmay_);

            Real temp = sigma_*sigma_/(a_*a_);
            mux_ = -((temp+rho_*sigma_*eta_/(a_*b_))*(1.0 - QL_EXP(-a*T_)) -
                     0.5*temp*(1.0 - QL_EXP(-2.0*a_*T_)) -
                     rho_*sigma_*eta_/(b_*(a_+b_))*
                     (1.0- QL_EXP(-(b_+a_)*T_)));

            temp = eta_*eta_/(b_*b_);
            muy_ = -((temp+rho_*sigma_*eta_/(a_*b_))*(1.0 - QL_EXP(-b*T_)) -
                     0.5*temp*(1.0 - QL_EXP(-2.0*b_*T_)) -
                     rho_*sigma_*eta_/(a_*(a_+b_))*
                     (1.0- QL_EXP(-(b_+a_)*T_)));

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
            Real txy = QL_SQRT(1.0 - rhoxy_*rhoxy_);

            Array lambda(size_);
            Size i;
            for (i=0; i<size_; i++) {
                Real tau = (i==0 ? t_[0] - T_ : t_[i] - t_[i-1]);
                Real c = (i==size_-1 ? (1.0+rate_*tau) : rate_*tau);
                lambda[i] = c*A_[i]*QL_EXP(-Ba_[i]*x);
            }

            SolvingFunction function(lambda, Bb_) ;
            Brent s1d;
            s1d.setMaxEvaluations(1000);
            Real yb = s1d.solve(function, 1e-6, 0.00, -100.0, 100.0);

            Real h1 = (yb - muy_)/(sigmay_*txy) - 
                rhoxy_*(x  - mux_)/(sigmax_*txy);
            Real value = phi(-w_*h1);

            
            for (i=0; i<size_; i++) {
                Real h2 = h1 + 
                    Bb_[i]*sigmay_*QL_SQRT(1.0-rhoxy_*rhoxy_);
                Real kappa = - Bb_[i] *
                    (muy_ - 0.5*txy*txy*sigmay_*sigmay_*Bb_[i] +
                     rhoxy_*sigmay_*(x-mux_)/sigmax_);
                value -= lambda[i] *QL_EXP(kappa)*phi(-w_*h2);
            }

            return QL_EXP(-0.5*temp*temp)*value/
                          (sigmax_*QL_SQRT(2.0*M_PI));
        }


      private:
        class SolvingFunction {
          public:
            SolvingFunction(const Array& lambda, const Array& Bb) 
            : lambda_(lambda), Bb_(Bb) {}
            Real operator()(Real y) const {
                Real value = 1.0;
                for (Size i=0; i<lambda_.size(); i++) {
                    value -= lambda_[i]*QL_EXP(-Bb_[i]*y);
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
                      Real range, Size intervals) const {

        Time start = arguments.floatingResetTimes[0];
        Real w = (arguments.payFixed ? 1 : -1 );
        SwaptionPricingFunction function(a(), sigma(), b(), eta(), rho(), 
                                         w, start, 
                                         arguments.floatingPayTimes, 
                                         arguments.fixedRate, (*this));

        Real upper = function.mux() + range*function.sigmax();
        Real lower = function.mux() - range*function.sigmax();

        SegmentIntegral integrator(intervals);

        return arguments.nominal*w*termStructure()->discount(start)*
            integrator(function, lower, upper);
    }

}
