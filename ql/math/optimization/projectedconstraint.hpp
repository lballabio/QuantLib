/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file projectedconstraint.hpp
    \brief Projected constraint
*/

#ifndef quantlib_optimization_projectedconstraint_h
#define quantlib_optimization_projectedconstraint_h

#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/projection.hpp>
#include <utility>

namespace QuantLib {

    class ProjectedConstraint : public Constraint {

      private:

        class Impl final : public Constraint::Impl {
          public:
            Impl(Constraint constraint,
                 const Array& parameterValues,
                 const std::vector<bool>& fixParameters)
            : constraint_(std::move(constraint)), projection_(parameterValues, fixParameters) {}
            Impl(Constraint constraint, const Projection& projection)
            : constraint_(std::move(constraint)), projection_(projection) {}
            bool test(const Array& params) const override {
                return constraint_.test(projection_.include(params));
            }
            Array upperBound(const Array& params) const override {
                return projection_.project(constraint_.upperBound(projection_.include(params)));
            }
            Array lowerBound(const Array& params) const override {
                return projection_.project(constraint_.lowerBound(projection_.include(params)));
            }

          private:
            const Constraint constraint_;
            const Projection projection_;
        };

      public:

        ProjectedConstraint(const Constraint &constraint,
                            const Array &parameterValues,
                            const std::vector<bool> &fixParameters)
            : Constraint(ext::shared_ptr<Constraint::Impl>(
                  new ProjectedConstraint::Impl(constraint, parameterValues,
                                                fixParameters))) {}

        ProjectedConstraint(const Constraint &constraint,
                            const Projection &projection)
            : Constraint(ext::shared_ptr<Constraint::Impl>(
                  new ProjectedConstraint::Impl(constraint, projection))) {}
    };
}

#endif
