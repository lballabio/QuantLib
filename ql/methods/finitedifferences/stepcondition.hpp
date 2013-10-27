/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file stepcondition.hpp
    \brief conditions to be applied at every time step
*/

#ifndef quantlib_step_condition_hpp
#define quantlib_step_condition_hpp

#include <ql/math/array.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! condition to be applied at every time step
    /*! \ingroup findiff */
    template <class array_type>
    class StepCondition {
      public:
        virtual ~StepCondition() {}
        virtual void applyTo(array_type& a, Time t) const = 0;
    };

    /* Abstract base class which allows step conditions to use both
       payoff and array functions */
    template <class array_type>
    class CurveDependentStepCondition :
        public StepCondition<array_type> {
      public:
        void applyTo(Array &a, Time) const {
            #pragma omp parallel for
            for (Size i = 0; i < a.size(); i++) {
                a[i] =
                    applyToValue(a[i], getValue(a,i));
            }
        }
      protected:
        CurveDependentStepCondition(Option::Type type, Real strike)
            : curveItem_(new PayoffWrapper(type, strike)) {};
        CurveDependentStepCondition(const Payoff *p)
            : curveItem_(new PayoffWrapper(p)) {};
        CurveDependentStepCondition(const array_type & a)
            : curveItem_(new ArrayWrapper(a)) {};
        class CurveWrapper;

        boost::shared_ptr<CurveWrapper> curveItem_;
        Real getValue(const array_type &a, Size index) const {
            return curveItem_->getValue(a, index);
        }

        virtual Real applyToValue(Real, Real) const {
            QL_FAIL("not yet implemented");
        }

        class CurveWrapper {
          public:
            virtual ~CurveWrapper() {}
            virtual Real getValue(const array_type &a,
                                  int i) = 0;
        };

        class ArrayWrapper : public CurveWrapper {
          private:
            array_type value_;
          public:
            ArrayWrapper (const array_type &a)
            : value_(a) {}

            Real getValue(const array_type&, int i) {
                return value_[i];
            }
        };

        class PayoffWrapper : public CurveWrapper {
          private:
            boost::shared_ptr<Payoff> payoff_;
          public:
            PayoffWrapper (const Payoff * p)
                : payoff_(p) {};
            PayoffWrapper (Option::Type type, Real strike)
                : payoff_(new PlainVanillaPayoff(type, strike)) {};
            Real getValue(const array_type &a,
                          int i) {
                return (*payoff_)(a[i]);
            }
        };
    };


    //! %null step condition
    /*! \ingroup findiff */
    template <class array_type>
    class NullCondition : public StepCondition<array_type> {
      public:
        void applyTo(array_type&, Time) const {}
    };

}


#endif
