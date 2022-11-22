/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file vasicek.hpp
    \brief Vasicek model class
*/

#ifndef quantlib_vasicek_hpp
#define quantlib_vasicek_hpp

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>

namespace QuantLib {

    //! %Vasicek model class
    /*! This class implements the Vasicek model defined by
        \f[
            dr_t = a(b - r_t)dt + \sigma dW_t ,
        \f]
        where \f$ a \f$, \f$ b \f$ and \f$ \sigma \f$ are constants;
        a risk premium \f$ \lambda \f$ can also be specified.

        \ingroup shortrate
    */
    class Vasicek : public OneFactorAffineModel {
      public:
        Vasicek(Rate r0 = 0.05,
                Real a = 0.1, Real b = 0.05, Real sigma = 0.01,
                Real lambda = 0.0);
        Real discountBondOption(Option::Type type,
                                Real strike,
                                Time maturity,
                                Time bondMaturity) const override;

        ext::shared_ptr<ShortRateDynamics> dynamics() const override;

        Real a() const { return a_(0.0); }
        Real b() const { return b_(0.0); }
        Real lambda() const { return lambda_(0.0); }
        Real sigma() const { return sigma_(0.0); }
        Real r0() const { return r0_;}

      protected:
        Real A(Time t, Time T) const override;
        Real B(Time t, Time T) const override;

        Real r0_;
        Parameter& a_;
        Parameter& b_;
        Parameter& sigma_;
        Parameter& lambda_;
      private:
        class Dynamics;

    };

    //! Short-rate dynamics in the %Vasicek model
    /*! The short-rate follows an Ornstein-Uhlenbeck process with mean
        \f$ b \f$.
    */
    class Vasicek::Dynamics : public OneFactorModel::ShortRateDynamics {
      public:
        Dynamics(Real a,
                 Real b,
                 Real sigma,
                 Real r0)
        : ShortRateDynamics(ext::shared_ptr<StochasticProcess1D>(
                             new OrnsteinUhlenbeckProcess(a, sigma, r0 - b))),
          b_(b) {}

        Real variable(Time, Rate r) const override { return r - b_; }
        Real shortRate(Time, Real x) const override { return x + b_; }

      private:
        Real b_;
    };


    // inline definitions

    inline ext::shared_ptr<OneFactorModel::ShortRateDynamics>
    Vasicek::dynamics() const {
        return ext::shared_ptr<ShortRateDynamics>(
                                     new Dynamics(a(), b() , sigma(), r0_));
    }

}


#endif

