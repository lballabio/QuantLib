/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Allen Kuo

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

/*! \file forwardrateagreement.hpp
    \brief forward rate agreement
*/

#ifndef quantlib_forward_rate_agreement_hpp
#define quantlib_forward_rate_agreement_hpp

#include <ql/instruments/forward.hpp>

namespace QuantLib {

    class IborIndex;

    //! %Forward rate agreement (FRA) class
    /*! 1. Unlike the forward contract conventions on carryable
           financial assets (stocks, bonds, commodities), the
           valueDate for a FRA is taken to be the day when the forward
           loan or deposit begins and when full settlement takes place
           (based on the NPV of the contract on that date).
           maturityDate is the date when the forward loan or deposit
           ends. In fact, the FRA settles and expires on the
           valueDate, not on the (later) maturityDate. It follows that
           (maturityDate - valueDate) is the tenor/term of the
           underlying loan or deposit

        2. Choose position type = Long for an "FRA purchase" (future
           long loan, short deposit [borrower])

        3. Choose position type = Short for an "FRA sale" (future short
           loan, long deposit [lender])

        <b>Example: </b>
        \link FRA.cpp
        valuation of a forward-rate agreement
        \endlink

        \todo Add preconditions and tests

        \todo Differentiate between BBA (British)/AFB (French)
              [assumed here] and ABA (Australian) banker conventions
              in the calculations.

        \warning This class still needs to be rigorously tested

        \ingroup instruments
    */
    class ForwardRateAgreement: public Instrument {
      public:
        ForwardRateAgreement(const Date& valueDate,
                             const Date& maturityDate,
                             Position::Type type,
                             Rate strikeForwardRate,
                             Real notionalAmount,
                             const ext::shared_ptr<IborIndex>& index,
                             const Handle<YieldTermStructure>& discountCurve =
                                                 Handle<YieldTermStructure>(),
                             bool useIndexedCoupon = true);
        //! \name Calculations
        //@{
        //! A FRA expires/settles on the value date
        bool isExpired() const override;
        //! The payoff on the value date
        Real amount() const;
        /*! This returns evaluationDate + settlementDays (not FRA
            valueDate).
        */
        Date settlementDate() const;
        const Calendar& calendar() const;
        BusinessDayConvention businessDayConvention() const;
        const DayCounter& dayCounter() const;
        //! term structure relevant to the contract (e.g. repo curve)
        Handle<YieldTermStructure> discountCurve() const;
        //! term structure that discounts the underlying's income cash flows
        Handle<YieldTermStructure> incomeDiscountCurve() const;
        Date fixingDate() const;
        /*!  Income is zero for a FRA */
        Real spotIncome(const Handle<YieldTermStructure>& incomeDiscountCurve) const;
        //! Spot value (NPV) of the underlying loan
        /*! This has always a positive value (asset), even if short the FRA */
        Real spotValue() const;
        //! Returns the relevant forward rate associated with the FRA term
        InterestRate forwardRate() const;

        /*! Simple yield calculation based on underlying spot and
            forward values, taking into account underlying income.
            When \f$ t>0 \f$, call with:
            underlyingSpotValue=spotValue(t),
            forwardValue=strikePrice, to get current yield. For a
            repo, if \f$ t=0 \f$, impliedYield should reproduce the
            spot repo rate. For FRA's, this should reproduce the
            relevant zero rate at the FRA's maturityDate_;
        */
        InterestRate impliedYield(Real underlyingSpotValue,
                                  Real forwardValue,
                                  Date settlementDate,
                                  Compounding compoundingConvention,
                                  const DayCounter& dayCounter);

        virtual Real forwardValue() const;
        //@}

      protected:
        void setupExpired() const override;
        void performCalculations() const override;
        Position::Type fraType_;
        //! aka FRA rate (the market forward rate)
        mutable InterestRate forwardRate_;
        //! aka FRA fixing rate, contract rate
        InterestRate strikeForwardRate_;
        Real notionalAmount_;
        ext::shared_ptr<IborIndex> index_;
        bool useIndexedCoupon_;

        /*! derived classes must set this, typically via spotIncome() */
        mutable Real underlyingIncome_;
        /*! derived classes must set this, typically via spotValue() */
        mutable Real underlyingSpotValue_;
        DayCounter dayCounter_;
        Calendar calendar_;
        BusinessDayConvention businessDayConvention_;
        Natural settlementDays_;
        ext::shared_ptr<Payoff> payoff_;
        /*! valueDate = settlement date (date the fwd contract starts
            accruing)
        */
        Date valueDate_;
        //! maturityDate of the forward contract or delivery date of underlying
        Date maturityDate_;
        Handle<YieldTermStructure> discountCurve_;
        /*! must set this in derived classes, based on particular underlying */
        Handle<YieldTermStructure> incomeDiscountCurve_;

      private:
        void calculateForwardRate() const;
        void calculateAmount() const;
        mutable Real amount_;
    };

    inline const Calendar& ForwardRateAgreement::calendar() const { return calendar_; }

    inline BusinessDayConvention ForwardRateAgreement::businessDayConvention() const {
        return businessDayConvention_;
    }

    inline const DayCounter& ForwardRateAgreement::dayCounter() const { return dayCounter_; }

    inline Handle<YieldTermStructure> ForwardRateAgreement::discountCurve() const {
        return discountCurve_;
    }

    inline Handle<YieldTermStructure> ForwardRateAgreement::incomeDiscountCurve() const {
        return incomeDiscountCurve_;
    }

}


#endif
