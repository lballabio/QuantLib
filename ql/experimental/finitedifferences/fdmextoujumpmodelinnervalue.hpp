/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmextoujumpmodelinnervalue.hpp
    \brief inner value calculator for the Ornstein Uhlenbeck
           plus exponential jumps model (Kluge Model)
*/

#ifndef quantlib_fdm_ext_ou_jump_model_inner_value_hpp
#define quantlib_fdm_ext_ou_jump_model_inner_value_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/payoff.hpp>
#include <utility>

namespace QuantLib {

    class FdmExtOUJumpModelInnerValue : public FdmInnerValueCalculator {
      public:
        typedef std::vector<std::pair<Time, Real> > Shape;

        FdmExtOUJumpModelInnerValue(std::shared_ptr<Payoff> payoff,
                                    std::shared_ptr<FdmMesher> mesher,
                                    std::shared_ptr<Shape> shape = std::shared_ptr<Shape>())
        : payoff_(std::move(payoff)), mesher_(std::move(mesher)), shape_(std::move(shape)) {}

        Real innerValue(const FdmLinearOpIterator& iter, Time t) override {
            const Real x = mesher_->location(iter, 0);
            const Real y = mesher_->location(iter, 1);

            Real f = 0;
            if (shape_ != nullptr) {
                f = std::lower_bound(shape_->begin(), shape_->end(),
                   std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;
            }
            return (*payoff_)(std::exp(f + x + y));
        }
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
            return innerValue(iter, t);
        }

      private:
        const std::shared_ptr<Payoff> payoff_;
        const std::shared_ptr<FdmMesher> mesher_;
        const std::shared_ptr<Shape> shape_;
    };
}

#endif
