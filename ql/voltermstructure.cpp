/*
 Copyright (C) 2002 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file voltermstructure.hpp
    \brief Volatility term structures

    \fullpath
    ql/%voltermstructure.hpp
*/

// $Id$


#include "ql/voltermstructure.hpp"

namespace QuantLib {

	const double BlackVolTermStructure::dT = 1.0/365.0;

	BlackVolTermStructure::~BlackVolTermStructure() {}
        
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

        QL_REQUIRE(time2>=time1,
            "VolTermStructure::blackForwardVol : "
            "time2<time1");
        double var1 = blackVarianceImpl(time1, strike, extrapolate);
        if (time2==time1) {
            Time epsilon = 0.00001;
            return QL_SQRT(
                (blackVarianceImpl(time1+epsilon, strike, extrapolate)-var1)
                /epsilon
                );
        } else {
            return QL_SQRT(
                (blackVarianceImpl(time2, strike, extrapolate)-var1)
                /(time2-time1)
                );
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

        QL_REQUIRE(time2>time1,
            "VolTermStructure::blackForwardVariance : "
            "time2<=time1");
        double v1 = blackVarianceImpl(time1, strike, extrapolate);
        double v2 = blackVarianceImpl(time2, strike, extrapolate);
        return (v2-v1);
    }

 
	double BlackVolTermStructure::timeDerivative(Time maturity,
                        double strike,
                        bool extrapolate = false) const {
		return (blackVol(maturity+dT, strike, extrapolate) -
			blackVol(maturity, strike, extrapolate))/dT	;
	}
     

     double BlackVolTermStructure::strikeDerivative(Time maturity,
                        double strike,
                        bool extrapolate = false) const {
		 double dK = 1.0e-3*strike;
		 return (blackVol(maturity, strike+dK, extrapolate) -
			blackVol(maturity, strike-dK, extrapolate))/(2*dK);
	 }
     

     double BlackVolTermStructure::strikeSecondDerivative(Time maturity,
                        double strike,
                        bool extrapolate = false) const {
		 double dK = 1.0e-3*strike;
		 return (blackVol(maturity, strike+dK, extrapolate) +
			blackVol(maturity, strike-dK, extrapolate) -
			2*blackVol(maturity, strike, extrapolate))/(dK*dK);
	 } 

	  
	 double BlackVolatilityTermStructure ::blackVarianceImpl(Time maturity,
        double strike, bool extrapolate) const {

        double vol = blackVolImpl(maturity, strike, extrapolate);
        return vol*vol*maturity;
    }
	 
	 double BlackVarianceTermStructure ::blackVolImpl(Time maturity,
		 double strike, bool extrapolate) const {

        Time nonZeroMaturity = (maturity==0.0 ? 0.00001 : maturity);
        double var = blackVarianceImpl(nonZeroMaturity, strike, extrapolate);
        return QL_SQRT(var/nonZeroMaturity);
    }

    LocalVolTermStructure::~LocalVolTermStructure() {}


    double LocalVolTermStructure::maxTime() const {
        return dayCounter().yearFraction(referenceDate(), maxDate());
    }

    double LocalVolTermStructure::localVol(const Date& date1,
        const Date& date2, double underlyingLevel, bool extrapolate) const {

        Time t1 = dayCounter().yearFraction(referenceDate(), date1);
        Time t2 = dayCounter().yearFraction(referenceDate(), date2);
        return localVolImpl(t1, t2, underlyingLevel, extrapolate);
    }

    double LocalVolTermStructure::localVol(Time t1, Time t2,
        double underlyingLevel, bool extrapolate) const {

        return localVolImpl(t1, t2, underlyingLevel, extrapolate);
    }

    double LocalVolTermStructure::localVariance(const Date& date1,
        const Date& date2, double underlyingLevel, bool extrapolate) const {

        Time t1 = dayCounter().yearFraction(referenceDate(), date1);
        Time t2 = dayCounter().yearFraction(referenceDate(), date2);
        return localVarianceImpl(t1, t2, underlyingLevel, extrapolate);
    }

    double LocalVolTermStructure::localVariance(Time t1, Time t2,
        double underlyingLevel, bool extrapolate) const {

        return localVarianceImpl(t1, t2, underlyingLevel, extrapolate);
    }
	     
    double LocalVolatilityTermStructure ::localVarianceImpl(Time t1, Time t2,
		double underlyingLevel, bool extrapolate) const {

        double vol = localVolImpl(t1, t2, underlyingLevel, extrapolate);
        return vol*vol*(t2-t1);
    }

    double LocalVarianceTermStructure ::localVolImpl(Time t1, Time t2,
		double underlyingLevel, bool extrapolate) const {

        double var = localVarianceImpl(t1, t2, underlyingLevel, extrapolate);
        return QL_SQRT(var/(t2-t1));
    }

}


