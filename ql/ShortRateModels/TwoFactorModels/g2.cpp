
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

    G2::G2(const RelinkableHandle<TermStructure>& termStructure,
           double a, double sigma, double b, double eta, double rho)
    : TwoFactorModel(5), TermStructureConsistentModel(termStructure),
      a_(arguments_[0]), sigma_(arguments_[1]), 
      b_(arguments_[2]), eta_(arguments_[3]),
      rho_(arguments_[4]) {
        a_ = ConstantParameter(a, PositiveConstraint());
        sigma_ = ConstantParameter(sigma, PositiveConstraint());
        b_ = ConstantParameter(b, PositiveConstraint());
        eta_ = ConstantParameter(eta, PositiveConstraint());
        rho_ = ConstantParameter(rho, BoundaryConstraint(-1.0, 1.0));
        generateArguments();
    }

    Handle<TwoFactorModel::ShortRateDynamics> G2::dynamics() const {
        return Handle<ShortRateDynamics>(new Dynamics(phi_, a(), sigma(), 
                                                      b(), eta(), rho()));
    }

    void G2::generateArguments() {
        phi_ = FittingParameter(termStructure(), 
                                a(), sigma(), b(), eta(), rho());
    }

    double G2::sigmaP(Time t, Time s) const {
        double temp = 1.0 - QL_EXP(-(a()+b())*t);
        double temp1 = 1.0 - QL_EXP(-a()*(s-t));
        double temp2 = 1.0 - QL_EXP(-b()*(s-t));
        double a3 = a()*a()*a();
        double b3 = b()*b()*b();
        double sigma2 = sigma()*sigma();
        double eta2 = eta()*eta();
        double value =
            0.5*sigma2*temp1*temp1*(1.0 - QL_EXP(-2.0*a()*t))/a3 +
            0.5*eta2*temp2*temp2*(1.0 - QL_EXP(-2.0*b()*t))/b3 +
            2.0*rho()*sigma()*eta()/(a()*b()*(a()+b()))*
            temp1*temp2*temp;
        return QL_SQRT(value);
    }

    double G2::discountBondOption(Option::Type type, double strike, 
                                  Time maturity, Time bondMaturity) const {

        double v = sigmaP(maturity, bondMaturity);
        double f = termStructure()->discount(bondMaturity);
        double k = termStructure()->discount(maturity)*strike;

        double w = (type==Option::Call)? 1.0 : -1.0;

        return BlackModel::formula(f, k, v, w);
    }

    double G2::V(Time t) const {
        double expat = QL_EXP(-a()*t);
        double expbt = QL_EXP(-b()*t);
        double cx = sigma()/a();
        double cy = eta()/b();
        double valuex = cx*cx*(t + (2.0*expat-0.5*expat*expat-1.5)/a());
        double valuey = cy*cy*(t + (2.0*expbt-0.5*expbt*expbt-1.5)/b());
        double value = 2.0*rho()*cx*cy* (t + (expat - 1.0)/a() 
                                         + (expbt - 1.0)/b() 
                                         - (expat*expbt-1.0)/(a()+b()));
        return valuex + valuey + value;
    }

    double G2::A(Time t, Time T) const {
        return termStructure()->discount(T)/termStructure()->discount(t)*
            QL_EXP(0.5*(V(T-t) - V(T) + V(t)));
    }

    double G2::B(double x, Time t) const {
        return (1.0 - QL_EXP(-x*t))/x;
    }

    class G2::SwaptionPricingFunction {
      public:
        SwaptionPricingFunction(double a, double sigma, double b, 
                                double eta, double rho, 
                                double w, double start, 
                                const std::vector<Time>& payTimes, 
                                Rate fixedRate, const G2& model)
        : a_(a), sigma_(sigma), b_(b), eta_(eta), rho_(rho), w_(w), 
          T_(start), t_(payTimes), rate_(fixedRate), size_(t_.size()), 
          A_(size_), Ba_(size_), Bb_(size_) {

            sigmax_ = sigma_*QL_SQRT(0.5*(1.0-QL_EXP(-2.0*a_*T_))/a_);
            sigmay_ =   eta_*QL_SQRT(0.5*(1.0-QL_EXP(-2.0*b_*T_))/b_);
            rhoxy_ = rho_*eta_*sigma_*(1.0 - QL_EXP(-(a_+b_)*T_))/
                ((a_+b_)*sigmax_*sigmay_);

            double temp = sigma_*sigma_/(a_*a_);
            mux_ = (temp+rho_*sigma_*eta_/(a_*b_))*(1.0 - QL_EXP(-a*T_)) -
                0.5*temp*(1.0 - QL_EXP(-2.0*a_*T_)) -
                rho_*sigma_*eta_/(b_*(a_+b_))*
                (1.0- QL_EXP(-(b_+a_)*T_));

            for (Size i=0; i<size_; i++) {
                A_[i] = model.A(T_, t_[i]);
                Ba_[i] = model.B(a_, t_[i]);
                Bb_[i] = model.B(b_, t_[i]);
            }
        }

        double operator()(double x) const {
            CumulativeNormalDistribution phi;
            double temp = (x - mux_)/sigmax_;
            double txy = QL_SQRT(1.0 - rhoxy_*rhoxy_);

            Array lambda(size_);
            Size i;
            for (i=0; i<size_; i++) {
                double tau = (i==0 ? t_[0] - T_ : t_[i] - t_[i-1]);
                lambda[i] = (1.0+rate_*tau)*A_[i]*QL_EXP(-Ba_[i]*x);
            }

            SolvingFunction function(lambda, Bb_) ;
            Brent s1d;
            s1d.setMaxEvaluations(1000);
            double yb = s1d.solve(function, 1e-6, 0.0, -1.0, 1.0);

            double h1 = (yb - muy_)/(sigmay_*txy) - 
                rhoxy_*(x  - mux_)/(sigmax_*txy);
            double value = phi(-w_*h1);

            for (i=0; i<size_; i++) {
                double h2 = h1 + 
                    Bb_[i]*sigmay_*QL_SQRT(1.0-rhoxy_*rhoxy_);
                double kappa = - Bb_[i] *
                    (muy_ - 0.5*txy*txy*sigmay_*sigmay_*Bb_[i] +
                     rhoxy_*sigmay_*(x-mux_)/sigmax_);
                value -= lambda[i] *QL_EXP(kappa)*phi(-w_*h2);
            }
            return QL_EXP(-0.5*temp*temp*value/
                          (sigmax_*QL_SQRT(2.0*M_PI)));
        }


      private:
        class SolvingFunction {
          public:
            SolvingFunction(const Array& lambda, const Array& Bb) 
            : lambda_(lambda), Bb_(Bb) {}
            double operator()(double y) const {
                double value = 1.0;
                for (Size i=0; i<lambda_.size(); i++) {
                    value -= lambda_[i]*QL_EXP(-Bb_[i]*y);
                }
                return value;
            }
          private:
            const Array& lambda_;
            const Array& Bb_;
        };

        double a_, sigma_, b_, eta_, rho_, w_;
        Time T_;
        std::vector<Time> t_;
        Rate rate_;
        Size size_;
        Array A_, Ba_, Bb_;
        double mux_, muy_, sigmax_, sigmay_, rhoxy_;
    };

    double G2::swaption(const Swaption::arguments& arguments) const {
        Time start = arguments.floatingResetTimes[0];
        double w = (arguments.payFixed ? 1 : -1 );
        SwaptionPricingFunction function(a(), sigma(), b(), eta(), rho(), 
                                         w, start, 
                                         arguments.floatingPayTimes, 
                                         arguments.fixedRate, (*this));
        SegmentIntegral integrator(1000);

        return arguments.nominal*w*termStructure()->discount(start)*
            integrator(function, -10000.0, 10000.0);
    }

}
