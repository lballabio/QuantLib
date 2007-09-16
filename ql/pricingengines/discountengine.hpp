/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file discountengine.hpp
    \brief discount engine
*/

#ifndef quantlib_discount_engine_hpp
#define quantlib_discount_engine_hpp

#include <ql/yieldtermstructure.hpp>
#include <ql/cashflows/cashflows.hpp>

namespace QuantLib {

    class DiscountEngine : public Observer,
                           public Observable {
      public:
        DiscountEngine(const Handle<YieldTermStructure>& discountCurve =
                                                Handle<YieldTermStructure>());

        //! \name Observer interface
        //@{
        void update();
        //@}
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the given term structure.
        */
        Real npv(const Leg& cashflows,
                 const Date& settlementDate = Date(),
                 const Date& npvDate = Date(),
                 Integer exDividendDays = 0);
        //! Basis-point sensitivity of the cash flows.
        /*! The result is the change in NPV due to a uniform
            1-basis-point change in the rate paid by the cash
            flows. The change for each coupon is discounted according
            to the given term structure.
        */
        Real bps(const Leg& leg,
                 const Date& settlementDate = Date(),
                 const Date& npvDate = Date(),
                 Integer exDividendDays = 0);

        Handle<YieldTermStructure> discountCurve() const;
      private:
        Handle<YieldTermStructure> discountCurve_;
    };

}


#endif
