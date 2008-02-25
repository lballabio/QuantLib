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

#include <ql/models/equity/hestonmodel.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    class HestonModel::FellerConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            bool test(const Array& params) const {
                const Real theta = params[0];
                const Real kappa = params[1];
                const Real sigma = params[2];

                return (sigma >= 0.0 && sigma*sigma < 2.0*kappa*theta);
            }
        };
      public:
        FellerConstraint()
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                           new FellerConstraint::Impl)) {}
    };

    HestonModel::HestonModel(const boost::shared_ptr<HestonProcess> & process)
    : CalibratedModel(5), process_(process) {
        arguments_[0] = ConstantParameter(process->theta(),
                                          PositiveConstraint());
        arguments_[1] = ConstantParameter(process->kappa(),
                                          PositiveConstraint());
        arguments_[2] = ConstantParameter(process->sigma(),
                                          PositiveConstraint());
        arguments_[3] = ConstantParameter(process->rho(),
                                          BoundaryConstraint(-1.0, 1.0));
        arguments_[4] = ConstantParameter(process->v0(),
                                          PositiveConstraint());
        generateArguments();

        registerWith(process_->riskFreeRate());
        registerWith(process_->dividendYield());
        registerWith(process_->s0());
    }

    void HestonModel::generateArguments() {
        process_.reset(new HestonProcess(process_->riskFreeRate(),
                                         process_->dividendYield(),
                                         process_->s0(),
                                         v0(), kappa(), theta(),
                                         sigma(), rho()));
    }

}

