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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fixedratebondforward.hpp
    \brief forward contract on a fixed-rate bond
*/

#ifndef quantlib_fixed_rate_bond_forward_hpp
#define quantlib_fixed_rate_bond_forward_hpp

#include <ql/instruments/bondforward.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>

namespace QuantLib {

    //! %Forward contract on a fixed-rate bond
    /*! \deprecated Use BondForward instead. */
    class [[deprecated]] FixedRateBondForward : public BondForward {
      public:
        FixedRateBondForward(
            const Date& valueDate,
            const Date& maturityDate,
            Position::Type type,
            Real strike,
            Natural settlementDays,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            BusinessDayConvention businessDayConvention,
            const ext::shared_ptr<FixedRateBond>& fixedRateBond,
            const Handle<YieldTermStructure>& discountCurve = Handle<YieldTermStructure>(),
            const Handle<YieldTermStructure>& incomeDiscountCurve = Handle<YieldTermStructure>())
        : BondForward(valueDate,
                      maturityDate,
                      type,
                      strike,
                      settlementDays,
                      dayCounter,
                      calendar,
                      businessDayConvention,
                      fixedRateBond,
                      discountCurve,
                      incomeDiscountCurve) {}
    };
}

#endif