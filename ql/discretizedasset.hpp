
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

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
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the constructor with no arguments */
        DiscretizedAsset(const boost::shared_ptr<NumericalMethod>& method)
        : method_(method), latestPreAdjustment_(QL_MAX_REAL),
          latestPostAdjustment_(QL_MAX_REAL) {}
        #endif

        DiscretizedAsset()
        : latestPreAdjustment_(QL_MAX_REAL),
          latestPostAdjustment_(QL_MAX_REAL) {}
        virtual ~DiscretizedAsset() {}

        //! \name inspectors
        //@{
        Time time() const { return time_; }
        Time& time() { return time_; }

        const Array& values() const { return values_; }
        Array& values() { return values_; }

        const boost::shared_ptr<NumericalMethod>& method() const {
            return method_;
        }
        //@}

        /*! \name High-level interface

            Users of discretized assets should use these methods for
            initializing, evolving and take the present value of the
            assets.

            @{
        */
        void initialize(const boost::shared_ptr<NumericalMethod>&,
                        Time t);
        void rollback(Time to);
        void partialRollback(Time to);
        Real presentValue();
        //@}

        /*! \name Low-level interface

            These methods (that developers should override when
            deriving from DiscretizedAsset) are to be used by
            numerical methods and not directly by users.

            @{
        */

        /*! This method should initialize the asset values to an Array
            of the given size and with values depending on the
            particular asset.
        */
        virtual void reset(Size size) = 0;

        /*! This method will be invoked after rollback and before any
            other asset (i.e., an option on this one) has any chance to 
            look at the values. For instance, payments happening at times 
            already spanned by the rollback will be added here.

            This method is not virtual; derived classes must override
            the protected preAdjustValuesImpl() method instead.
        */
        void preAdjustValues();

        /*! This method will be invoked after rollback and after any
            other asset had their chance to look at the values. For 
            instance, payments happening at the present time (and therefore 
            not included in an option to be exercised at this time) will be 
            added here.

            This method is not virtual; derived classes must override
            the protected postAdjustValuesImpl() method instead.
        */
        void postAdjustValues();

        /*! This method performs both pre- and post-adjustment */
        void adjustValues() {
            preAdjustValues();
            postAdjustValues();
        }

        /*! This method appends to the given list the times at which
            the numerical method should stop while rolling back.
            Typical examples include payment times, exercise times
            and such.
        */
        virtual void addTimesTo(std::list<Time>&) const {}
        //@}
      protected:
        /*! This method checks whether the asset was rolled at the
            given time. */
        bool isOnTime(Time t) const;
        /*! This method performs the actual pre-adjustment */
        virtual void preAdjustValuesImpl() {}
        /*! This method performs the actual post-adjustment */
        virtual void postAdjustValuesImpl() {}

        Time time_;
        Time latestPreAdjustment_, latestPostAdjustment_;
        Array values_;
      private:
        boost::shared_ptr<NumericalMethod> method_;
    };


    //! Useful discretized discount bond asset
    class DiscretizedDiscountBond : public DiscretizedAsset {
      public:
        DiscretizedDiscountBond(
                             const boost::shared_ptr<NumericalMethod>& method)
        : DiscretizedAsset(method) {}
        void reset(Size size) {
            values_ = Array(size, 1.0);
        }
    };


    //! Discretized option on another asset
    /*! \pre The underlying asset must be initialized */
    class DiscretizedOption : public DiscretizedAsset {
      public:
        DiscretizedOption(
                      const boost::shared_ptr<DiscretizedAsset>& underlying,
                      Exercise::Type exerciseType,
                      const std::vector<Time>& exerciseTimes)
        : DiscretizedAsset(underlying->method()),
          underlying_(underlying), exerciseType_(exerciseType),
          exerciseTimes_(exerciseTimes) {}
        void reset(Size size);
        void addTimesTo(std::list<Time>& times) const;
      protected:
        void postAdjustValuesImpl();
        void applyExerciseCondition();
        boost::shared_ptr<DiscretizedAsset> underlying_;
        Exercise::Type exerciseType_;
        std::vector<Time> exerciseTimes_;
    };



    // inline definitions

    inline void DiscretizedAsset::initialize(
                             const boost::shared_ptr<NumericalMethod>& method,
                             Time t) {
        method_ = method;
        method_->initialize(*this, t);
    }

    inline void DiscretizedAsset::rollback(Time to) {
        method_->rollback(*this, to);
    }

    inline void DiscretizedAsset::partialRollback(Time to) {
        method_->partialRollback(*this, to);
    }

    inline Real DiscretizedAsset::presentValue() {
        return method_->presentValue(*this);
    }

    inline void DiscretizedAsset::preAdjustValues() {
        if (!close_enough(time(),latestPreAdjustment_)) {
            preAdjustValuesImpl();
            latestPreAdjustment_ = time();
        }
    }

    inline void DiscretizedAsset::postAdjustValues() {
        if (!close_enough(time(),latestPostAdjustment_)) {
            postAdjustValuesImpl();
            latestPostAdjustment_ = time();
        }
    }

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
