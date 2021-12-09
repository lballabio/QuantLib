/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file hestonmodel.hpp
    \brief Heston model for the stochastic volatility of an asset
*/

#ifndef quantlib_heston_model_hpp
#define quantlib_heston_model_hpp

#include <ql/models/model.hpp>
#include <ql/processes/hestonprocess.hpp>

namespace QuantLib {

    //! Heston model for the stochastic volatility of an asset
    /*! References:

        Heston, Steven L., 1993. A Closed-Form Solution for Options
        with Stochastic Volatility with Applications to Bond and
        Currency Options.  The review of Financial Studies, Volume 6,
        Issue 2, 327-343.

        \test calibration is tested against known good values.
    */
    class HestonModel : public CalibratedModel {
      public:
        explicit HestonModel(const ext::shared_ptr<HestonProcess>& process);

        // variance mean version level
        Real theta() const { return arguments_[0](0.0); }
        // variance mean reversion speed
        Real kappa() const { return arguments_[1](0.0); }
        // volatility of the volatility
        Real sigma() const { return arguments_[2](0.0); }
        // correlation
        Real rho()   const { return arguments_[3](0.0); }
        // spot variance
        Real v0()    const { return arguments_[4](0.0); }

        // underlying process
        ext::shared_ptr<HestonProcess> process() const { return process_; }

        class FellerConstraint;
      protected:
        void generateArguments() override;
        ext::shared_ptr<HestonProcess> process_;
    };

    class HestonModel::FellerConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            bool test(const Array& params) const override {
                const Real theta = params[0];
                const Real kappa = params[1];
                const Real sigma = params[2];

                return (sigma >= 0.0 && sigma*sigma < 2.0*kappa*theta);
            }
        };
      public:
        FellerConstraint()
        : Constraint(ext::shared_ptr<Constraint::Impl>(
                                           new FellerConstraint::Impl)) {}
    };
}


#endif

