/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmexpextouinnervaluecalculator.hpp
    \brief inner value calculator for an exponential extended
           Ornstein Uhlenbeck grid
*/

#ifndef quantlib_fdm_exp_ext_ou_inner_value_calculator_hpp
#define quantlib_fdm_exp_ext_ou_inner_value_calculator_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/payoff.hpp>
#include <utility>

namespace QuantLib {

    class FdmExpExtOUInnerValueCalculator : public FdmInnerValueCalculator {
      public:
        typedef std::vector<std::pair<Time, Real> > Shape;

        FdmExpExtOUInnerValueCalculator(ext::shared_ptr<Payoff> payoff,
                                        ext::shared_ptr<FdmMesher> mesher,
                                        ext::shared_ptr<Shape> shape = ext::shared_ptr<Shape>(),
                                        Size direction = 0)
        : direction_(direction), payoff_(std::move(payoff)), mesher_(std::move(mesher)),
          shape_(std::move(shape)) {}

        Real innerValue(const FdmLinearOpIterator& iter, Time t) override {
            const Real u = mesher_->location(iter, direction_);

            Real f = 0;
            if (shape_ != nullptr) {
                f = std::lower_bound(shape_->begin(), shape_->end(),
                   std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;
            }

            return (*payoff_)(std::exp(f + u));
        }
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
            return innerValue(iter, t);
        }

      private:
        const Size direction_;
        const ext::shared_ptr<Payoff> payoff_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<Shape> shape_;
    };

}
#endif
