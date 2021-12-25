/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Matthias Groncki

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

#include <ql/time/calendars/thailand.hpp>

namespace QuantLib {

    Thailand::Thailand() {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Thailand::SetImpl);
        impl_ = impl;
    }

    bool Thailand::SetImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
            // New Year's Day
            || ((d == 1 || (d==3 && w==Monday)) && m == January)
            // Chakri Memorial Day
            || ((d == 6 || ((d==7 || d==8) && w==Monday)) && m == April)
            // Songkran Festival
            || ((d == 13 || d == 14 || d == 15) && m == April)
            // Songkran Festival obersvence (usually not more then 1 holiday will be replaced)
            || (d == 16 && (w == Monday || w == Tuesday) && m == April)
            // Labor Day
            || ((d == 1 || ((d==2 || d==3) && w==Monday)) && m == May)
            // H.M. the King's Birthday
            || ((d == 28 || ((d==29 || d==30) && w==Monday)) && m == July && y >= 2017)
            // H.M. the Queen's Birthday
            || ((d == 12 || ((d==13 || d==14) && w==Monday)) && m == August)
            // H.M. King Bhumibol Adulyadej Memorial Day
            || ((d == 13 || ((d==14 || d==15) && w==Monday)) && m == October && y >= 2017)
            // H.M. King Bhumibol Adulyadej's Birthday
            || ((d == 5 || ((d==6 || d==7) && w==Monday)) && m == December)
            // Constitution Day
            || ((d == 10 || ((d==11 || d==12) && w==Monday)) && m == December)
            // New Year’s Eve
            || (d == 31 && m == December)
            // New Year’s Eve Observence
            || ((d == 1 || d==2) && w == Monday && m == January)
            )
            return false;

        if ((y == 2000) &&
            (   (d==21 && m==February)  // Makha Bucha Day (Substitution Day)
             || (d==5  && m==May)       // Coronation Day
             || (d==17 && m==May)       // Wisakha Bucha Day
             || (d==17 && m==July)      // Buddhist Lent Day
             || (d==23 && m==October)   // Chulalongkorn Day
                ))
            return false;

        if ((y == 2001) &&
            (   (d==8 && m==February) // Makha Bucha Day
             || (d==7 && m==May)      // Wisakha Bucha Day
             || (d==8 && m==May)      // Coronation Day (Substitution Day)
             || (d==6 && m==July)     // Buddhist Lent Day
             || (d==23 && m==October) // Chulalongkorn Day
                ))
            return false;

        // 2002, 2003 and 2004 are missing

        if ((y == 2005) &&
            (   (d==23 && m==February) // Makha Bucha Day
             || (d==5 && m==May)       // Coronation Day
             || (d==23 && m==May)      // Wisakha Bucha Day (Substitution Day for Sunday 22 May)
             || (d==1 && m==July)      // Mid Year Closing Day
             || (d==22 && m==July)     // Buddhist Lent Day
             || (d==24 && m==October)  // Chulalongkorn Day (Substitution Day for Sunday 23 October)
                ))
            return false;

        if ((y == 2006) &&
            (  (d==13 && m==February) // Makha Bucha Day
            || (d==19 && m==April)    // Special Holiday
            || (d==5 && m==May)       // Coronation Day
            || (d==12 && m==May)      // Wisakha Bucha Day
            || (d==12 && m==June)     // Special Holidays (Due to the auspicious occasion of the
                                      // celebration of 60th Anniversary of His Majesty's Accession
                                      // to the throne. For Bangkok, Samut Prakan, Nonthaburi,
                                      // Pathumthani and Nakhon Pathom province)
            || (d==13 && m==June)     // Special Holidays (as above)
            || (d==11 && m==July)     // Buddhist Lent Day
            || (d==23 && m==October)  // Chulalongkorn Day
               ))
            return false;

        if ((y == 2007) &&
            (   (d==5 && m==March)     // Makha Bucha Day (Substitution Day for Saturday 3 March)
             || (d==7 && m==May)       // Coronation Day (Substitution Day for Saturday 5 May)
             || (d==31 && m==May)      // Wisakha Bucha Day
             || (d==30 && m==July)     // Asarnha Bucha Day (Substitution Day for Sunday 29 July)
             || (d==23 && m==October)  // Chulalongkorn Day
             || (d==24 && m==December) // Special Holiday
                ))
            return false;

        if ((y == 2008) &&
            (   (d==21 && m==February) // Makha Bucha Day
             || (d==5 && m==May)       // Coronation Day
             || (d==19 && m==May)      // Wisakha Bucha Day
             || (d==1 && m==July)      // Mid Year Closing Day
             || (d==17 && m==July)     // Asarnha Bucha Day
             || (d==23 && m==October)  // Chulalongkorn Day
               ))
            return false;

        if ((y == 2009) &&
            (   (d==2 && m==January)  // Special Holiday
             || (d==9 && m==February) // Makha Bucha Day
             || (d==5 && m==May)      // Coronation Day
             || (d==8 && m==May)      // Wisakha Bucha Day
             || (d==1 && m==July)     // Mid Year Closing Day
             || (d==6 && m==July)     // Special Holiday
             || (d==7 && m==July)     // Asarnha Bucha Day
             || (d==23 && m==October) // Chulalongkorn Day
               ))
            return false;

        if ((y == 2010) &&
            (   (d==1 && m==March)    // Substitution for Makha Bucha Day(Sunday 28 February)
             || (d==5 && m==May)      // Coronation Day
             || (d==20 && m==May)     // Special Holiday
             || (d==21 && m==May)     // Special Holiday
             || (d==28 && m==May)     // Wisakha Bucha Day
             || (d==1 && m==July)     // Mid Year Closing Day
             || (d==26 && m==July)    // Asarnha Bucha Day
             || (d==13 && m==August)  // Special Holiday
             || (d==25 && m==October) // Substitution for Chulalongkorn Day(Saturday 23 October)
               ))
            return false;

        if ((y == 2011) &&
            (   (d==18 && m==February) // Makha Bucha Day
             || (d==5 && m==May)       // Coronation Day
             || (d==16 && m==May)      // Special Holiday
             || (d==17 && m==May)      // Wisakha Bucha Day
             || (d==1 && m==July)      // Mid Year Closing Day
             || (d==15 && m==July)     // Asarnha Bucha Day
             || (d==24 && m==October)  // Substitution for Chulalongkorn Day(Sunday 23 October)
               ))
            return false;

        if ((y == 2012) &&
            (   (d==3 && m==January)  // Special Holiday
             || (d==7 && m==March)    // Makha Bucha Day 2/
             || (d==9 && m==April)    // Special Holiday
             || (d==7 && m==May)      // Substitution for Coronation Day(Saturday 5 May)
             || (d==4 && m==June)     // Wisakha Bucha Day
             || (d==2 && m==August)   // Asarnha Bucha Day
             || (d==23 && m==October) // Chulalongkorn Day
                ))
            return false;

        if ((y == 2013) &&
            (   (d==25 && m==February) // Makha Bucha Day
             || (d==6 && m==May)       // Substitution for Coronation Day(Sunday 5 May)
             || (d==24 && m==May)      // Wisakha Bucha Day
             || (d==1 && m==July)      // Mid Year Closing Day
             || (d==22 && m==July)     // Asarnha Bucha Day 2/
             || (d==23 && m==October)  // Chulalongkorn Day
             || (d==30 && m==December) // Special Holiday
                ))
            return false;

        if ((y == 2014) &&
            (   (d==14 && m==February) // Makha Bucha Day
             || (d==5 && m==May)       // Coronation Day
             || (d==13 && m==May)      // Wisakha Bucha Day
             || (d==1 && m==July)      // Mid Year Closing Day
             || (d==11 && m==July)     // Asarnha Bucha Day 1/
             || (d==11 && m==August)   // Special Holiday
             || (d==23 && m==October)  // Chulalongkorn Day
                ))
            return false;

        if ((y == 2015) &&
            (   (d==2 && m==January)  // Special Holiday
             || (d==4 && m==March)    // Makha Bucha Day
             || (d==4 && m==May)      // Special Holiday
             || (d==5 && m==May)      // Coronation Day
             || (d==1 && m==June)     // Wisakha Bucha Day
             || (d==1 && m==July)     // Mid Year Closing Day
             || (d==30 && m==July)    // Asarnha Bucha Day 1/
             || (d==23 && m==October) // Chulalongkorn Day
                ))
            return false;

        if ((y == 2016) &&
            (   (d==22 && m==February) // Makha Bucha Day
             || (d==5 && m==May)       // Coronation Day
             || (d==6 && m==May)       // Special Holiday
             || (d==20 && m==May)      // Wisakha Bucha Day
             || (d==1 && m==July)      //  Mid Year Closing Day
             || (d==18 && m==July)     // Special Holiday
             || (d==19 && m==July)     // Asarnha Bucha Day 1/
             || (d==24 && m==October)  // Substitution for Chulalongkorn Day (Sunday 23rd October)
                ))
            return false;

        if ((y == 2017) &&
            (      (d == 13 && m == February)  // Makha Bucha Day
                || (d == 10 && m == May)       // Wisakha Bucha Day
                || (d == 10 && m == July)      // Asarnha Bucha Day
                || (d == 23 && m == October)   // Chulalongkorn Day
                || (d == 26 && m == October)   // Special Holiday
                ))
            return false;

        if ((y == 2018) &&
            (   (d==1 && m==March)    // Makha Bucha Day
             || (d==29 && m==May)     // Wisakha Bucha Day
             || (d==27 && m==July)    // Asarnha Bucha Day1
             || (d==23 && m==October) // Chulalongkorn Day
                ))
            return false;

        return true;
    }

}
