/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file numericalmethod.hpp
    \brief Numerical method class

    \fullpath
    ql/%numericalmethod.hpp
*/

// $Id$

#ifndef quantlib_numerical_method_h
#define quantlib_numerical_method_h

#include <ql/array.hpp>
#include <ql/handle.hpp>

#include <list>

namespace QuantLib {

    class NumericalDerivative;

    class NumericalMethod {
      public:
        virtual void initialize(
            const Handle<NumericalDerivative>& derivative,
            Time time) const = 0;
        virtual void rollback(
            const Handle<NumericalDerivative>& derivative,
            Time to) const = 0;
    };

    class NumericalDerivative {
      public:
        NumericalDerivative(const Handle<NumericalMethod>& method)
        : method_(method) {}
        virtual ~NumericalDerivative() {}

        Time time() const { return time_; }

        const Array& values() const { return values_; }
        Array& values() { return values_; }

        const Handle<NumericalMethod>& method() const { return method_; }

        virtual void reset(Size size) = 0;
        void setTime(Time t) { time_ = t; }
        void setValues(const Array& values) { values_ = values; }

        virtual void applyCondition() {}
        virtual void addTimes(std::list<Time>& times) const {}
      protected:
        Array values_;
        Time time_;

      private:
        Handle<NumericalMethod> method_;
    };

    class NumericalDiscountBond : public NumericalDerivative {
      public:
        NumericalDiscountBond(const Handle<NumericalMethod>& method)
        : NumericalDerivative(method) {}
        void reset(Size size) {
            values_ = Array(size, 1.0);
        }
    };

}

#endif
