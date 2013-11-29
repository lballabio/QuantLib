/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file hullwhite.hpp
    \brief Hull & White (HW) model
*/

#ifndef quantlib_hull_white_hpp
#define quantlib_hull_white_hpp

#include <ql/models/shortrate/onefactormodels/vasicek.hpp>

namespace QuantLib {

    //! Single-factor Hull-White (extended %Vasicek) model class.
    /*! This class implements the standard single-factor Hull-White model
        defined by
        \f[
            dr_t = (\theta(t) - \alpha r_t)dt + \sigma dW_t
        \f]
        where \f$ \alpha \f$ and \f$ \sigma \f$ are constants.

        \test calibration results are tested against cached values

        \bug When the term structure is relinked, the r0 parameter of
             the underlying Vasicek model is not updated.

        \ingroup shortrate
    */
    class HullWhite : public Vasicek, public TermStructureConsistentModel {
      public:
        HullWhite(const Handle<YieldTermStructure>& termStructure,
                  Real a = 0.1, Real sigma = 0.01);

        boost::shared_ptr<Lattice> tree(const TimeGrid& grid) const;

        boost::shared_ptr<ShortRateDynamics> dynamics() const;

        Real discountBondOption(Option::Type type,
                                Real strike,
                                Time maturity,
                                Time bondMaturity) const;

       Real discountBondOption(Option::Type type,
                               Real strike,
                               Time maturity,
                               Time bondStart,
                               Time bondMaturity) const;

        /*! Futures convexity bias (i.e., the difference between
            futures implied rate and forward rate) calculated as in
            G. Kirikos, D. Novak, "Convexity Conundrums", Risk
            Magazine, March 1997.

            \note t and T should be expressed in yearfraction using
                  deposit day counter, F_quoted is futures' market price.
        */
        static Rate convexityBias(Real futurePrice,
                                  Time t,
                                  Time T,
                                  Real sigma,
                                  Real a);

        static std::vector<bool> FixedReversion() {
            std::vector<bool> c(2);
            c[0] = true; c[1] = false;
            return c;
        }

      protected:
        void generateArguments();

        Real A(Time t, Time T) const;

      private:
        class Dynamics;
        class FittingParameter;

        Parameter phi_;
    };

    //! Short-rate dynamics in the Hull-White model
    /*! The short-rate is here
        \f[
            r_t = \varphi(t) + x_t
        \f]
        where \f$ \varphi(t) \f$ is the deterministic time-dependent
        parameter used for term-structure fitting and \f$ x_t \f$ is the
        state variable following an Ornstein-Uhlenbeck process.
    */
    class HullWhite::Dynamics : public OneFactorModel::ShortRateDynamics {
      public:
        Dynamics(const Parameter& fitting,
                 Real a,
                 Real sigma)
        : ShortRateDynamics(boost::shared_ptr<StochasticProcess1D>(
                                     new OrnsteinUhlenbeckProcess(a, sigma))),
          fitting_(fitting) {}

        Real variable(Time t, Rate r) const {
            return r - fitting_(t);
        }
        Real shortRate(Time t, Real x) const {
            return x + fitting_(t);
        }
      private:
        Parameter fitting_;
    };

    //! Analytical term-structure fitting parameter \f$ \varphi(t) \f$.
    /*! \f$ \varphi(t) \f$ is analytically defined by
        \f[
            \varphi(t) = f(t) + \frac{1}{2}[\frac{\sigma(1-e^{-at})}{a}]^2,
        \f]
        where \f$ f(t) \f$ is the instantaneous forward rate at \f$ t \f$.
    */
    class HullWhite::FittingParameter
        : public TermStructureFittingParameter {
      private:
        class Impl : public Parameter::Impl {
          public:
            Impl(const Handle<YieldTermStructure>& termStructure,
                 Real a, Real sigma)
            : termStructure_(termStructure), a_(a), sigma_(sigma) {}

            Real value(const Array&, Time t) const {
                Rate forwardRate =
                    termStructure_->forwardRate(t, t, Continuous, NoFrequency);
                Real temp = a_ < std::sqrt(QL_EPSILON) ?
                            sigma_*t :
                            sigma_*(1.0 - std::exp(-a_*t))/a_;
                return (forwardRate + 0.5*temp*temp);
            }
          private:
            Handle<YieldTermStructure> termStructure_;
            Real a_, sigma_;
        };
      public:
        FittingParameter(const Handle<YieldTermStructure>& termStructure,
                         Real a, Real sigma)
        : TermStructureFittingParameter(boost::shared_ptr<Parameter::Impl>(
                      new FittingParameter::Impl(termStructure, a, sigma))) {}
    };


    // inline definitions

    inline boost::shared_ptr<OneFactorModel::ShortRateDynamics>
    HullWhite::dynamics() const {
        return boost::shared_ptr<ShortRateDynamics>(
                                            new Dynamics(phi_, a(), sigma()));
    }

}


#endif

