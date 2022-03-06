/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 Mike Parker

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

/*! \file g2.hpp
    \brief Two-factor additive Gaussian Model G2++
*/

#ifndef quantlib_two_factor_models_g2_h
#define quantlib_two_factor_models_g2_h

#include <ql/instruments/swaption.hpp>
#include <ql/models/shortrate/twofactormodel.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <utility>

namespace QuantLib {

    //! Two-additive-factor gaussian model class.
    /*! This class implements a two-additive-factor model defined by
        \f[
            dr_t = \varphi(t) + x_t + y_t
        \f]
        where \f$ x_t \f$ and \f$ y_t \f$ are defined by
        \f[
            dx_t = -a x_t dt + \sigma dW^1_t, x_0 = 0
        \f]
        \f[
            dy_t = -b y_t dt + \sigma dW^2_t, y_0 = 0
        \f]
        and \f$ dW^1_t dW^2_t = \rho dt \f$.

        \bug This class was not tested enough to guarantee
             its functionality.

        \ingroup shortrate
    */
    class G2 : public TwoFactorModel,
               public AffineModel,
               public TermStructureConsistentModel {
      public:
        G2(const Handle<YieldTermStructure>& termStructure,
           Real a = 0.1,
           Real sigma = 0.01,
           Real b = 0.1,
           Real eta = 0.01,
           Real rho = -0.75);

        ext::shared_ptr<ShortRateDynamics> dynamics() const override;

        Real discountBond(Time now, Time maturity, Array factors) const override {
            QL_REQUIRE(factors.size()>1,
                       "g2 model needs two factors to compute discount bond");
            return discountBond(now, maturity, factors[0], factors[1]);
        }

        Real discountBond(Time, Time, Rate, Rate) const;

        Real discountBondOption(Option::Type type,
                                Real strike,
                                Time maturity,
                                Time bondMaturity) const override;

        Real swaption(const Swaption::arguments& arguments,
                      Rate fixedRate,
                      Real range,
                      Size intervals) const;

        DiscountFactor discount(Time t) const override { return termStructure()->discount(t); }

        Real a() const { return a_(0.0); }
        Real sigma() const { return sigma_(0.0); }
        Real b() const { return b_(0.0); }
        Real eta() const { return eta_(0.0); }
        Real rho() const { return rho_(0.0); }

      protected:
        void generateArguments() override;

        Real A(Time t, Time T) const;
        Real B(Real x, Time t) const;

      private:
        class Dynamics;
        class FittingParameter;

        Real sigmaP(Time t, Time s) const;

        Parameter& a_;
        Parameter& sigma_;
        Parameter& b_;
        Parameter& eta_;
        Parameter& rho_;

        Parameter phi_;

        Real V(Time t) const;

        class SwaptionPricingFunction;
        friend class SwaptionPricingFunction;
    };

    class G2::Dynamics : public TwoFactorModel::ShortRateDynamics {
      public:
        Dynamics(Parameter fitting, Real a, Real sigma, Real b, Real eta, Real rho)
        : ShortRateDynamics(
              ext::shared_ptr<StochasticProcess1D>(new OrnsteinUhlenbeckProcess(a, sigma)),
              ext::shared_ptr<StochasticProcess1D>(new OrnsteinUhlenbeckProcess(b, eta)),
              rho),
          fitting_(std::move(fitting)) {}
        Rate shortRate(Time t, Real x, Real y) const override { return fitting_(t) + x + y; }

      private:
        Parameter fitting_;
    };

    //! Analytical term-structure fitting parameter \f$ \varphi(t) \f$.
    /*! \f$ \varphi(t) \f$ is analytically defined by
        \f[
            \varphi(t) = f(t) +
                 \frac{1}{2}(\frac{\sigma(1-e^{-at})}{a})^2 +
                 \frac{1}{2}(\frac{\eta(1-e^{-bt})}{b})^2 +
                 \rho\frac{\sigma(1-e^{-at})}{a}\frac{\eta(1-e^{-bt})}{b},
        \f]
        where \f$ f(t) \f$ is the instantaneous forward rate at \f$ t \f$.
    */
    class G2::FittingParameter : public TermStructureFittingParameter {
      private:
        class Impl : public Parameter::Impl {
          public:
            Impl(Handle<YieldTermStructure> termStructure,
                 Real a,
                 Real sigma,
                 Real b,
                 Real eta,
                 Real rho)
            : termStructure_(std::move(termStructure)), a_(a), sigma_(sigma), b_(b), eta_(eta),
              rho_(rho) {}

            Real value(const Array&, Time t) const override {
                Rate forward = termStructure_->forwardRate(t, t,
                                                           Continuous,
                                                           NoFrequency);
                Real temp1 = sigma_*(1.0-std::exp(-a_*t))/a_;
                Real temp2 = eta_*(1.0-std::exp(-b_*t))/b_;
                Real value = 0.5*temp1*temp1 + 0.5*temp2*temp2 +
                    rho_*temp1*temp2 + forward;
                return value;
            }

          private:
            Handle<YieldTermStructure> termStructure_;
            Real a_, sigma_, b_, eta_, rho_;
        };
      public:
        FittingParameter(const Handle<YieldTermStructure>& termStructure,
                         Real a,
                         Real sigma,
                         Real b,
                         Real eta,
                         Real rho)
        : TermStructureFittingParameter(ext::shared_ptr<Parameter::Impl>(
                          new FittingParameter::Impl(termStructure, a, sigma,
                                                     b, eta, rho))) {}
    };

}


#endif



#ifndef id_fcb7b51d6053a1c3ec6b896d158b1360
#define id_fcb7b51d6053a1c3ec6b896d158b1360
inline bool test_fcb7b51d6053a1c3ec6b896d158b1360(int* i) { return i != 0; }
#endif
