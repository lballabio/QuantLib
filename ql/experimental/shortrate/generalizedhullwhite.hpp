/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 SunTrust Bank
 Copyright (C) 2010, 2014 Cavit Hafizoglu

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

/*! \file generalizedornsteinuhlenbeckprocess.hpp
    \brief Ornstein-Uhlenbeck process with piecewise linear coefficients
*/

#ifndef quantlib_generalized_hull_white_hpp
#define quantlib_generalized_hull_white_hpp

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/experimental/shortrate/generalizedornsteinuhlenbeckprocess.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>

namespace QuantLib {

    //! Generalized Hull-White model class.
    /*! This class implements the standard Black-Karasinski model defined by
        \f[
        d f(r_t) = (\theta(t) - \alpha f(r_t))dt + \sigma dW_t,
        \f]
        where \f$ alpha \f$ and \f$ sigma \f$ are piecewise linear functions.

        \ingroup shortrate
    */
    class GeneralizedHullWhite : public OneFactorAffineModel,
                                 public TermStructureConsistentModel {
      public:

        GeneralizedHullWhite(
            const Handle<YieldTermStructure>& yieldtermStructure,
            const std::vector<Date>& speedstructure,
            const std::vector<Date>& volstructure,
            const boost::function<Real(Real)>& f =
                                            boost::function<Real(Real)>(),
            const boost::function<Real(Real)>& fInverse =
                                            boost::function<Real(Real)>());

        GeneralizedHullWhite(
            const Handle<YieldTermStructure>& yieldtermStructure,
            const std::vector<Date>& speedstructure,
            const std::vector<Date>& volstructure,
            const std::vector<Real>& speed,
            const std::vector<Real>& vol,
            const boost::function<Real(Real)>& f =
                                            boost::function<Real(Real)>(),
            const boost::function<Real(Real)>& fInverse =
                                            boost::function<Real(Real)>());

        boost::shared_ptr<ShortRateDynamics> dynamics() const {
            QL_FAIL("no defined process for generalized Hull-White model, "
                    "use HWdynamics()");
        }

        boost::shared_ptr<Lattice> tree(const TimeGrid& grid)const;

        //Analytical calibration of HW

        GeneralizedHullWhite(
                  const Handle<YieldTermStructure>& yieldtermStructure,
                  Real a = 0.1, Real sigma = 0.01);


        boost::shared_ptr<ShortRateDynamics> HWdynamics() const;

        Real discountBondOption(Option::Type type,
                                Real strike,
                                Time maturity,
                                Time bondMaturity) const;

        static Rate convexityBias(Real futurePrice,
                                  Time t,
                                  Time T,
                                  Real sigma,
                                  Real a);

      protected:
        //Analytical calibration of HW
        Real a() const { return a_(0.0); }
        Real sigma() const { return sigma_(0.0); }
        void generateArguments();
        Real A(Time t, Time T) const;
        Real B(Time t, Time T) const;

      private:

        class Dynamics;
        class Helper;
        class FittingParameter;// for analytic HW fitting

        std::vector<Date> speedstructure_;
        std::vector<Date> volstructure_;
        std::vector<Time> speedperiods_;
        std::vector<Time> volperiods_;

        boost::function<Real (Time)> speed() const;
        boost::function<Real (Time)> vol() const;

        Parameter& a_;
        Parameter& sigma_;
        Parameter phi_;

        boost::function<Real(Real)> f_;
        boost::function<Real(Real)> fInverse_;
    };

    //! Short-rate dynamics in the generalized Hull-White model
    /*! The short-rate is here

        f(r_t) = x_t + g(t)

        where g is the deterministic time-dependent
        parameter (which can't be determined analytically)
        used for initial term-structure fitting and  x_t is the state
        variable following an Ornstein-Uhlenbeck process.

        In this version, the function f may also be defined as a piece-wise linear
        function and can be calibrated to the away-from-the-money instruments.

    */
    class GeneralizedHullWhite::Dynamics
        : public GeneralizedHullWhite::ShortRateDynamics {
      public:
        Dynamics(const Parameter& fitting,
                 const boost::function<Real (Time)>& alpha,
                 const boost::function<Real (Time)>& sigma,
                 const boost::function<Real(Real)>& f,
                 const boost::function<Real(Real)>& fInverse)
        : ShortRateDynamics(boost::shared_ptr<StochasticProcess1D>(
                      new GeneralizedOrnsteinUhlenbeckProcess(alpha, sigma))),
          fitting_(fitting),
          _f_(f), _fInverse_(fInverse) {}

        //classical HW dynamics
        Dynamics(const Parameter& fitting,
                 Real a,
                 Real sigma)
        : GeneralizedHullWhite::ShortRateDynamics(
              boost::shared_ptr<StochasticProcess1D>(
                      new OrnsteinUhlenbeckProcess(a, sigma))),
          fitting_(fitting) {

            _f_=identity();
            _fInverse_=identity();
        }

        Real variable(Time t, Rate r=0.01) const {
            return _f_(r) - fitting_(t);
        }

        Real shortRate(Time t, Real x) const {
            return _fInverse_(x + fitting_(t));
        }

      private:
        Parameter fitting_;
        boost::function<Real(Real)> _f_;
        boost::function<Real(Real)> _fInverse_;
        struct identity {
            Real operator()(Real x) const {return x;};
        };
    };

    //! Analytical term-structure fitting parameter \f$ \varphi(t) \f$.
    /*! \f$ \varphi(t) \f$ is analytically defined by
        \f[
            \varphi(t) = f(t) + \frac{1}{2}[\frac{\sigma(1-e^{-at})}{a}]^2,
        \f]
        where \f$ f(t) \f$ is the instantaneous forward rate at \f$ t \f$.
    */
    class GeneralizedHullWhite::FittingParameter
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

    // Analytic fitting dynamics
    inline boost::shared_ptr<OneFactorModel::ShortRateDynamics>
    GeneralizedHullWhite::HWdynamics() const {
        return boost::shared_ptr<ShortRateDynamics>(
                                            new Dynamics(phi_, a(), sigma()));
    }

}


#endif
