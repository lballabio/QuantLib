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

/*! \file americancondition.hpp
    \brief american option exercise condition
*/

#ifndef quantlib_fd_american_condition_h
#define quantlib_fd_american_condition_h

#include <ql/methods/finitedifferences/fdtypedefs.hpp>
#include <ql/discretizedasset.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! American exercise condition.
    class AmericanCondition :
        public StandardStepCondition {
      public:
        explicit AmericanCondition(const Array& intrinsicValues)
        : impl_(new ArrayImpl(intrinsicValues)) {}

        /*! \deprecated Use the other constructor.
                        Deprecated in version 1.19.
        */
        QL_DEPRECATED
        AmericanCondition(Option::Type type,
                          Real strike)
        : impl_(new PayoffImpl(type, strike)) {}

        void applyTo(Array &a, Time) const {
            //#pragma omp parallel for
            for (Size i = 0; i < a.size(); i++) {
                a[i] = std::max(a[i], impl_->getValue(a, i));
            }
        }
      private:
        // This part should be removed and the array-based implementation
        // inlined once the payoff-based constructor is removed.

        class Impl;

        ext::shared_ptr<Impl> impl_;

        class Impl {
          public:
            virtual ~Impl() {}
            virtual Real getValue(const Array &a,
                                  int i) = 0;
        };

        class ArrayImpl : public Impl {
          private:
            Array intrinsicValues_;
          public:
            explicit ArrayImpl(const Array &a)
            : intrinsicValues_(a) {}

            Real getValue(const Array&, int i) {
                return intrinsicValues_[i];
            }
        };

        class PayoffImpl : public Impl {
          private:
            ext::shared_ptr<const Payoff> payoff_;
          public:
            PayoffImpl(Option::Type type, Real strike)
            : payoff_(new PlainVanillaPayoff(type, strike)) {};
            Real getValue(const Array &a,
                          int i) {
                return (*payoff_)(std::exp(a[i]));
            }
        };
    };
}


#endif
