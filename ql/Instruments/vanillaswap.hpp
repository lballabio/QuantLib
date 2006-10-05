/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file vanillaswap.hpp
    \brief Simple fixed-rate vs Libor swap
*/

#ifndef quantlib_vanilla_swap_hpp
#define quantlib_vanilla_swap_hpp

#include <ql/Instruments/swap.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/schedule.hpp>

namespace QuantLib {

    //! Plain-vanilla swap
    /*! \ingroup instruments

        \test
        - the correctness of the returned value is tested by checking
          that the price of a swap paying the fair fixed rate is null.
        - the correctness of the returned value is tested by checking
          that the price of a swap receiving the fair floating-rate
          spread is null.
        - the correctness of the returned value is tested by checking
          that the price of a swap decreases with the paid fixed rate.
        - the correctness of the returned value is tested by checking
          that the price of a swap increases with the received
          floating-rate spread.
        - the correctness of the returned value is tested by checking
          it against a known good value.
    */
    class VanillaSwap : public Swap {
      public:
        class arguments;
        class results;
        #ifndef QL_DISABLE_DEPRECATED
        //! \deprecated use the other VanillaSwap constructor or Swap instead
        VanillaSwap(bool payFixedRate,
                    Real nominal,
                    const Schedule& fixedSchedule,
                    Rate fixedRate,
                    const DayCounter& fixedDayCount,
                    const Schedule& floatSchedule,
                    const boost::shared_ptr<Xibor>& index,
                    Integer indexFixingDays,
                    Spread spread,
                    const DayCounter& floatingDayCount,
                    const Handle<YieldTermStructure>& termStructure);
        #endif
        VanillaSwap(bool payFixedRate,
                    Real nominal,
                    const Schedule& fixedSchedule,
                    Rate fixedRate,
                    const DayCounter& fixedDayCount,
                    const Schedule& floatSchedule,
                    const boost::shared_ptr<Xibor>& index,
                    Spread spread,
                    const DayCounter& floatingDayCount,
                    const Handle<YieldTermStructure>& termStructure);
        // results
        Rate fairRate() const;
        Spread fairSpread() const;
        Real fixedLegBPS() const;
        Real floatingLegBPS() const;
        // inspectors
        Rate fixedRate() const;
        Spread spread() const;
        Real nominal() const;
        bool payFixedRate() const;
        const std::vector<boost::shared_ptr<CashFlow> >& fixedLeg() const {
            return legs_[0];
        }
        const std::vector<boost::shared_ptr<CashFlow> >& floatingLeg() const {
            return legs_[1];
        }
        // other
        void setupArguments(Arguments* args) const;
        void fetchResults(const Results*) const;
      private:
        void setupExpired() const;
        void performCalculations() const;
        bool payFixedRate_;
        Rate fixedRate_;
        Spread spread_;
        Real nominal_;
        // results
        mutable Rate fairRate_;
        mutable Spread fairSpread_;
    };


    //! %Arguments for simple swap calculation
    class VanillaSwap::arguments : public virtual Arguments {
      public:
        arguments() : payFixed(false),
                      nominal(Null<Real>()),
                      currentFloatingCoupon(Null<Real>()) {}
        bool payFixed;
        Real nominal;
        std::vector<Time> fixedResetTimes;
        std::vector<Time> fixedPayTimes;
        std::vector<Real> fixedCoupons;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Time> floatingResetTimes;
        std::vector<Time> floatingFixingTimes;
        std::vector<Time> floatingPayTimes;
        std::vector<Spread> floatingSpreads;
        Real currentFloatingCoupon;
        void validate() const;
    };

    //! %Results from simple swap calculation
    class VanillaSwap::results : public Value {
      public:
        Real fixedLegBPS;
        Real floatingLegBPS;
        Rate fairRate;
        Spread fairSpread;
        results() { reset(); }
        void reset() {
            fixedLegBPS = floatingLegBPS = Null<Real>();
            fairRate = Null<Rate>();
            fairSpread = Null<Spread>();
        }
    };

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate standard market swap.
    */
    class MakeVanillaSwap {
      public:
        MakeVanillaSwap(const Date& effectiveDate,
                        const Period& swapTenor, 
                        const Calendar& cal,
                        Rate fixedRate,
                        const boost::shared_ptr<Xibor>& index,
                        const boost::shared_ptr<YieldTermStructure>& termStructure);

