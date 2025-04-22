/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file iborindex.hpp
    \brief base class for Inter-Bank-Offered-Rate indexes
*/

#ifndef quantlib_ibor_index_hpp
#define quantlib_ibor_index_hpp

#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/patterns/lazyobject.hpp>

namespace QuantLib {

    //! base class for Inter-Bank-Offered-Rate indexes (e.g. %Libor, etc.)
    class IborIndex : public InterestRateIndex {
      public:
        IborIndex(const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  const Currency& currency,
                  const Calendar& fixingCalendar,
                  BusinessDayConvention convention,
                  bool endOfMonth,
                  const DayCounter& dayCounter,
                  Handle<YieldTermStructure> h = {});
        //! \name InterestRateIndex interface
        //@{
        Date maturityDate(const Date& valueDate) const override;
        Rate forecastFixing(const Date& fixingDate) const override;
        // @}
        //! \name Inspectors
        //@{
        BusinessDayConvention businessDayConvention() const;
        bool endOfMonth() const { return endOfMonth_; }
        //! the curve used to forecast fixings
        Handle<YieldTermStructure> forwardingTermStructure() const;
        //@}
        //! \name Other methods
        //@{
        //! returns a copy of itself linked to a different forwarding curve
        virtual ext::shared_ptr<IborIndex> clone(
                        const Handle<YieldTermStructure>& forwarding) const;
        // @}
      protected:
        BusinessDayConvention convention_;
        Handle<YieldTermStructure> termStructure_;
        bool endOfMonth_;
      private:
        // overload to avoid date/time (re)calculation
        /* This can be called with cached coupon dates (and it does
           give quite a performance boost to coupon calculations) but
           is potentially misleading: by passing the wrong dates, one
           can ask a 6-months index for a 1-year fixing.

           For that reason, we're leaving this method private and
           we're declaring the IborCoupon class (which uses it) as a
           friend.  Should the need arise, we might promote it to
           public, but before doing that I'd think hard whether we
           have any other way to get the same results.
        */
        Rate forecastFixing(const Date& valueDate,
                            const Date& endDate,
                            Time t) const;
        friend class IborCoupon;
    };


    class OvernightIndex : public IborIndex,
                           virtual public LazyObject {
      public:
        OvernightIndex(const std::string& familyName,
                       Natural settlementDays,
                       const Currency& currency,
                       const Calendar& fixingCalendar,
                       const DayCounter& dayCounter,
                       const Handle<YieldTermStructure>& h = {});
        //! returns a copy of itself linked to a different forwarding curve
        ext::shared_ptr<IborIndex> clone(const Handle<YieldTermStructure>& h) const override;
        void update() override;
        void addFixing(const Date& fixingDate, Real fixing, bool forceOverwrite = false) override;
        void addFixings(const TimeSeries<Real>& t, bool forceOverwrite = false) override;
        Rate compoundedFixings(const Date& fromFixingDate, const Date& toFixingDate);
      protected:
          void performCalculations() const override;
      private:
        mutable TimeSeries<Real> compoundIndex_;
    };


    // inline

    inline BusinessDayConvention IborIndex::businessDayConvention() const {
        return convention_;
    }

    inline Handle<YieldTermStructure>
    IborIndex::forwardingTermStructure() const {
        return termStructure_;
    }

    inline Rate IborIndex::forecastFixing(const Date& d1,
                                          const Date& d2,
                                          Time t) const {
        QL_REQUIRE(!termStructure_.empty(),
                   "null term structure set to this instance of " << name());
        DiscountFactor disc1 = termStructure_->discount(d1);
        DiscountFactor disc2 = termStructure_->discount(d2);
        return (disc1/disc2 - 1.0) / t;
    }

}

#endif
