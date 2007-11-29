/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

/*! \file strippedoptionletbase.hpp

*/

#ifndef quantlib_strippedoptionletbase_hpp
#define quantlib_strippedoptionletbase_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <vector>

namespace QuantLib {

    class StrippedOptionletBase : public LazyObject {
      public:
        virtual const std::vector<Rate>& optionletStrikes(Size i) const = 0;
        virtual const std::vector<Volatility>& optionletVolatilities(Size i) const = 0;

        virtual const std::vector<Date>& optionletDates() const = 0;
        virtual const std::vector<Time>& optionletTimes() const = 0;

        virtual DayCounter dayCounter() const = 0;
        virtual Calendar calendar() const = 0;
        virtual Natural settlementDays() const = 0;
        virtual BusinessDayConvention businessDayConvention() const = 0;
        virtual const Date& referenceDate() const = 0;
    };
}

#endif
