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

/*! \file fdoujumpmodelinnervalue.hpp
    \brief inner value calculator for the Ornstein Uhlenbeck
           plus exponential jumps model (Kluge Model)
*/

#ifndef quantlib_fdm_ext_ou_jump_model_inner_value_hpp
#define quantlib_fdm_ext_ou_jump_model_inner_value_hpp

#include <ql/payoff.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmlinearopiterator.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>

namespace QuantLib {

    class FdmExtOUJumpModelInnerValue : public FdmInnerValueCalculator {
      public:
        FdmExtOUJumpModelInnerValue(const boost::shared_ptr<Payoff>& payoff,
                                    const boost::shared_ptr<FdmMesher>& mesher)
        : payoff_(payoff),
          mesher_(mesher) { }

        Real innerValue(const FdmLinearOpIterator& iter) {
            const Real x = mesher_->location(iter, 0);
            const Real y = mesher_->location(iter, 1);

            return payoff_->operator()(std::exp(x+y));
        }
        Real avgInnerValue(const FdmLinearOpIterator& iter) {
            return innerValue(iter);
        }

      private:
        const boost::shared_ptr<Payoff> payoff_;
        const boost::shared_ptr<FdmMesher> mesher_;
    };
}

#endif
