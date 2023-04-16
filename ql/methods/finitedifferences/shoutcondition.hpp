/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

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

/*! \file shoutcondition.hpp
    \brief shout option exercise condition
*/

#ifndef quantlib_fd_shout_condition_hpp
#define quantlib_fd_shout_condition_hpp

#include <ql/discretizedasset.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/methods/finitedifferences/fdtypedefs.hpp>
#include <utility>

namespace QuantLib {

    //! Shout option condition
    /*! \deprecated Use the new finite-differences framework instead.
                    Deprecated in version 1.27.
    */
    class QL_DEPRECATED ShoutCondition : public StandardStepCondition {
      public:
        ShoutCondition(const Array& intrinsicValues,
                       Time resTime,
                       Rate rate)
        : resTime_(resTime), rate_(rate),
          impl_(new ArrayImpl(intrinsicValues)) {}

        void applyTo(Array& a, Time t) const override {
            DiscountFactor B = std::exp(-rate_ * (t - resTime_));
            //#pragma omp parallel for
            for (Size i = 0; i < a.size(); i++) {
                a[i] = std::max(a[i], B * impl_->getValue(a, i));
            }
        }

      private:
        Time resTime_;
        Rate rate_;

        // This part should be removed and the array-based implementation
        // inlined once the payoff-based constructor is removed.

        class Impl;

        std::shared_ptr<Impl> impl_;

        class Impl {
          public:
            virtual ~Impl() = default;
            virtual Real getValue(const Array &a,
                                  int i) = 0;
        };

        class ArrayImpl : public Impl {
          private:
            Array intrinsicValues_;
          public:
            explicit ArrayImpl(Array a) : intrinsicValues_(std::move(a)) {}

            Real getValue(const Array&, int i) override { return intrinsicValues_[i]; }
        };
    };

}


#endif
