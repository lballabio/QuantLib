/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
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

#include <ql/termstructures/volatility/flatsmilesection.hpp>

namespace QuantLib {

    FlatSmileSection::FlatSmileSection(const Date& d,
                                       Volatility vol,
                                       const DayCounter& dc,
                                       const Date& referenceDate,
                                       Real atmLevel,
                                       VolatilityType type,
                                       Real shift)
    : SmileSection(d, dc, referenceDate, type, shift),
      vol_(vol), atmLevel_(atmLevel) {}

    FlatSmileSection::FlatSmileSection(Time exerciseTime,
                                       Volatility vol,
                                       const DayCounter& dc,
                                       Real atmLevel,
                                       VolatilityType type,
                                       Real shift)
    : SmileSection(exerciseTime, dc, type, shift),
      vol_(vol), atmLevel_(atmLevel) {}

}
