
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/voltermstructure.hpp>

namespace QuantLib {

	const Time BlackVolTermStructure::dT = 1.0/365.0;

    Volatility BlackVolTermStructure::blackForwardVol(const Date& date1,
                                                      const Date& date2, 
                                                      Real strike, 
                                                      bool extrapolate) const {
        QL_REQUIRE(date1 <= date2,
                   DateFormatter::toString(date1) +
                   " later than " +
                   DateFormatter::toString(date2));
        Time time1 = dayCounter().yearFraction(referenceDate(), date1);
        Time time2 = dayCounter().yearFraction(referenceDate(), date2);
        return blackForwardVol(time1, time2, strike, extrapolate);
    }

    Volatility BlackVolTermStructure::blackForwardVol(Time time1, Time time2,
                                                      Real strike, 
                                                      bool extrapolate) const {
        QL_REQUIRE(time1 <= time2,
                   DecimalFormatter::toString(time1) +
                   " later than " +
                   DecimalFormatter::toString(time2));
        checkRange(time2,strike,extrapolate);
        if (time2==time1) {
            if (time1==0.0) {
                Time epsilon = 0.00001;
                Real var = blackVarianceImpl(epsilon, strike);
                return QL_SQRT(var/epsilon);
            } else {
                Time epsilon = QL_MIN(0.00001, time1);
                Real var1 = blackVarianceImpl(time1-epsilon, strike);
                Real var2 = blackVarianceImpl(time1+epsilon, strike);
                QL_ENSURE(var2>=var1,
                          "variances must be non-decreasing");
                return QL_SQRT((var2-var1)/(2*epsilon));
            }
        } else {
            Real var1 = blackVarianceImpl(time1, strike);
            Real var2 = blackVarianceImpl(time2, strike);
            QL_ENSURE(var2 >= var1,
                      "variances must be non-decreasing");
            return QL_SQRT((var2-var1)/(time2-time1));
        }
    }

    Real BlackVolTermStructure::blackForwardVariance(const Date& date1,
                                                     const Date& date2, 
                                                     Real strike,
                                                     bool extrapolate) 
                                                                      const {
        QL_REQUIRE(date1 <= date2,
                   DateFormatter::toString(date1) +
                   " later than " +
                   DateFormatter::toString(date2));
        Time time1 = dayCounter().yearFraction(referenceDate(), date1);
        Time time2 = dayCounter().yearFraction(referenceDate(), date2);
        return blackForwardVariance(time1, time2, strike, extrapolate);
    }


    Real BlackVolTermStructure::blackForwardVariance(Time time1, Time time2, 
                                                     Real strike,
                                                     bool extrapolate) const {
        QL_REQUIRE(time1 <= time2,
                   DecimalFormatter::toString(time1) +
                   " later than " +
                   DecimalFormatter::toString(time2));
        checkRange(time2, strike, extrapolate);
        Real v1 = blackVarianceImpl(time1, strike);
        Real v2 = blackVarianceImpl(time2, strike);
        QL_ENSURE(v2 >= v1,
                  "variances must be non-decreasing");
        return v2-v1;
    }

}


