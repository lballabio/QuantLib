/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2008 Simon Ibbotson

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
#include <ql/cashflow.hpp>
#include <ql/compounding.hpp>

#include <vector>

namespace QuantLib {

    class DayCounter;

    //! Base bond class
    /*! Derived classes must fill the uninitialized data members.

        \warning Most methods assume that the cash flows are stored
                 sorted by date, the redemption(s) being after any
                 cash flow at the same date. In particular, if there's
                 one single redemption, it must be the last cash flow,

        \ingroup instruments

        \test
        - price/yield calculations are cross-checked for consistency.
        - price/yield calculations are checked against known good
          values.
    */
    class Bond : public Instrument {
      public:
        //! Bond price information
        class Price {
          public:
            enum Type { Dirty, Clean };
            Price() : amount_(Null<Real>()) {}
            Price(Real amount, Type type) : amount_(amount), type_(type) {}
            Real amount() const {
                QL_REQUIRE(amount_ != Null<Real>(), "no amount given");
                return amount_;
            }
            Type type() const { return type_; }
          private:
            Real amount_;
            Type type_;
        };

        //! constructor for amortizing or non-amortizing bonds.
        /*! Redemptions and maturity are calculated from the coupon
            data, if available.  Therefore, redemptions must not be
            included in the passed cash flows.
        */
        Bond(Natural settlementDays,
             Calendar calendar,
             const Date& issueDate = Date(),
             const Leg& coupons = Leg());

        //! old constructor for non amortizing bonds.
        /*! \warning The last passed cash flow must be the bond
                     redemption. No other cash flow can have a date
                     later than the redemption date.
        */
        Bond(Natural settlementDays,
             Calendar calendar,
             Real faceAmount,
             const Date& maturityDate,
             const Date& issueDate = Date(),
             const Leg& cashflows = Leg());

        class arguments;
        class results;
        class engine;

        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        //@}
        //! \name Observable interface
        //@{
        void deepUpdate() override;
        //@}
        //! \name Inspectors
        //@{
        Natural settlementDays() const;
        const Calendar& calendar() const;

        const std::vector<Real>& notionals() const;
        virtual Real notional(Date d = Date()) const;

        /*! \note returns all the cashflows, including the redemptions. */
        const Leg& cashflows() const;
        /*! returns just the redemption flows (not interest payments) */
        const Leg& redemptions() const;
        /*! returns the redemption, if only one is defined */
        const ext::shared_ptr<CashFlow>& redemption() const;

        Date startDate() const;
        Date maturityDate() const;
        Date issueDate() const;

        bool isTradable(Date d = Date()) const;
        Date settlementDate(Date d = Date()) const;
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

        //! theoretical settlement value
        /*! The default bond settlement date is used for calculation. */
        Real settlementValue() const;

        //! theoretical bond yield
        /*! The default bond settlement and theoretical price are used
            for calculation.
        */
        Rate yield(const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100,
                   Real guess = 0.05,
                   Bond::Price::Type priceType = Bond::Price::Clean) const;

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

        //! settlement value as a function of the clean price
        /*! The default bond settlement date is used for calculation. */
        Real settlementValue(Real cleanPrice) const;

        //! yield given a (clean) price and settlement date
        /*! The default bond settlement is used if no date is given. */
        Rate yield(Real cleanPrice,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Date settlementDate = Date(),
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100,
                   Real guess = 0.05,
                   Bond::Price::Type priceType = Bond::Price::Clean) const;

        //! accrued amount at a given date
        /*! The default bond settlement is used if no date is given. */
        virtual Real accruedAmount(Date d = Date()) const;
        //@}

        /*! Expected next coupon: depending on (the bond and) the given date
            the coupon can be historic, deterministic or expected in a
            stochastic sense. When the bond settlement date is used the coupon
            is the already-fixed not-yet-paid one.

            The current bond settlement is used if no date is given.
        */
        virtual Rate nextCouponRate(Date d = Date()) const;

        //! Previous coupon already paid at a given date
        /*! Expected previous coupon: depending on (the bond and) the given
            date the coupon can be historic, deterministic or expected in a
            stochastic sense. When the bond settlement date is used the coupon
            is the last paid one.

            The current bond settlement is used if no date is given.
        */
        Rate previousCouponRate(Date d = Date()) const;

        Date nextCashFlowDate(Date d = Date()) const;
        Date previousCashFlowDate(Date d = Date()) const;

      protected:
        void setupExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;

        /*! This method can be called by derived classes in order to
            build redemption payments from the existing cash flows.
            It must be called after setting up the cashflows_ vector
            and will fill the notionalSchedule_, notionals_, and
            redemptions_ data members.

            If given, the elements of the redemptions vector will
            multiply the amount of the redemption cash flow.  The
            elements will be taken in base 100, i.e., a redemption
            equal to 100 does not modify the amount.

            \pre The cashflows_ vector must contain at least one
                 coupon and must be sorted by date.
        */
        void addRedemptionsToCashflows(const std::vector<Real>& redemptions
                                                       = std::vector<Real>());

        /*! This method can be called by derived classes in order to
            build a bond with a single redemption payment.  It will
            fill the notionalSchedule_, notionals_, and redemptions_
            data members.
        */
        void setSingleRedemption(Real notional,
                                 Real redemption,
                                 const Date& date);

        /*! This method can be called by derived classes in order to
            build a bond with a single redemption payment.  It will
            fill the notionalSchedule_, notionals_, and redemptions_
            data members.
        */
        void setSingleRedemption(Real notional,
                                 const ext::shared_ptr<CashFlow>& redemption);

        /*! used internally to collect notional information from the
            coupons. It should not be called by derived classes,
            unless they already provide redemption cash flows (in
            which case they must set up the redemptions_ data member
            independently).  It will fill the notionalSchedule_ and
            notionals_ data members.
        */
        void calculateNotionalsFromCashflows();

        Natural settlementDays_;
        Calendar calendar_;
        std::vector<Date> notionalSchedule_;
        std::vector<Real> notionals_;
        Leg cashflows_; // all cashflows
        Leg redemptions_; // the redemptions

        Date maturityDate_, issueDate_;
        mutable Real settlementValue_;
    };

    class Bond::arguments : public PricingEngine::arguments {
      public:
        Date settlementDate;
        Leg cashflows;
        Calendar calendar;
        void validate() const override;
    };

    class Bond::results : public Instrument::results {
      public:
        Real settlementValue;
        void reset() override {
            settlementValue = Null<Real>();
            Instrument::results::reset();
        }
    };

    class Bond::engine : public GenericEngine<Bond::arguments,
                                              Bond::results> {};


    // inline definitions

    inline Natural Bond::settlementDays() const {
        return settlementDays_;
    }

    inline const Calendar& Bond::calendar() const {
        return calendar_;
    }

    inline const std::vector<Real>& Bond::notionals() const {
        return notionals_;
    }

    inline const Leg& Bond::cashflows() const {
        return cashflows_;
    }

    inline const Leg& Bond::redemptions() const {
        return redemptions_;
    }

    inline Date Bond::issueDate() const {
        return issueDate_;
    }

}

#endif


#ifndef id_9d8c0e25a66b040085098d7169570f0f
#define id_9d8c0e25a66b040085098d7169570f0f
inline bool test_9d8c0e25a66b040085098d7169570f0f(int* i) { return i != 0; }
#endif
