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

/*! \file fdmspreadpayoffinnervalue.hpp
    \brief inner value calculator for a spread payoff
*/

#ifndef quantlib_fdm_spread_payoff_inner_value_hpp
#define quantlib_fdm_spread_payoff_inner_value_hpp

#include <ql/instruments/basketoption.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <utility>

namespace QuantLib {

    class FdmSpreadPayoffInnerValue : public FdmInnerValueCalculator {
      public:
        FdmSpreadPayoffInnerValue(ext::shared_ptr<BasketPayoff> payoff,
                                  ext::shared_ptr<FdmInnerValueCalculator> calc1,
                                  ext::shared_ptr<FdmInnerValueCalculator> calc2)
        : payoff_(std::move(payoff)), calc1_(std::move(calc1)), calc2_(std::move(calc2)) {}

        Real innerValue(const FdmLinearOpIterator& iter, Time t) override {
            Array a(2);
            a[0] = calc1_->innerValue(iter, t);
            a[1] = calc2_->innerValue(iter, t);

            return (*payoff_)(a);
        }
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
            return innerValue(iter, t);
        }

      private:
        const ext::shared_ptr<BasketPayoff> payoff_;
        const ext::shared_ptr<FdmInnerValueCalculator> calc1_;
        const ext::shared_ptr<FdmInnerValueCalculator> calc2_;
    };
}

#endif
