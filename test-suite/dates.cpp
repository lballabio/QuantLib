
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2003 RiskMap srl

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

#include "dates.hpp"
#include <ql/date.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void DateTest::immDates() {
    BOOST_MESSAGE("Testing IMM dates...");

    Date counter = Date::minDate();
    Date last = Date::maxDate() - 4*Months;
    Date imm;

    while (counter<=last) {
        imm = Date::nextIMMdate(counter);
        // check that imm is greater than or equal to counter
        if (imm<counter)
            BOOST_FAIL("\n  "
                       << imm.weekday() << " " << imm
                       << " is not greater than or equal to "
                       << counter.weekday() << " " << counter);
        // check that imm is an IMM date
        if (!Date::isIMMdate(imm))
            BOOST_FAIL("\n  "
                       << imm.weekday() << " " << imm
                       << " is not an IMM date (calculated from "
                       << counter.weekday() << " " << counter << ")");
        // check that if counter is an IMM date, then imm==counter
        if (Date::isIMMdate(counter) && (imm!=counter))
            BOOST_FAIL("\n  "
                       << counter.weekday() << " " << counter
                       << " is already an IMM date, while nextIMM() returns "
                       << imm.weekday() << " " << imm);

        counter = counter + 1;
    }
}

void DateTest::testConsistency() {

    BOOST_MESSAGE("Testing dates...");

    BigInteger minDate = Date::minDate().serialNumber()+1,
               maxDate = Date::maxDate().serialNumber();

    BigInteger dyold = Date(minDate-1).dayOfYear(),
               dold  = Date(minDate-1).dayOfMonth(),
               mold  = Date(minDate-1).month(),
               yold  = Date(minDate-1).year(),
               wdold = Date(minDate-1).weekday();

    for (BigInteger i=minDate; i<=maxDate; i++) {
        Date t(i);
        BigInteger serial = t.serialNumber();

        // check serial number consistency
        if (serial != i)
            BOOST_FAIL("inconsistent serial number:\n"
                       << "    original:      " << i << "\n"
                       << "    date:          " << t << "\n"
                       << "    serial number: " << serial);

        Integer dy = t.dayOfYear(),
                d  = t.dayOfMonth(),
                m  = t.month(),
                y  = t.year(),
                wd = t.weekday();

        // check if skipping any date
        if (!((dy == dyold+1) ||
              (dy == 1 && dyold == 365 && !Date::isLeap(yold)) ||
              (dy == 1 && dyold == 366 && Date::isLeap(yold))))
            BOOST_FAIL("wrong day of year increment: \n"
                       << "    date: " << t << "\n"
                       << "    day of year: " << dy << "\n"
                       << "    previous:    " << dyold);
        dyold = dy;

        if (!((d == dold+1 && m == mold   && y == yold) ||
              (d == 1      && m == mold+1 && y == yold) ||
              (d == 1      && m == 1      && y == yold+1)))
            BOOST_FAIL("wrong day,month,year increment: \n"
                       << "    date: " << t << "\n"
                       << "    day,month,year: "
                       << d << "," << Integer(m) << "," << y << "\n"
                       << "    previous:       "
                       << dold<< "," << Integer(mold) << "," << yold);
        dold = d; mold = m; yold = y;

        // check month definition
        if (m < 1 || m > 12)
            BOOST_FAIL("invalid month: \n"
                       << "    date:  " << t << "\n"
                       << "    month: " << Integer(m));

        // check day definition
        if (d < 1)
            BOOST_FAIL("invalid day of month: \n"
                       << "    date:  " << t << "\n"
                       << "    day: " << d);
        if (!((m == 1  && d <= 31) ||
              (m == 2  && d <= 28) ||
              (m == 2  && d == 29 && Date::isLeap(y)) ||
              (m == 3  && d <= 31) ||
              (m == 4  && d <= 30) ||
              (m == 5  && d <= 31) ||
              (m == 6  && d <= 30) ||
              (m == 7  && d <= 31) ||
              (m == 8  && d <= 31) ||
              (m == 9  && d <= 30) ||
              (m == 10 && d <= 31) ||
              (m == 11 && d <= 30) ||
              (m == 12 && d <= 31)))
            BOOST_FAIL("invalid day of month: \n"
                       << "    date:  " << t << "\n"
                       << "    day: " << d);

        // check weekday definition
        if (!((Integer(wd) == Integer(wdold+1)) ||
              (Integer(wd) == 1 && Integer(wdold) == 7)))
            BOOST_FAIL("invalid weekday: \n"
                       << "    date:  " << t << "\n"
                       << "    weekday:  " << Integer(wd) << "\n"
                       << "    previous: " << Integer(wdold));
        wdold = wd;

        // create the same date with a different constructor
        Date s(d,Month(m),y);
        // check serial number consistency
        serial = s.serialNumber();
        if (serial != i)
            BOOST_FAIL("inconsistent serial number:\n"
                       << "    date:          " << t << "\n"
                       << "    serial number: " << i << "\n"
                       << "    cloned date:   " <<  s << "\n"
                       << "    serial number: " << serial);
    }

}


test_suite* DateTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Date tests");
    suite->add(BOOST_TEST_CASE(&DateTest::testConsistency));
    suite->add(BOOST_TEST_CASE(&DateTest::immDates));
    return suite;
}

