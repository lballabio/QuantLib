/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
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
#include <ql/cashflow.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/types.hpp>
#include <ql/handle.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <vector>

namespace QuantLib {

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
      public:
        Bond(Natural settlementDays,
             const Calendar& calendar,
             Real faceAmount,
             const Date& maturityDate,
             const Date& issueDate = Null<Date>(),
             const Leg& leg = std::vector<boost::shared_ptr<CashFlow> >());
        class arguments;
        class results;
        class engine;
        //! \name Inspectors
        //@{
        Natural settlementDays() const;
        const Calendar& calendar() const;
        Real faceAmount() const;
        /*! \warning the returned vector includes the redemption as
                     the last cash flow.
        */
        const Leg& cashflows() const;
        const boost::shared_ptr<CashFlow>& redemption() const;

        Date maturityDate() const;
        Date issueDate() const;

        Date settlementDate(const Date& d = Date()) const;
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
        Rate yield(const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100) const;

        //! clean price given a yield and settlement date
        /*! The default bond settlement is used if no date is given. */
        Real cleanPrice(Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        Date settlementDate = Date()) const;
        //! dirty price given a yield and settlement date
        /*! The default bond settlement is used if no date is given. */
        Real dirtyPrice(Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        Date settlementDate = Date()) const;
        //! yield given a (clean) price and settlement date
        /*! The default bond settlement is used if no date is given. */
        Rate yield(Real cleanPrice,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
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
                                   const DayCounter& dc,
                                   Compounding comp,
                                   Frequency freq,
                                   Date settlementDate = Date()) const;
        //! dirty price given Z-spread
        /*! Z-spread compounding, frequency, daycount are taken into account
            The default bond settlement is used if no date is given.
            For details on Z-spread refer to:
            "Credit Spreads Explained", Lehman Brothers European Fixed
            Income Research - March 2004, D. O'Kane*/
        Real dirtyPriceFromZSpread(Spread zSpread,
                                   const DayCounter& dc,
                                   Compounding comp,
                                   Frequency freq,
                                   Date settlementDate = Date()) const;

        //! accrued amount at a given date
        /*! The default bond settlement is used if no date is given. */
        virtual Real accruedAmount(Date d = Date()) const;
        bool isExpired() const;
        //@}

        /*! Expected next coupon: depending on (the bond and) the given date
            the coupon can be historic, deterministic or expected in a
            stochastic sense. When the bond settlement date is used the coupon
            is the already-fixed not-yet-paid one.

            The current bond settlement is used if no date is given.
        */
        virtual Rate nextCoupon(Date d = Date()) const;

        //! Previous coupon already paid at a given date
        /*! Expected previous coupon: depending on (the bond and) the given
            date the coupon can be historic, deterministic or expected in a
            stochastic sense. When the bond settlement date is used the coupon
            is the last paid one.

            The current bond settlement is used if no date is given.
        */
        Rate previousCoupon(Date d = Date()) const;
        //@}

      protected:
        void setupArguments(PricingEngine::arguments*) const;

        Natural settlementDays_;
        Calendar calendar_;
        Real faceAmount_;
        Leg cashflows_;

        Date maturityDate_, issueDate_;
    };

    class Bond::arguments : public PricingEngine::arguments {
      public:
        Date settlementDate;
        Leg cashflows;
        Calendar calendar;
        void validate() const;
    };

    class Bond::results : public Instrument::results {};

    class Bond::engine : public GenericEngine<Bond::arguments,
                                              Bond::results> {};


    // inline definitions

    inline Natural Bond::settlementDays() const {
        return settlementDays_;
    }

    inline const Calendar& Bond::calendar() const {
        return calendar_;
    }

    inline Real Bond::faceAmount() const {
        return faceAmount_;
    }

    inline const Leg& Bond::cashflows() const {
        return cashflows_;
    }

    inline const boost::shared_ptr<CashFlow>& Bond::redemption() const {
        return cashflows_.back();
    }

    inline Date Bond::maturityDate() const {
        if (maturityDate_!=Null<Date>())
            return maturityDate_;
        else
            return cashflows_.back()->date();
    }

    inline Date Bond::issueDate() const {
        return issueDate_;
    }

}

#endif
