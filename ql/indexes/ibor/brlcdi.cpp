/*
 Copyright (C) 2025 Sotirios Papathanasopoulos
 
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

#include <ql/indexes/ibor/brlcdi.hpp>

namespace QuantLib {
    // Reference: Zine-eddine, Arroub. "OpenGamma Quantitative research Brazilian Swaps", London, December 2013. paragraph 5
    Rate BRLCdi::forecastFixing(const Date& fixingDate) const {

        Date startDate = valueDate(fixingDate);
        Date endDate = maturityDate(startDate);
        Time yf = dayCounter_.yearFraction(startDate, endDate);

        QL_REQUIRE(yf > 0.0, "year fraction (" << yf << ") must be positive");

        QL_REQUIRE(!termStructure_.empty(), "null term structure set to this instance of " << name());

        DiscountFactor discountStart = termStructure_->discount(startDate);
        DiscountFactor discountEnd = termStructure_->discount(endDate);
        return std::pow(discountStart / discountEnd, 1.0 / yf) - 1.0;
    }

}