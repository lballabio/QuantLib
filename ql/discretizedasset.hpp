
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file discretizedasset.hpp
    \brief Discretized asset classes
*/

#ifndef quantlib_discretized_asset_h
#define quantlib_discretized_asset_h

#include <ql/numericalmethod.hpp>
#include <ql/Math/comparison.hpp>

namespace QuantLib {

    //! Discretized asset class used by numerical methods
    class DiscretizedAsset {
      public:
        DiscretizedAsset(const Handle<NumericalMethod>& method)
        : method_(method) {}
        virtual ~DiscretizedAsset() {}

        virtual void reset(Size size) = 0;

        Time time() const { return time_; }
        Time& time() { return time_; }

        const Array& values() const { return values_; }
        Array& values() { return values_; }

        const Handle<NumericalMethod>& method() const { return method_; }

        /*! This method will be invoked after rollback and before any
            other asset (i.e., an option on this one) has any chance to 
            look at the values. For instance, payments happening at times 
            already spanned by the rollback will be added here.
        */
        virtual void preAdjustValues() {}
        /*! This method will be invoked after rollback and after any
            other asset had their chance to look at the values. For 
            instance, payments happening at the present time (and therefore 
            not included in an option to be exercised at this time) will be 
            added here.
        */
        virtual void postAdjustValues() {}

        void adjustValues() {
            preAdjustValues();
            postAdjustValues();
        }

        virtual void addTimesTo(std::list<Time>& times) const {}
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

    //! Discretized option on another asset
    /*! \pre The underlying asset must be initialized */
    class DiscretizedOption : public DiscretizedAsset {
      public:
        DiscretizedOption(const Handle<DiscretizedAsset>& underlying,
                          Exercise::Type exerciseType,
                          const std::vector<Time>& exerciseTimes)
        : DiscretizedAsset(underlying->method()),
          underlying_(underlying), exerciseType_(exerciseType),
          exerciseTimes_(exerciseTimes) {}
        void reset(Size size);
        void postAdjustValues();
        void addTimesTo(std::list<Time>& times) const;
      protected:
        void applyExerciseCondition();
        Handle<DiscretizedAsset> underlying_;
        Exercise::Type exerciseType_;
        std::vector<Time> exerciseTimes_;
    };



    // inline definitions

    inline bool DiscretizedAsset::isOnTime(Time t) const {
        const TimeGrid& grid = method()->timeGrid();
        Time gridTime = grid[grid.findIndex(t)];
        return close_enough(gridTime,time());
    }


    inline void DiscretizedOption::reset(Size size) {
        values_ = Array(size, 0.0);
        adjustValues();
    }

    inline void DiscretizedOption::addTimesTo(std::list<Time>& times) const {
        underlying_->addTimesTo(times);
        // discard negative times...
        std::vector<Time>::const_iterator i =
            std::find_if(exerciseTimes_.begin(),exerciseTimes_.end(),
                         std::bind2nd(std::greater_equal<Time>(),0.0));
        // and add the positive ones
        std::copy(i,exerciseTimes_.end(),std::back_inserter(times));
    }


    inline void DiscretizedOption::applyExerciseCondition() {
        for (Size i=0; i<values_.size(); i++)
            values_[i] = QL_MAX(underlying_->values()[i], values_[i]);
    }


}


#endif