        MakeVanillaSwap& receiveFixed(bool flag = true);
        MakeVanillaSwap& withNominal(Real n);
        MakeVanillaSwap& withFixedLegTenor(const Period& t);
        MakeVanillaSwap& withFixedLegCalendar(const Calendar& cal);
        MakeVanillaSwap& withFixedLegConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFixedLegTerminationDateConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFixedLegForward(bool flag = true);
        MakeVanillaSwap& withFixedLegNotEndOfMonth(bool flag = true);
        MakeVanillaSwap& withFixedLegFirstDate(const Date& d);
        MakeVanillaSwap& withFixedLegNextToLastDate(const Date& d);
        MakeVanillaSwap& withFixedLegDayCount(const DayCounter& dc);

        MakeVanillaSwap& withFloatingLegTenor(const Period& t);
        MakeVanillaSwap& withFloatingLegCalendar(const Calendar& cal);
        MakeVanillaSwap& withFloatingLegConvention(const BusinessDayConvention bdc);
        MakeVanillaSwap& withFloatingLegTerminationDateConvention(BusinessDayConvention bdc);
        MakeVanillaSwap& withFloatingLegForward(bool flag = true);
        MakeVanillaSwap& withFloatingLegNotEndOfMonth(bool flag = true);
        MakeVanillaSwap& withFloatingLegFirstDate(const Date& d);
        MakeVanillaSwap& withFloatingLegNextToLastDate(const Date& d);
        MakeVanillaSwap& withFloatingLegDayCount(const DayCounter& dc);
        MakeVanillaSwap& withFloatingLegSpread(Spread sp);
        
        operator VanillaSwap() const;
        operator boost::shared_ptr<VanillaSwap>() const ;

      private:
        bool payFixed_;
        Real nominal_;
        Date effectiveDate_;
        Period swapTenor_, fixedTenor_, floatTenor_; 
        Calendar fixedCalendar_, floatCalendar_;
        BusinessDayConvention fixedConvention_, fixedTerminationDateConvention_;
        BusinessDayConvention floatConvention_, floatTerminationDateConvention_;
        bool fixedBackward_, floatBackward_;
        bool fixedEndOfMonth_, floatEndOfMonth_;
        Date fixedFirstDate_, fixedNextToLastDate_;
        Date floatFirstDate_, floatNextToLastDate_;
        Rate fixedRate_;
        Spread floatSpread_;
        DayCounter fixedDayCount_, floatDayCount_;
        boost::shared_ptr<Xibor> index_;
        Handle<YieldTermStructure> termStructure_;
        Date terminationDate_;
    };

    // inline definitions

    inline Rate VanillaSwap::fixedRate() const {
        return fixedRate_;
    }

    inline Spread VanillaSwap::spread() const {
        return spread_;
    }

    inline Real VanillaSwap::nominal() const {
        return nominal_;
    }

    inline bool VanillaSwap::payFixedRate() const {
        return payFixedRate_;
    }





    inline MakeVanillaSwap& MakeVanillaSwap::receiveFixed(bool flag) {
        payFixed_ = !flag;
        return *this;
    }

    inline MakeVanillaSwap& MakeVanillaSwap::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegTenor(const Period& t) {
        fixedTenor_ = t;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegCalendar(const Calendar& cal) {
        fixedCalendar_ = cal;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegConvention(BusinessDayConvention bdc) {
        fixedConvention_ = bdc;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegTerminationDateConvention(BusinessDayConvention bdc) {
        fixedTerminationDateConvention_ = bdc;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegForward(bool flag) {
        fixedBackward_ = !flag;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegNotEndOfMonth(bool flag) {
        fixedEndOfMonth_ = !flag;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegFirstDate(const Date& d) {
        fixedFirstDate_ = d;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegNextToLastDate(const Date& d) {
        fixedNextToLastDate_ = d;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegTenor(const Period& t) {
        floatTenor_ = t;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegCalendar(const Calendar& cal) {
        floatCalendar_ = cal;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegConvention(BusinessDayConvention bdc) {
        floatConvention_ = bdc;
        return *this;
    }    

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
        floatTerminationDateConvention_ = bdc;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegForward(bool flag) {
        floatBackward_ = !flag;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegNotEndOfMonth(bool flag) {
        floatEndOfMonth_ = !flag;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegFirstDate(const Date& d) {
        floatFirstDate_ = d;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegNextToLastDate(const Date& d) {
        floatNextToLastDate_ = d;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

    inline MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegSpread(Spread sp) {
        floatSpread_ = sp;
        return *this;
    }

}

#endif
