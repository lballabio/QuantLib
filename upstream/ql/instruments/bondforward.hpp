/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Allen Kuo
 Copyright (C) 2022 Marcin Rybacki

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

/*! \file bondforward.hpp
    \brief forward contract on a bond
*/

#ifndef quantlib_bond_forward_hpp
#define quantlib_bond_forward_hpp

#include <ql/instruments/forward.hpp>
#include <ql/instruments/bond.hpp>

namespace QuantLib {

    //! %Forward contract on a bond
    /*! 1. valueDate refers to the settlement date of the bond forward
           contract.  maturityDate is the delivery (or repurchase)
           date for the underlying bond (not the bond's maturity
           date).

        2. Relevant formulas used in the calculations (\f$P\f$ refers
           to a price):

           a. \f$ P_{CleanFwd}(t) = P_{DirtyFwd}(t) -
              AI(t=deliveryDate) \f$ where \f$ AI \f$ refers to the
              accrued interest on the underlying bond.

           b. \f$ P_{DirtyFwd}(t) = \frac{P_{DirtySpot}(t) -
              SpotIncome(t)} {discountCurve->discount(t=deliveryDate)} \f$

           c. \f$ SpotIncome(t) = \sum_i \left( CF_i \times
              incomeDiscountCurve->discount(t_i) \right) \f$ where \f$
              CF_i \f$ represents the ith bond cash flow (coupon
              payment) associated with the underlying bond falling
              between the settlementDate and the deliveryDate. (Note
              the two different discount curves used in b. and c.)

        \todo Add preconditions and tests

        \todo Create switch- if coupon goes to seller is toggled on,
              don't consider income in the \f$ P_{DirtyFwd}(t) \f$
              calculation.

        \todo Verify this works when the underlying is paper (in which
              case ignore all AI.)

        \warning This class still needs to be rigorously tested

        \ingroup instruments
    */
    class BondForward : public Forward {
      public:
        //! \name Constructors
        /*! If strike is given in the constructor, can calculate the
            NPV of the contract via NPV().

            If strike/forward price is desired, it can be obtained via
            forwardPrice(). In this case, the strike variable in the
            constructor is irrelevant and will be ignored.
        */
        //@{
        BondForward(
            const Date& valueDate,
            const Date& maturityDate,
            Position::Type type,
            Real strike,
            Natural settlementDays,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            BusinessDayConvention businessDayConvention,
            const ext::shared_ptr<Bond>& bond,
            const Handle<YieldTermStructure>& discountCurve = Handle<YieldTermStructure>(),
            const Handle<YieldTermStructure>& incomeDiscountCurve = Handle<YieldTermStructure>());
        //@}

        //! \name Calculations
        //@{

        //! (dirty) forward bond price
        Real forwardPrice() const;

        //! (dirty) forward bond price minus accrued on bond at delivery
        Real cleanForwardPrice() const;

        //!  NPV of bond coupons discounted using incomeDiscountCurve
        /*! Here only coupons between max(evaluation date,settlement
            date) and maturity date of bond forward contract are
            considered income.
        */
        Real spotIncome(const Handle<YieldTermStructure>& incomeDiscountCurve) const override;

        //!  NPV of underlying bond
        Real spotValue() const override;

        //@}

      protected:
        ext::shared_ptr<Bond> bond_;
        void performCalculations() const override;
    };

}

#endif
