/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Matthias Groncki
 Copyright (C) 2023, 2024 Skandinaviska Enskilda Banken AB (publ)

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
            || ((d == 1 || (d == 3 && w == Monday)) && m == January)
            // Chakri Memorial Day
            || ((d == 6 || ((d == 7 || d == 8) && w == Monday)) && m == April)
            // Songkran Festival (was cancelled in 2020 due to the Covid-19 Pandamic)
            || ((d == 13 || d == 14 || d == 15) && m == April && y != 2020)
            // Substitution Songkran Festival, usually not more than 5 days in total (was cancelled
            // in 2020 due to the Covid-19 Pandamic)
            || (d == 16 && (w == Monday || w == Tuesday) && m == April && y != 2020)
            // Labor Day
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) && m == May)
            // Coronation Day
            || ((d == 4 || ((d == 5 || d == 6) && w == Monday)) && m == May && y >= 2019)
            // H.M.Queen Suthida Bajrasudhabimalalakshana’s Birthday
            || ((d == 03 || ((d == 04 || d == 05) && w == Monday)) && m == June && y >= 2019)
            // H.M. King Maha Vajiralongkorn Phra Vajiraklaochaoyuhua’s Birthday
            || ((d == 28 || ((d == 29 || d == 30) && w == Monday)) && m == July && y >= 2017)
            // 	​H.M. Queen Sirikit The Queen Mother’s Birthday / Mother’s Day
            || ((d == 12 || ((d == 13 || d == 14) && w == Monday)) && m == August)
            // H.M. King Bhumibol Adulyadej The Great Memorial Day
            || ((d == 13 || ((d == 14 || d == 15) && w == Monday)) && m == October && y >= 2017)
            // Chulalongkorn Day
            || ((d == 23 || ((d == 24 || d == 25) && w == Monday)) && m == October && y != 2021)  // Moved 2021, see below
            // H.M. King Bhumibol Adulyadej The Great’s Birthday/ National Day / Father’s Day
            || ((d == 5 || ((d == 6 || d == 7) && w == Monday)) && m == December)
            // Constitution Day
            || ((d == 10 || ((d == 11 || d == 12) && w == Monday)) && m == December)
            // New Year’s Eve
            || ((d == 31 && m == December) || (d == 2 && w == Monday && m == January && y != 2024))  // Moved 2024
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

        if ((y == 2019) && ((d == 19 && m == February) // Makha Bucha Day
                            || (d == 6 && m == May)    // Special Holiday
                            || (d == 20 && m == May)   // Wisakha Bucha Day
                            || (d == 16 && m == July)  // Asarnha Bucha Day
                            ))
            return false;

        if ((y == 2020) && ((d == 10 && m == February)    // Makha Bucha Day
                            || (d == 6 && m == May)       // Wisakha Bucha Day
                            || (d == 6 && m == July)      // Asarnha Bucha Day
                            || (d == 27 && m == July)     // Substitution for Songkran Festival
                            || (d == 4 && m == September) // Substitution for Songkran Festival
                            || (d == 7 && m == September) // Substitution for Songkran Festival
                            || (d == 11 && m == December) // Special Holiday
                            ))
            return false;

        if ((y == 2021) && ((d == 12 && m == February)     // Special Holiday
                            || (d == 26 && m == February)  // Makha Bucha Day
                            || (d == 26 && m == May)       // Wisakha Bucha Day
                            || (d == 26 && m == July)      // Substitution for Asarnha Bucha Day (Saturday 24th July 2021)
                            || (d == 24 && m == September) // Special Holiday
                            || (d == 22 && m == October)   // ​Substitution for Chulalongkorn Day
                            ))
            return false;

        if ((y == 2022) && ((d == 16 && m == February)   // Makha Bucha Day
                            || (d == 16 && m == May)     // Substitution for Wisakha Bucha Day (Sunday 15th May 2022)
                            || (d == 13 && m == July)    // Asarnha Bucha Day
                            || (d == 29 && m == July)    // Additional special holiday (added)
                            || (d == 14 && m == October) // Additional special holiday (added)
                            || (d == 24 && m == October) // ​Substitution for Chulalongkorn Day (Sunday 23rd October 2022)
            ))
            return false;

        if ((y == 2023) && ((d == 6 && m == March)        // Makha Bucha Day
                            || (d == 5 && m == May)       // Additional special holiday (added)
                            || (d == 5 && m == June)      // Substitution for H.M. Queen's birthday and Wisakha Bucha Day (Saturday 3rd June 2022)
                            || (d == 1 && m == August)    // Asarnha Bucha Day
                            || (d == 23 && m == October)  // Chulalongkorn Day
                            || (d == 29 && m == December) // Substitution for New Year’s Eve (Sunday 31st December 2023) (added)
            ))
            return false;

        if ((y == 2024) && ((d == 26 && m == February)    // Substitution for Makha Bucha Day (Saturday 24th February 2024)
                            || (d == 8 && m == April)     // Substitution for Chakri Memorial Day (Saturday 6th April 2024)
                            || (d == 12 && m == April)    // Additional holiday in relation to the Songkran festival
                            || (d == 6 && m == May)       // Substitution for Coronation Day (Saturday 4th May 2024)
                            || (d == 22 && m == May)      // Wisakha Bucha Day
                            || (d == 22 && m == July)     // Substitution for Asarnha Bucha Day (Saturday 20th July 2024)
                            || (d == 23 && m == October)  // Chulalongkorn Day
            ))
            return false;

        if ((y == 2025) && ((d == 12 && m == February)    // Substitution for Makha Bucha Day (Wednesday 12th February 2025)
            || (d == 7 && m == April)     // Substitution for Chakri Memorial Day (Sunday 6th April 2025)
            || (d == 5 && m == May)       // Substitution for Coronation Day (Sunday 4th May 2025)
            || (d == 12 && m == May)      // Wisakha Bucha Day
            || (d == 10 && m == July)     // Substitution for Asarnha Bucha Day (Tuesday 20th July 2025)
            || (d == 23 && m == October)  // Chulalongkorn Day
            ))
            return false;

        return true;
    }

}
