/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

#include <ql/grid.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class DiscretizedAsset;

    //! Numerical method (Tree, Finite Differences) base class
    class NumericalMethod {
      public:
        NumericalMethod(const TimeGrid& timeGrid) : t_(timeGrid) {}
        virtual ~NumericalMethod() {}

        const TimeGrid& timeGrid() const { return t_; }

        virtual void initialize(const Handle<DiscretizedAsset>& derivative,
                                Time time) const = 0;

        virtual void rollback(const Handle<DiscretizedAsset>& derivative,
                              Time to) const = 0;
      protected:
        TimeGrid t_;
    };

    //! Discretized asset class used by numerical methods
    class DiscretizedAsset {
      public:
        DiscretizedAsset(const Handle<NumericalMethod>& method)
        : method_(method) {}
        virtual ~DiscretizedAsset() {}

        virtual void reset(Size size) = 0;

        Time time() const { return time_; }
        void setTime(Time t) { time_ = t; }

        double value(Size i) const { return values_[i]; }
        Array& values() { return values_; }
        void setValues(const Array& values) { values_ = values; }

        const Handle<NumericalMethod>& method() const { return method_; }


        virtual void adjustValues() {}
        virtual void addTimes(std::list<Time>& times) const {}
      protected:
        bool isOnTime(Time t) const;

        Time time_;
        Array values_;

      private:
        Handle<NumericalMethod> method_;
    };

    //! Useful discretized discount bond asset
    class DiscretizedDiscountBond : public DiscretizedAsset {
      public:
        DiscretizedDiscountBond(const Handle<NumericalMethod>& method)
        : DiscretizedAsset(method) {}
        void reset(Size size) {
            values_ = Array(size, 1.0);
        }
    };

    // inline methods
    inline bool DiscretizedAsset::isOnTime(Time t) const {
        const TimeGrid& grid = method()->timeGrid();
        Time gridTime = grid[grid.findIndex(t)];
        if (QL_FABS(gridTime - time()) < QL_EPSILON)
            return true;
        else
            return false;
    }

}

#endif
