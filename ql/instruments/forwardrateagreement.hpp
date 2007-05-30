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

        4. If strike is given in the constructor, can calculate the NPV
           of the contract via NPV().

        5. If forward rate is desired/unknown, it can be obtained via
           forwardRate(). In this case, the strike variable in the
           constructor is irrelevant and will be ignored.

        <b>Example: </b>
        \link FRA.cpp
        valuation of a forward-rate agreement
        \endlink

        \todo Add preconditions and tests

        \todo Should put an instance of ForwardRateAgreement in the
              FraRateHelper to ensure consistency with the piecewise
              yield curve.

        \todo Differentiate between BBA (British)/AFB (French)
              [assumed here] and ABA (Australian) banker conventions
              in the calculations.

        \warning This class still needs to be rigorously tested

        \ingroup instruments
    */
    class IborIndex;
    class ForwardRateAgreement: public Forward {
      public:
        ForwardRateAgreement(const Date& valueDate,
                             const Date& maturityDate,
                             Position::Type type,
                             Rate strikeForwardRate,
                             Real notionalAmount,
                             const boost::shared_ptr<IborIndex>& index,
                             const Handle<YieldTermStructure>& discountCurve =
                                                 Handle<YieldTermStructure>());
        //! \name Calculations
        //@{
        /*! A FRA expires/settles on the valueDate */
        bool isExpired() const;
        /*! This returns evaluationDate + settlementDays (not FRA
            valueDate).
        */
        Date settlementDate() const;
        /*!  Income is zero for a FRA */
        Real spotIncome(const Handle<YieldTermStructure>& incomeDiscountCurve)
            const;
        //! Spot value (NPV) of the underlying loan
        /*! This has always a positive value (asset), even if short the FRA */
        Real spotValue() const;
        //! Returns the relevant forward rate associated with the FRA term
        InterestRate forwardRate() const;
        //@}

      protected:
        void performCalculations() const;
        Position::Type fraType_;
        //! aka FRA rate (the market forward rate)
        mutable InterestRate forwardRate_;
        //! aka FRA fixing rate, contract rate
        InterestRate strikeForwardRate_;
        Real notionalAmount_;
        boost::shared_ptr<IborIndex> index_;
    };

}


#endif

