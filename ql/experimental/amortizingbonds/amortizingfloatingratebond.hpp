/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file amortizingfloatingratebond.hpp
    \brief amortizing floating-rate bond
*/

#ifndef quantlib_amortizing_floating_rate_bond_hpp
#define quantlib_amortizing_floating_rate_bond_hpp

#include <ql/instruments/bond.hpp>

namespace QuantLib {

    class Schedule;
    class IborIndex;

    //! amortizing floating-rate bond (possibly capped and/or floored)
    class AmortizingFloatingRateBond : public Bond {
      public:
        AmortizingFloatingRateBond(
                        Natural settlementDays,
                        const std::vector<Real>& notional,
                        const Schedule& schedule,
                        const boost::shared_ptr<IborIndex>& index,
                        const DayCounter& accrualDayCounter,
                        BusinessDayConvention paymentConvention = Following,
                        Natural fixingDays = Null<Natural>(),
                        const std::vector<Real>& gearings =
                                                    std::vector<Real>(1, 1.0),
                        const std::vector<Spread>& spreads =
                                                  std::vector<Spread>(1, 0.0),
                        const std::vector<Rate>& caps = std::vector<Rate>(),
                        const std::vector<Rate>& floors = std::vector<Rate>(),
                        bool inArrears = false,
                        const Date& issueDate = Date());
    };

}

#endif
