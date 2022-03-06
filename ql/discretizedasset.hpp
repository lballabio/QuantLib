/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl

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

/*! \file discretizedasset.hpp
    \brief Discretized asset classes
*/

#ifndef quantlib_discretized_asset_hpp
#define quantlib_discretized_asset_hpp

#include <ql/exercise.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/functional.hpp>
#include <ql/numericalmethod.hpp>
#include <utility>

namespace QuantLib {

    //! Discretized asset class used by numerical methods
    class DiscretizedAsset {
      public:
        DiscretizedAsset()
        : latestPreAdjustment_(QL_MAX_REAL),
          latestPostAdjustment_(QL_MAX_REAL) {}
        virtual ~DiscretizedAsset() = default;

        //! \name inspectors
        //@{
        Time time() const { return time_; }
        Time& time() { return time_; }

        const Array& values() const { return values_; }
        Array& values() { return values_; }

        const ext::shared_ptr<Lattice>& method() const {
            return method_;
        }
        //@}

        /*! \name High-level interface

            Users of discretized assets should use these methods in
            order to initialize, evolve and take the present value of
            the assets.  They call the corresponding methods in the
            Lattice interface, to which we refer for
            documentation.

            @{
        */
        void initialize(const ext::shared_ptr<Lattice>&,
                        Time t);
        void rollback(Time to);
        void partialRollback(Time to);
        Real presentValue();
        //@}

        /*! \name Low-level interface

            These methods (that developers should override when
            deriving from DiscretizedAsset) are to be used by
            numerical methods and not directly by users, with the
            exception of adjustValues(), preAdjustValues() and
            postAdjustValues() that can be used together with
            partialRollback().

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

        /*! This method returns the times at which the numerical
            method should stop while rolling back the asset. Typical
            examples include payment times, exercise times and such.

            \note The returned values are not guaranteed to be sorted.
        */
        virtual std::vector<Time> mandatoryTimes() const = 0;
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
        ext::shared_ptr<Lattice> method_;
    };


    //! Useful discretized discount bond asset
    class DiscretizedDiscountBond : public DiscretizedAsset {
      public:
        DiscretizedDiscountBond() = default;
        void reset(Size size) override { values_ = Array(size, 1.0); }
        std::vector<Time> mandatoryTimes() const override { return std::vector<Time>(); }
    };


    //! Discretized option on a given asset
    /*! \warning it is advised that derived classes take care of
                 creating and initializing themselves an instance of
                 the underlying.
    */
    class DiscretizedOption : public DiscretizedAsset {
      public:
        DiscretizedOption(ext::shared_ptr<DiscretizedAsset> underlying,
                          Exercise::Type exerciseType,
                          std::vector<Time> exerciseTimes)
        : underlying_(std::move(underlying)), exerciseType_(exerciseType),
          exerciseTimes_(std::move(exerciseTimes)) {}
        void reset(Size size) override;
        std::vector<Time> mandatoryTimes() const override;

      protected:
        void postAdjustValuesImpl() override;
        void applyExerciseCondition();
        ext::shared_ptr<DiscretizedAsset> underlying_;
        Exercise::Type exerciseType_;
        std::vector<Time> exerciseTimes_;
    };



    // inline definitions

    inline void DiscretizedAsset::initialize(
                             const ext::shared_ptr<Lattice>& method,
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
        return close_enough(grid[grid.index(t)],time());
    }


    inline void DiscretizedOption::reset(Size size) {
        QL_REQUIRE(method() == underlying_->method(),
                   "option and underlying were initialized on "
                   "different methods");
        values_ = Array(size, 0.0);
        adjustValues();
    }

    inline std::vector<Time> DiscretizedOption::mandatoryTimes() const {
        std::vector<Time> times = underlying_->mandatoryTimes();
        // discard negative times...
        auto i = std::find_if(exerciseTimes_.begin(), exerciseTimes_.end(),
                              greater_or_equal_to<Time>(0.0));
        // and add the positive ones
        times.insert(times.end(), i, exerciseTimes_.end());
        return times;
    }

    inline void DiscretizedOption::applyExerciseCondition() {
        for (Size i=0; i<values_.size(); i++)
            values_[i] = std::max(underlying_->values()[i], values_[i]);
    }


}


#endif


#ifndef id_daef5606722ab3fa03a22e83941b084c
#define id_daef5606722ab3fa03a22e83941b084c
inline bool test_daef5606722ab3fa03a22e83941b084c(const int* i) {
    return i != nullptr;
}
#endif
