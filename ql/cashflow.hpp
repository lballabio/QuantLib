/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009 StatPro Italia srl

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

/*! \file cashflow.hpp
    \brief Base class for cash flows
*/

#ifndef quantlib_cash_flow_hpp
#define quantlib_cash_flow_hpp

#include <ql/event.hpp>
#include <ql/math/comparison.hpp>
#include <vector>

namespace QuantLib {

    //! Base class for cash flows
    /*! This class is purely virtual and acts as a base class for the
        actual cash flow implementations.
    */
    class CashFlow : public Event {
      public:
        virtual ~CashFlow() {}
        //! \name Event interface
        //@{
        //! \note This is inherited from the event class
        virtual Date date() const = 0;
        //! returns true if an event has already occurred before a date
        /*! overloads Event::hasOccurred in order to take
            Settings::includeTodaysCashflows in account
        */
        bool hasOccurred(
                    const Date& refDate = Date(),
                    boost::optional<bool> includeRefDate = boost::none) const;
        //@}
        //! \name CashFlow interface
        //@{
        //! returns the amount of the cash flow
        /*! \note The amount is not discounted, i.e., it is the actual
                  amount paid at the cash flow date.
        */
        virtual Real amount() const = 0;
        //! returns the date that the cash flow trades exCoupon
        virtual Date exCouponDate() const {return Date();};
        //! returns true if the cashflow is trading ex-coupon on the refDate
        bool tradingExCoupon(const Date& refDate = Date()) const;

        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! Sequence of cash-flows
    typedef std::vector<boost::shared_ptr<CashFlow> > Leg;

    template <>
    struct earlier_than<CashFlow>
            : public std::binary_function<CashFlow,CashFlow,bool> {
        bool operator()(const CashFlow& c1,
                        const CashFlow& c2) {
            return c1.date() < c2.date();
        }
    };

}

#endif
