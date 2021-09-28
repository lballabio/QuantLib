/*
 Copyright (C) 2006, 2009 Ferdinando Ametrano
 Copyright (C) 2006, 2007, 2009 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file swapindex.hpp
    \brief swap-rate indexes
*/

#ifndef quantlib_swapindex_hpp
#define quantlib_swapindex_hpp

#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/cashflows/rateaveraging.hpp>

namespace QuantLib {

    class Schedule;

    class IborIndex;
    class VanillaSwap;

    class OvernightIndex;
    class OvernightIndexedSwap;

    //! base class for swap-rate indexes
    class SwapIndex : public InterestRateIndex {
      public:
        SwapIndex(const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  const Currency& currency,
                  const Calendar& fixingCalendar,
                  const Period& fixedLegTenor,
                  BusinessDayConvention fixedLegConvention,
                  const DayCounter& fixedLegDayCounter,
                  ext::shared_ptr<IborIndex> iborIndex);
        SwapIndex(const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  const Currency& currency,
                  const Calendar& fixingCalendar,
                  const Period& fixedLegTenor,
                  BusinessDayConvention fixedLegConvention,
                  const DayCounter& fixedLegDayCounter,
                  ext::shared_ptr<IborIndex> iborIndex,
                  Handle<YieldTermStructure> discountingTermStructure);
        //! \name InterestRateIndex interface
        //@{
        Date maturityDate(const Date& valueDate) const override;
        //@}
        //! \name Inspectors
        //@{
        Period fixedLegTenor() const { return fixedLegTenor_; }
        BusinessDayConvention fixedLegConvention() const;
        ext::shared_ptr<IborIndex> iborIndex() const { return iborIndex_; }
        Handle<YieldTermStructure> forwardingTermStructure() const;
        Handle<YieldTermStructure> discountingTermStructure() const;
        bool exogenousDiscount() const;
        /*! \warning Relinking the term structure underlying the index will
                     not have effect on the returned swap.
        */
        ext::shared_ptr<VanillaSwap> underlyingSwap(
                                                const Date& fixingDate) const;
        //@}
        //! \name Other methods
        //@{
        //! returns a copy of itself linked to a different forwarding curve
        virtual ext::shared_ptr<SwapIndex> clone(
                        const Handle<YieldTermStructure>& forwarding) const;
        //! returns a copy of itself linked to different curves
        virtual ext::shared_ptr<SwapIndex> clone(
                        const Handle<YieldTermStructure>& forwarding,
                        const Handle<YieldTermStructure>& discounting) const;
        //! returns a copy of itself with different tenor
        virtual ext::shared_ptr<SwapIndex> clone(
                        const Period& tenor) const;
        // @}
      protected:
        Rate forecastFixing(const Date& fixingDate) const override;
        Period tenor_;
        ext::shared_ptr<IborIndex> iborIndex_;
        Period fixedLegTenor_;
        BusinessDayConvention fixedLegConvention_;
        bool exogenousDiscount_;
        Handle<YieldTermStructure> discount_;
        // cache data to avoid swap recreation when the same fixing date
        // is used multiple time to forecast changing fixing
        mutable ext::shared_ptr<VanillaSwap> lastSwap_;
        mutable Date lastFixingDate_;
    };


    //! base class for overnight indexed swap indexes
    class OvernightIndexedSwapIndex : public SwapIndex {
      public:
        OvernightIndexedSwapIndex(
                  const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  const Currency& currency,
                  const ext::shared_ptr<OvernightIndex>& overnightIndex,
                  bool telescopicValueDates = false,
                  RateAveraging::Type averagingMethod = RateAveraging::Compound);
        //! \name Inspectors
        //@{
        ext::shared_ptr<OvernightIndex> overnightIndex() const;
        /*! \warning Relinking the term structure underlying the index will
                     not have effect on the returned swap.
        */
        ext::shared_ptr<OvernightIndexedSwap> underlyingSwap(
                                                const Date& fixingDate) const;
        //@}
      protected:
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        bool telescopicValueDates_;
        RateAveraging::Type averagingMethod_;
        // cache data to avoid swap recreation when the same fixing date
        // is used multiple time to forecast changing fixing
        mutable ext::shared_ptr<OvernightIndexedSwap> lastSwap_;
        mutable Date lastFixingDate_;
    };

    // inline definitions

    inline BusinessDayConvention SwapIndex::fixedLegConvention() const {
        return fixedLegConvention_;
    }

    inline bool SwapIndex::exogenousDiscount() const {
        return exogenousDiscount_;
    }

    inline ext::shared_ptr<OvernightIndex>
    OvernightIndexedSwapIndex::overnightIndex() const {
        return overnightIndex_;
    }

}

#endif
