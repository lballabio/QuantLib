/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 SunTrust Bank
 Copyright (C) 2010 Cavit Hafizoglu

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

namespace QuantLib {

    //! Generalized Hull-White model class.
    /*! This class implements the standard Black-Karasinski model defined by
        \f[
        d f(r_t) = (\theta(t) - \alpha f(r_t))dt + \sigma dW_t,
        \f]
        where \f$ alpha \f$ and \f$ sigma \f$ are piecewise linear functions.

        \ingroup shortrate
    */
    class GeneralizedHullWhite : public OneFactorModel,
                                 public TermStructureConsistentModel {

        // TO DO: Build your fInverse class according to your
        // model. The template class T should overload the () operator
        // to return a function value, f inverse.

      public:

        GeneralizedHullWhite(
            const Handle<YieldTermStructure>& yieldtermStructure,
            const std::vector<Date>& speedstructure,
            const std::vector<Date>& volstructure);

        GeneralizedHullWhite(
            const Handle<YieldTermStructure>& yieldtermStructure,
            const std::vector<Date>& speedstructure,
            const std::vector<Date>& volstructure,
            const std::vector<Real>& speed,
            const std::vector<Real>& vol);

        boost::shared_ptr<ShortRateDynamics> dynamics() const {
            QL_FAIL("no defined process for generalized Hull-White model");
        }

        boost::shared_ptr<Lattice> tree(const TimeGrid& grid)const;

      private:

        class Dynamics;
        class Helper;

        std::vector<Date> speedstructure_;
        std::vector<Date> volstructure_;
        std::vector<Time> speedperiods_;
        std::vector<Time> volperiods_;

        boost::function<Real (Time)> speed() const;
        boost::function<Real (Time)> vol() const;

        Parameter& a_;
        Parameter& sigma_;
        Parameter phi_;

    };

    //! Short-rate dynamics in the generalized Hull-White model
    /*! The short-rate is here

        f(r_t) = x_t + g(t)

        where g is the deterministic time-dependent
        parameter (which can not be determined analytically)
        used for term-structure fitting and  x_t is the state
        variable following an Ornstein-Uhlenbeck process.
    */
    class GeneralizedHullWhite::Dynamics
        : public GeneralizedHullWhite::ShortRateDynamics {
      public:
        Dynamics(const Parameter& fitting,
                 const boost::function<Real (Time)>& alpha,
                 const boost::function<Real (Time)>& sigma)
        : ShortRateDynamics(boost::shared_ptr<StochasticProcess1D>(
                      new GeneralizedOrnsteinUhlenbeckProcess(alpha, sigma))),
          fitting_(fitting) {}

        Real variable(Time t, Rate r) const {
            return std::log(r) - fitting_(t);
        }

        Real shortRate(Time t, Real x) const {
            return std::exp(x + fitting_(t));
        }

      private:
        Parameter fitting_;
    };

}


#endif

