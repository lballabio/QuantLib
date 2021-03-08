/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Ralf Konrad Eckel

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

#include <ql/cashflows/inflationcashflow.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {
    ZeroInflationCashFlow::ZeroInflationCashFlow(const Real& notional,
                                                 const ext::shared_ptr<ZeroInflationIndex>& index,
                                                 const bool& useInterpolatedFixings,
                                                 const Date& baseDate,
                                                 const Date& fixingDate,
                                                 const Date& paymentDate,
                                                 const bool& growthOnly)
    : IndexedCashFlow(notional, index, baseDate, fixingDate, paymentDate, growthOnly),
      zeroInflationIndex_(index), useInterpolatedFixings_(useInterpolatedFixings) {}

    Real QuantLib::ZeroInflationCashFlow::amount() const { QL_FAIL("Not implemented yet..."); }
}
