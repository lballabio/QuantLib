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

/*! \file fdmexpextouinnervaluecalculator.hpp
    \brief inner value calculator for an exponential extended
           Ornstein Uhlenbeck grid
*/

#ifndef quantlib_fdm_exp_ext_ou_inner_value_calculator_hpp
#define quantlib_fdm_exp_ext_ou_inner_value_calculator_hpp

#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    class FdmExpExtOUInnerValueCalculator : public FdmInnerValueCalculator {
      public:
        typedef std::vector<std::pair<Time, Real> > Shape;

        FdmExpExtOUInnerValueCalculator(
            const boost::shared_ptr<Payoff>& payoff,
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<Shape>& shape = boost::shared_ptr<Shape>(),
            Size direction = 0)
        : direction_(direction),
          payoff_(payoff),
          mesher_(mesher),
          shape_(shape) { }

        Real innerValue(const FdmLinearOpIterator& iter, Time t) {
            const Real u = mesher_->location(iter, direction_);

            Real f = 0;
            if (shape_) {
                f = std::lower_bound(shape_->begin(), shape_->end(),
                   std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;
            }

            return payoff_->operator()(std::exp(f + u));
        }
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) {
            return innerValue(iter, t);
        }
      private:
        const Size direction_;
        const boost::shared_ptr<Payoff> payoff_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<Shape> shape_;
    };

}
#endif
