/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Pratyush Patel

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

#include <ql/currencies/asia.hpp>
#include <ql/indexes/ibor/shir.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/israel.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {
    Shir::Shir(const Handle<YieldTermStructure>& h)
    : OvernightIndex("Shir", 0, ILSCurrency(), Israel(Israel::SHIR), Actual365Fixed(), h) {}

}