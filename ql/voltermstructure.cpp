
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
#include <ql/dataformatters.hpp>

namespace QuantLib {

	const double BlackVolTermStructure::dT = 1.0/365.0;

	double BlackVolTermStructure::maxTime() const {
        return dayCounter().yearFraction(referenceDate(), maxDate());
    }

    double BlackVolTermStructure::blackVol(const Date& maturity,
        double strike, bool extrapolate) const {

        Time t = dayCounter().yearFraction(referenceDate(), maturity);
        return blackVolImpl(t, strike, extrapolate);
    }

    double BlackVolTermStructure::blackVol(Time maturity,
        double strike, bool extrapolate) const {

        return blackVolImpl(maturity, strike, extrapolate);
    }

    double BlackVolTermStructure::blackVariance(const Date& maturity,
        double strike, bool extrapolate) const {

        Time t = dayCounter().yearFraction(referenceDate(), maturity);
        return blackVarianceImpl(t, strike, extrapolate);
    }

    double BlackVolTermStructure::blackVariance(Time maturity,
        double strike, bool extrapolate) const {

        return blackVarianceImpl(maturity, strike, extrapolate);
    }


    double BlackVolTermStructure::blackForwardVol(const Date& date1,
        const Date& date2, double strike, bool extrapolate) const {

        Time time1 = dayCounter().yearFraction(referenceDate(), date1);
        Time time2 = dayCounter().yearFraction(referenceDate(), date2);
        return blackForwardVol(time1, time2, strike, extrapolate);
    }


    double BlackVolTermStructure::blackForwardVol(Time time1, Time time2,
        double strike, bool extrapolate) const {


        if (time2==time1) {
            if (time1==0.0) {
                Time epsilon = 0.00001;
                double var = blackVarianceImpl(epsilon, strike,
                    extrapolate);
                return QL_SQRT(var/epsilon);
            } else {
                QL_REQUIRE(time1>0.0,
                    "BlackVolTermStructure::blackForwardVol : "
                    "negative times");
                Time epsilon = QL_MIN(0.00001, time1);
                double var1 = blackVarianceImpl(time1-epsilon, strike,
                    extrapolate);
                double var2 = blackVarianceImpl(time1+epsilon, strike,
                    extrapolate);
                QL_REQUIRE(var2>=var1,
                    "BlackVolTermStructure::blackForwardVol : "
                    "variances must be non-decreasing");
                return QL_SQRT((var2-var1)/(2*epsilon));
            }
        } else {
            QL_REQUIRE(time2>time1,
                "BlackVolTermStructure::blackForwardVol : "
                "time2 ("
                + DoubleFormatter::toString(time2) +
                ") < time1("
                + DoubleFormatter::toString(time1) +
                ")");
            double var1 = blackVarianceImpl(time1, strike, extrapolate);
            double var2 = blackVarianceImpl(time2, strike, extrapolate);
            QL_REQUIRE(var2>=var1,
                "BlackVolTermStructure::blackForwardVol : "
                "variances must be non-decreasing");
            return QL_SQRT((var2-var1)/(time2-time1));
        }
    }


    double BlackVolTermStructure::blackForwardVariance(const Date& date1,
        const Date& date2, double strike, bool extrapolate) const {

        Time time1 = dayCounter().yearFraction(referenceDate(), date1);
        Time time2 = dayCounter().yearFraction(referenceDate(), date2);
        return blackForwardVariance(time1, time2, strike, extrapolate);
    }


    double BlackVolTermStructure::blackForwardVariance(Time time1,
        Time time2, double strike, bool extrapolate) const {

        QL_REQUIRE(time2>=time1,
            "VolTermStructure::blackForwardVariance : "
            "time2 ("
            + DoubleFormatter::toString(time2) +
            ") < time1("
            + DoubleFormatter::toString(time1) +
            ")");
        double v1 = blackVarianceImpl(time1, strike, extrapolate);
        double v2 = blackVarianceImpl(time2, strike, extrapolate);
        double result = v2-v1;
        QL_REQUIRE(result>=0.0,
            "BlackVolTermStructure::blackForwardVariance : "
            "variances must be non-decreasing");
        return result;
    }


	 double BlackVolatilityTermStructure ::blackVarianceImpl(Time maturity,
        double strike, bool extrapolate) const {

        double vol = blackVolImpl(maturity, strike, extrapolate);
        return vol*vol*maturity;
    }

	 double BlackVarianceTermStructure ::blackVolImpl(Time maturity,
		 double strike, bool extrapolate) const {

         QL_REQUIRE(maturity>=0,
             "BlackVarianceTermStructure ::blackVolImpl : "
             "negative time not allowed");
        Time nonZeroMaturity = (maturity==0.0 ? 0.00001 : maturity);
        double var = blackVarianceImpl(nonZeroMaturity, strike, extrapolate);
        return QL_SQRT(var/nonZeroMaturity);
    }



    double LocalVolTermStructure::maxTime() const {
        return dayCounter().yearFraction(referenceDate(), maxDate());
    }

    double LocalVolTermStructure::localVol(const Date& d,
        double underlyingLevel, bool extrapolate) const {

        Time t = dayCounter().yearFraction(referenceDate(), d);
        return localVolImpl(t, underlyingLevel, extrapolate);
    }

    double LocalVolTermStructure::localVol(Time t,
        double underlyingLevel, bool extrapolate) const {

        return localVolImpl(t, underlyingLevel, extrapolate);
    }

}


