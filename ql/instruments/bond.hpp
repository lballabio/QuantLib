/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl

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

/*! \file bond.hpp
    \brief concrete bond class
*/

#ifndef quantlib_bond_hpp
#define quantlib_bond_hpp

#include <ql/instrument.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/interestrate.hpp>
#include <ql/types.hpp>
#include <ql/handle.hpp>
#include <vector>

namespace QuantLib {
     class YieldTermStructure;
     class CashFlow;
     typedef std::vector<boost::shared_ptr<CashFlow> > Leg;
    //! Base bond class
    /*! Derived classes must fill the unitialized data members.

        \warning Most methods assume that the cashflows are stored
                 sorted by date, the redemption being the last one.

        \ingroup instruments

        \test
        - price/yield calculations are cross-checked for consistency.
        - price/yield calculations are checked against known good
          values.
    */
    class Bond : public Instrument {
      protected:
        Bond(Natural settlementDays,
             Real faceAmount,
             const Calendar& calendar,
             const DayCounter& paymentDayCounter,
             BusinessDayConvention paymentConvention,
             const Handle<YieldTermStructure>& discountCurve
                                              = Handle<YieldTermStructure>());
      public:
        class arguments;
        class results;
        class engine;
        //! \name Inspectors
        //@{
        Date settlementDate(const Date& d = Date()) const;
        Date issueDate() const;
        Date maturityDate() const;
        Date interestAccrualDate() const;
        /*! \warning the returned vector includes the redemption as
                     the last cash flow.
        */
        const Leg& cashflows() const;
        const boost::shared_ptr<CashFlow>& redemption() const;
        const Calendar& calendar() const;
        BusinessDayConvention paymentConvention() const;
        Real faceAmount() const { return faceAmount_;}
        const DayCounter& dayCounter() const;
        Frequency frequency() const;
        Handle<YieldTermStructure> discountCurve() const;
        //@}
        //! \name Calculations
        //@{
        //! theoretical clean price
        /*! The default bond settlement is used for calculation.

            \warning the theoretical price calculated from a flat term
                     structure might differ slightly from the price
                     calculated from the corresponding yield by means
                     of the other overload of this function. If the
                     price from a constant yield is desired, it is
                     advisable to use such other overload.
        */
        Real cleanPrice() const;
        //! theoretical dirty price
        /*! The default bond settlement is used for calculation.

            \warning the theoretical price calculated from a flat term
                     structure might differ slightly from the price
                     calculated from the corresponding yield by means
                     of the other overload of this function. If the
                     price from a constant yield is desired, it is
                     advisable to use such other overload.
        */
        Real dirtyPrice() const;
        //! theoretical bond yield
        /*! The default bond settlement and theoretical price are used
            for calculation.
        */
        Rate yield(Compounding compounding,
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100) const;

        //! clean price given a yield and settlement date
        /*! The default bond settlement is used if no date is given. */
        Real cleanPrice(Rate yield,
                        Compounding compounding,
                        Date settlementDate = Date()) const;
        //! dirty price given a yield and settlement date
        /*! The default bond settlement is used if no date is given. */
        Real dirtyPrice(Rate yield,
                        Compounding compounding,
                        Date settlementDate = Date()) const;
        //! yield given a (clean) price and settlement date
        /*! The default bond settlement is used if no date is given. */
        Rate yield(Real cleanPrice,
                   Compounding compounding,
                   Date settlementDate = Date(),
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100) const;

        //! clean price given Z-spread
        /*! Z-spread compounding, frequency, daycount are taken into account
            The default bond settlement is used if no date is given.
            For details on Z-spread refer to:
            "Credit Spreads Explained", Lehman Brothers European Fixed
            Income Research - March 2004, D. O'Kane*/
        Real cleanPriceFromZSpread(Spread zSpread,
                                   Compounding compounding,
                                   Frequency frequency,
                                   DayCounter paymentDayCounter,
                                   Date settlementDate = Date()) const;
        //! dirty price given Z-spread
        /*! Z-spread compounding, frequency, daycount are taken into account
            The default bond settlement is used if no date is given.
            For details on Z-spread refer to:
            "Credit Spreads Explained", Lehman Brothers European Fixed
            Income Research - March 2004, D. O'Kane*/
        Real dirtyPriceFromZSpread(Spread zSpread,
                                   Compounding compounding,
                                   Frequency frequency,
                                   DayCounter paymentDayCounter,
                                   Date settlementDate = Date()) const;

        //! accrued amount at a given date
        /*! The default bond settlement is used if no date is given. */
        virtual Real accruedAmount(Date d = Date()) const;
        bool isExpired() const;
        //@}

        //! current coupon at a given date
        /*! The default bond settlement is used if no date is given. */
        Real currentCoupon(Date d = Date()) const;

        //! previous coupon already paid at a given date
        /*! The default bond settlement is used if no date is given. */
        Real previousCoupon(Date d = Date()) const;
        //@}

      protected:
        void performCalculations() const;
        void setupArguments(PricingEngine::arguments*) const;
        Natural settlementDays_;
        Real faceAmount_;
        Calendar calendar_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentConvention_;
        Handle<YieldTermStructure> discountCurve_;

        Date issueDate_, datedDate_, maturityDate_;
        Frequency frequency_;
        Leg cashflows_;
    };

    class Bond::arguments : public PricingEngine::arguments {
      public:
        Date settlementDate;
        Leg cashflows;
        Calendar calendar;
        BusinessDayConvention paymentConvention;
        DayCounter paymentDayCounter;
        Frequency frequency;
        void validate() const;
    };

    class Bond::results : public Instrument::results {};

    class Bond::engine : public GenericEngine<Bond::arguments,
                                              Bond::results> {};


    // inline definitions

    inline Date Bond::issueDate() const {
        return issueDate_;
    }

    inline Date Bond::maturityDate() const {
        return maturityDate_;
    }

    inline Date Bond::interestAccrualDate() const {
        return datedDate_;
    }

    inline
    const Leg& Bond::cashflows() const {
        return cashflows_;
    }

    inline const boost::shared_ptr<CashFlow>& Bond::redemption() const {
        return cashflows_.back();
    }

    inline const Calendar& Bond::calendar() const {
        return calendar_;
    }

    inline BusinessDayConvention Bond::paymentConvention() const {
        return paymentConvention_;
    }

    inline const DayCounter& Bond::dayCounter() const {
        return paymentDayCounter_;
    }

    inline Frequency Bond::frequency() const {
        return frequency_;
    }

    inline Handle<YieldTermStructure> Bond::discountCurve() const {
        return discountCurve_;
    }

}

#endif
