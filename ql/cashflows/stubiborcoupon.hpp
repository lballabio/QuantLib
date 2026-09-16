/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

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

/*! \file stubiborcoupon.hpp
    \brief Ibor coupon for a broken period, fixing on selected indices
*/

#ifndef quantlib_stub_ibor_coupon_hpp
#define quantlib_stub_ibor_coupon_hpp

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    //! Index selection for irregular Ibor coupons
    /*! A default constructor is empty, meaning that broken
        periods fix on the leg's own index as usual.  A non-empty one supplies
        the candidate indices used instead.

        The indices are supplied as fully constructed objects so that each one
        can use its own conventions, fixing history and forwarding curve.

        For ClosestIndex, the index whose maturity is closest to the coupon end
        date is used. For Interpolated, the two index maturities bracketing
        the coupon end date are used for linear interpolation in calendar days.

        The weights depend on the accrual dates.
    */
    class StubIndexSelection {
      public:
        //! Convention used to select an index for an irregular Ibor coupon
        enum Convention {
            ClosestIndex,
            Interpolated
        };

        //! empty selection: broken periods fix on the leg's own index
        StubIndexSelection() = default;
        StubIndexSelection(Convention convention,
                           std::vector<ext::shared_ptr<IborIndex> > indices);

        bool empty() const { return indices_.empty(); }
        Convention convention() const { return convention_; }
        const std::vector<ext::shared_ptr<IborIndex> >& indices() const { return indices_; }

      private:
        Convention convention_ = ClosestIndex;
        std::vector<ext::shared_ptr<IborIndex> > indices_;
    };


    //! Stub Ibor coupon fixing on an explicitly supplied set of indices
    /*! This coupon is intended for broken periods.  It fixes on a
        combination of the indices selected by the given selection.
    */
    class StubIborCoupon : public IborCoupon {
      public:
        //! %Index fixing as a fixed-weight linear combination of Ibor indices
        /*! Like SwapSpreadIndex, this index has no fixings and no forwarding
            curve of its own (it delegates both to its components).

            The components must share their conventions (fixing days, fixing
            calendar, day counter, currency). Only their tenors differ, and
            the maturity of the combination is that of the longest component.
        */
        class WeightedIndex : public IborIndex {
          public:
            //! a component index and the weight it carries in the fixing
            using Component = std::pair<ext::shared_ptr<IborIndex>, Real>;

            explicit WeightedIndex(std::vector<Component> components);

            //! \name InterestRateIndex interface
            //@{
            //! the maturity of the longest component
            Date maturityDate(const Date& valueDate) const override;
            Rate forecastFixing(const Date& fixingDate) const override;
            Rate pastFixing(const Date& fixingDate) const override;
            bool allowsNativeFixings() override { return false; }
            //@}

            //! \name IborIndex interface
            //@{
            /*! \warning a combination of indices cannot be relinked to a single
                         curve (exception thrown). Relink the components instead.
            */
            ext::shared_ptr<IborIndex> clone(const Handle<YieldTermStructure>&) const override;
            //@}

            //! \name Inspectors
            //@{
            const std::vector<Component>& components() const { return components_; }
            //@}

          private:
            static const ext::shared_ptr<IborIndex>&
            firstComponent(const std::vector<Component>& components);
            static Period longestTenor(const std::vector<Component>& components);

            std::vector<Component> components_;
        };

        StubIborCoupon(const Date& paymentDate,
                       Real nominal,
                       const Date& startDate,
                       const Date& endDate,
                       Natural fixingDays,
                       const StubIndexSelection& stubIndexSelection,
                       Real gearing = 1.0,
                       Spread spread = 0.0,
                       const Date& refPeriodStart = Date(),
                       const Date& refPeriodEnd = Date(),
                       const DayCounter& dayCounter = DayCounter(),
                       bool isInArrears = false,
                       const Date& exCouponDate = Date(),
                       BusinessDayConvention fixingConvention = Preceding);

        const StubIndexSelection& stubIndexSelection() const {
            return stubIndexSelection_;
        }

        Rate indexFixing() const override;
        bool hasFixed() const override;
        void accept(AcyclicVisitor&) override;

      private:
        StubIborCoupon(const Date& paymentDate,
                       Real nominal,
                       const Date& startDate,
                       const Date& endDate,
                       Natural fixingDays,
                       ext::shared_ptr<WeightedIndex> weightedIndex,
                       StubIndexSelection stubIndexSelection,
                       Real gearing,
                       Spread spread,
                       const Date& refPeriodStart,
                       const Date& refPeriodEnd,
                       const DayCounter& dayCounter,
                       bool isInArrears,
                       const Date& exCouponDate,
                       BusinessDayConvention fixingConvention);

        StubIndexSelection stubIndexSelection_;
        ext::shared_ptr<WeightedIndex> weightedIndex_;
    };

}

#endif
