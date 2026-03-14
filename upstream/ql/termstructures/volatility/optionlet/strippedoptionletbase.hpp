/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers

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

/*! \file strippedoptionletbase.hpp

*/

#ifndef quantlib_strippedoptionletbase_hpp
#define quantlib_strippedoptionletbase_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/types.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

#include <vector>

namespace QuantLib {

    class Date;
    class Calendar;
    class DayCounter;

    /*! Abstract base class interface for a (time indexed) vector of (strike
        indexed) optionlet (i.e. caplet/floorlet) volatilities.
    */
    class StrippedOptionletBase : public LazyObject {
      public:
        virtual const std::vector<Rate>& optionletStrikes(Size i) const = 0;
        virtual const std::vector<Volatility>& optionletVolatilities(Size i) const = 0;

        virtual const std::vector<Date>& optionletFixingDates() const = 0;
        virtual const std::vector<Time>& optionletFixingTimes() const = 0;
        virtual Size optionletMaturities() const = 0;

        virtual const std::vector<Rate>& atmOptionletRates() const = 0;

        virtual DayCounter dayCounter() const = 0;
        virtual Calendar calendar() const = 0;
        virtual Natural settlementDays() const = 0;
        virtual BusinessDayConvention businessDayConvention() const = 0;
        virtual VolatilityType volatilityType() const = 0;
        virtual Real displacement() const = 0;
    };

}

#endif
