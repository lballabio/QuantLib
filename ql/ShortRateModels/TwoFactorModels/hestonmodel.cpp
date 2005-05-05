/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/ShortRateModels/TwoFactorModels/hestonmodel.hpp>

namespace QuantLib {

    class HestonModel::VolatilityConstraint : public Constraint {
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
        VolatilityConstraint()
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                           new VolatilityConstraint::Impl)) {}
    };

    HestonModel::HestonModel(const boost::shared_ptr<HestonProcess> & process)
    : ShortRateModel(5), theta_(arguments_[0]), kappa_(arguments_[1]),
      sigma_(arguments_[2]), rho_(arguments_[3]), v0_(arguments_[4]) {
        kappa_ = ConstantParameter(process->kappa(), PositiveConstraint());
        theta_ = ConstantParameter(process->theta(), PositiveConstraint());
        sigma_ = ConstantParameter(process->sigma(), PositiveConstraint());
        rho_   = ConstantParameter(process->rho(),
                                   BoundaryConstraint(-1.0, 1.0));
        v0_    = ConstantParameter(process->v0(), PositiveConstraint());
	}

    boost::shared_ptr<NumericalMethod> HestonModel::tree(
                                                      const TimeGrid&) const {
        QL_FAIL("tree not yet supported");
    }

}

