
/*
 Copyright (C) 2003 RiskMap srl

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

#include "daycounters.hpp"
#include <ql/DayCounters/actualactual.hpp>
#include <ql/dataformatters.hpp>

using namespace QuantLib;
using QuantLib::DayCounters::ActualActual;

namespace {

    struct SingleCase {
        SingleCase(ActualActual::Convention convention,
                   const Date& start,
                   const Date& end,
                   const Date& refStart,
                   const Date& refEnd,
                   double result)
            : convention(convention), start(start), end(end),
              refStart(refStart), refEnd(refEnd), result(result) {}
        SingleCase(ActualActual::Convention convention,
                   const Date& start,
                   const Date& end,
                   double result)
            : convention(convention), start(start), end(end),
              refStart(Date()), refEnd(Date()), result(result) {}
        ActualActual::Convention convention;
        Date start;
        Date end;
        Date refStart;
        Date refEnd;
        double result;
    };

}

void DayCounterTest::runTest() {

    SingleCase testCases[] = {
        // first example
        SingleCase(ActualActual::ISDA, 
                   Date(1,November,2003), Date(1,May,2004),
                   0.497724380567),
        SingleCase(ActualActual::ISMA,
                   Date(1,November,2003), Date(1,May,2004), 
                   Date(1,November,2003), Date(1,May,2004),
                   0.500000000000),
        SingleCase(ActualActual::AFB, 
                   Date(1,November,2003), Date(1,May,2004),
                   0.497267759563),
        // short first calculation period (first period)
        SingleCase(ActualActual::ISDA, 
                   Date(1,February,1999), Date(1,July,1999),
                   0.410958904110),
        SingleCase(ActualActual::ISMA,
                   Date(1,February,1999), Date(1,July,1999),
                   Date(1,July,1998), Date(1,July,1999),
                   0.410958904110),
        SingleCase(ActualActual::AFB, 
                   Date(1,February,1999), Date(1,July,1999),
                   0.410958904110),
        // short first calculation period (second period)
        SingleCase(ActualActual::ISDA, 
                   Date(1,July,1999), Date(1,July,2000),
                   1.001377348600),
        SingleCase(ActualActual::ISMA,
                   Date(1,July,1999), Date(1,July,2000), 
                   Date(1,July,1999), Date(1,July,2000),
                   1.000000000000),
        SingleCase(ActualActual::AFB, 
                   Date(1,July,1999), Date(1,July,2000),
                   1.000000000000),
        // long first calculation period (first period)
        SingleCase(ActualActual::ISDA, 
                   Date(15,August,2002), Date(15,July,2003),
                   0.915068493151),
        SingleCase(ActualActual::ISMA,
                   Date(15,August,2002), Date(15,July,2003),
                   Date(15,January,2003), Date(15,July,2003),
                   0.915760869565),
        SingleCase(ActualActual::AFB, 
                   Date(15,August,2002), Date(15,July,2003),
                   0.915068493151),
        // long first calculation period (second period)
        /* Warning: the ISDA case is in disagreement with mktc1198.pdf */
        SingleCase(ActualActual::ISDA, 
                   Date(15,July,2003), Date(15,January,2004),
                   0.504004790778),
        SingleCase(ActualActual::ISMA,
                   Date(15,July,2003), Date(15,January,2004),
                   Date(15,July,2003), Date(15,January,2004),
                   0.500000000000),
        SingleCase(ActualActual::AFB, 
                   Date(15,July,2003), Date(15,January,2004),
                   0.504109589041),
        // short final calculation period (penultimate period)
        SingleCase(ActualActual::ISDA, 
                   Date(30,July,1999), Date(30,January,2000),
                   0.503892506924),
        SingleCase(ActualActual::ISMA,
                   Date(30,July,1999), Date(30,January,2000),
                   Date(30,July,1999), Date(30,January,2000),
                   0.500000000000),
        SingleCase(ActualActual::AFB, 
                   Date(30,July,1999), Date(30,January,2000),
                   0.504109589041),
        // short final calculation period (final period)
        SingleCase(ActualActual::ISDA, 
                   Date(30,January,2000), Date(30,June,2000),
                   0.415300546448),
        SingleCase(ActualActual::ISMA,
                   Date(30,January,2000), Date(30,June,2000),
                   Date(30,January,2000), Date(30,July,2000),
                   0.417582417582),
        SingleCase(ActualActual::AFB, 
                   Date(30,January,2000), Date(30,June,2000),
                   0.41530054644)
    };

    int n = sizeof(testCases)/sizeof(SingleCase);
    for (int i=0; i<n; i++) {
        ActualActual dayCounter(testCases[i].convention);
        Date d1 = testCases[i].start,
             d2 = testCases[i].end,
             rd1 = testCases[i].refStart,
             rd2 = testCases[i].refEnd;
        double calculated = dayCounter.yearFraction(d1,d2,rd1,rd2);
                                                    
        if (QL_FABS(calculated-testCases[i].result) > 1.0e-10) {
            std::string period, refPeriod;
            period = "period: " 
                + DateFormatter::toString(d1) + " to " 
                + DateFormatter::toString(d2) + "\n" ;
            if (testCases[i].convention == ActualActual::ISMA)
                refPeriod = "referencePeriod: "
                    + DateFormatter::toString(rd1) + " to " 
                    + DateFormatter::toString(rd2) + "\n";
            CPPUNIT_FAIL(
                dayCounter.name() + ":\n"
                + period + refPeriod +
                "    calculated: " 
                + DoubleFormatter::toString(calculated) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(testCases[i].result,11));
        }
    }
}

