/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2021 Magnus Mencke

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

/*! \file extendedcoxingersollross.hpp
    \brief Extended Cox-Ingersoll-Ross model
*/

#ifndef quantlib_extended_cox_ingersoll_ross_hpp
#define quantlib_extended_cox_ingersoll_ross_hpp

#include <ql/models/shortrate/onefactormodels/coxingersollross.hpp>
#include <utility>

namespace QuantLib {

    //! Extended Cox-Ingersoll-Ross model class.
    /*! This class implements the extended Cox-Ingersoll-Ross model
        defined by
        \f[
            r(t) = \varphi(t)+y(t)
        \f]
        where \f$ \varphi(t) \f$ is the deterministic time-dependent
        parameter used for term-structure fitting and \f$ y_t \f$ is a standard CIR process.

        \bug this class was not tested enough to guarantee
             its functionality.

        \ingroup shortrate
    */
    class ExtendedCoxIngersollRoss : public CoxIngersollRoss,
                                     public TermStructureConsistentModel {
      public:
        ExtendedCoxIngersollRoss(
                              const Handle<YieldTermStructure>& termStructure,
                              Real theta = 0.1,
                              Real k = 0.1,
                              Real sigma = 0.1,
                              Real x0 = 0.05,
                              bool withFellerConstraint = true);

        ext::shared_ptr<Lattice> tree(const TimeGrid& grid) const override;

        ext::shared_ptr<ShortRateDynamics> dynamics() const override;

        Real discountBondOption(Option::Type type,
                                Real strike,
                                Time maturity,
                                Time bondMaturity) const override;

      protected:
        void generateArguments() override;
        Real A(Time t, Time T) const override;

      private:
        class Dynamics;
        class FittingParameter;

        Parameter phi_;
    };

    //! Short-rate dynamics in the extended Cox-Ingersoll-Ross model
    /*! The short-rate is here
        \f[
            r(t) = \varphi(t) + y(t)
        \f]
        where \f$ \varphi(t) \f$ is the deterministic time-dependent
        parameter used for term-structure fitting and \f$ y_t \f$ is a standard CIR process
        with dynamics
        \f[
            dy(t)=k(\theta-y(t))dt+\sigma \sqrt{y(t)}dW(t)
        \f]
    */
    class ExtendedCoxIngersollRoss::Dynamics
        : public CoxIngersollRoss::Dynamics {
      public:
        Dynamics(Parameter phi, Real theta, Real k, Real sigma, Real x0)
        : CoxIngersollRoss::Dynamics(theta, k, sigma, x0), phi_(std::move(phi)) {}

        Real variable(Time t, Rate r) const override { return r - phi_(t); }
        Real shortRate(Time t, Real y) const override { return y + phi_(t); }

      private:
        Parameter phi_;
    };

    //! Analytical term-structure fitting parameter \f$ \varphi(t) \f$.
    /*! \f$ \varphi(t) \f$ is analytically defined by
        \f[
            \varphi(t) = f(t) -
                         \frac{2k\theta(e^{th}-1)}{2h+(k+h)(e^{th}-1)} -
                         \frac{4 x_0 h^2 e^{th}}{(2h+(k+h)(e^{th}-1))^1},
        \f]
        where \f$ f(t) \f$ is the instantaneous forward rate at \f$ t \f$
        and \f$ h = \sqrt{k^2 + 2\sigma^2} \f$.
    */
    class ExtendedCoxIngersollRoss::FittingParameter
        : public TermStructureFittingParameter {
      private:
        class Impl : public Parameter::Impl {
          public:
            Impl(Handle<YieldTermStructure> termStructure, Real theta, Real k, Real sigma, Real x0)
            : termStructure_(std::move(termStructure)), theta_(theta), k_(k), sigma_(sigma),
              x0_(x0) {}

            Real value(const Array&, Time t) const override {
                Rate forwardRate =
                    termStructure_->forwardRate(t, t, Continuous, NoFrequency);
                Real h = std::sqrt(k_*k_ + 2.0*sigma_*sigma_);
                Real expth = std::exp(t*h);
                Real temp = 2.0*h + (k_+h)*(expth-1.0);
                Real phi = forwardRate -
                    2.0*k_*theta_*(expth - 1.0)/temp -
                    x0_*4.0*h*h*expth/(temp*temp);
                return phi;
            }

          private:
            Handle<YieldTermStructure> termStructure_;
            Real theta_, k_, sigma_, x0_;
        };
      public:
        FittingParameter(const Handle<YieldTermStructure>& termStructure,
                         Real theta, Real k, Real sigma, Real x0)
        : TermStructureFittingParameter(ext::shared_ptr<Parameter::Impl>(
                 new FittingParameter::Impl(
                                     termStructure, theta, k, sigma, x0))) {}
    };

    // inline definitions

    inline ext::shared_ptr<OneFactorModel::ShortRateDynamics>
    ExtendedCoxIngersollRoss::dynamics() const {
        return ext::shared_ptr<ShortRateDynamics>(
                            new Dynamics(phi_, theta(), k() , sigma(), x0()));
    }

    inline void ExtendedCoxIngersollRoss::generateArguments() {
        phi_ = FittingParameter(termStructure(), theta(), k(), sigma(), x0());
    }

}


#endif
