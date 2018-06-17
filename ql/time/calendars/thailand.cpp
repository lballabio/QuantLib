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

    Thailand::Thailand(Market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Thailand::SetImpl);
        impl_ = impl;
    }

    bool Thailand::SetImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
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
            || ((d == 3 || ((d==14 || d==15) && w==Monday)) && m == October && y >= 2017)
            // H.M. King Bhumibol Adulyadej's Birthday
            || ((d == 5 || ((d==6 || d==7) && w==Monday)) && m == December)
            // Constitution Day
            || ((d == 10 || ((d==11 || d==12) && w==Monday)) && m == December)
            // New Year’s Eve
            || (d == 31 && m == December)
            // New Year’s Eve Observence
            || ((d == 1 || d==2) && w == Monday && m == January)


            //2000

            ||(d==21 & m==February & y==2000) // Makha Bucha Day​ (Substitution Day)
            ||(d==5 & m==May & y==2000) // Coronation Day
            ||(d==17 & m==May & y==2000) // Wisakha Bucha Day
            ||(d==17 & m==July & y==2000) // Buddhist Lent Day
            ||(d==23 & m==October & y==2000) // Chulalongkorn Day


            //2001

            ||(d==8 & m==February & y==2001) // Makha Bucha Day
            ||(d==7 & m==May & y==2001) // Wisakha Bucha Day
            ||(d==8 & m==May & y==2001) // Coronation Day (Substitution Day)
            ||(d==6 & m==July & y==2001) // Buddhist Lent Day
            ||(d==23 & m==October & y==2001) // Chulalongkorn Day


            //2002



            //2003



            //2004



            //2005

            ||(d==23 & m==February & y==2005) // Makha Bucha Day
            ||(d==5 & m==May & y==2005) // Coronation Day
            ||(d==23 & m==May & y==2005) // Wisakha Bucha Day (Substitution Day for Sunday 22 May 2005)
            ||(d==1 & m==July & y==2005) // Mid Year Closing Day
            ||(d==22 & m==July & y==2005) // Buddhist Lent Day
            ||(d==24 & m==October & y==2005) // Chulalongkorn Day (Substitution Day for Sunday 23 October 2005)


            //2006

            ||(d==13 & m==February & y==2006) // Makha Bucha Day
            ||(d==19 & m==April & y==2006) // Special Holiday
            ||(d==5 & m==May & y==2006) // Coronation Day
            ||(d==12 & m==May & y==2006) // Wisakha Bucha Day
            ||(d==12 & m==June & y==2006) // Special Holidays(Due to the auspicious occasion of the celebration of 60th Anniversary of His Majesty's Accession to the throne. For Bangkok, Samut Prakan, Nonthaburi, Pathumthani and Nakhon Pathom province)
            ||(d==13 & m==June & y==2006) // Special Holidays(Due to the auspicious occasion of the celebration of 60th Anniversary of His Majesty's Accession to the throne. For Bangkok, Samut Prakan, Nonthaburi, Pathumthani and Nakhon Pathom province)
            ||(d==11 & m==July & y==2006) // Buddhist Lent Day
            ||(d==23 & m==October & y==2006) // Chulalongkorn Day


            //2007

            ||(d==5 & m==March & y==2007) // Makha Bucha Day (Substitution Day for Saturday 3 March)
            ||(d==7 & m==May & y==2007) // Coronation Day (Substitution Day for Saturday 5 May)
            ||(d==31 & m==May & y==2007) // Wisakha Bucha Day
            ||(d==30 & m==July & y==2007) // Asarnha Bucha Day (Substitution Day for Sunday 29 July)Effective from B.E. 2550 onwards, the Asarnha Bucha Day is prescribed as a holiday, in replacement of the Buddhist Lent Day
            ||(d==23 & m==October & y==2007) // Chulalongkorn Day
            ||(d==24 & m==December & y==2007) // Special Holiday


            //2008

            ||(d==21 & m==February & y==2008) // Makha Bucha Day
            ||(d==5 & m==May & y==2008) // Coronation Day 
            ||(d==19 & m==May & y==2008) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2008) // Mid Year Closing Day
            ||(d==17 & m==July & y==2008) // Asarnha Bucha Day
            ||(d==23 & m==October & y==2008) // Chulalongkorn Day


            //2009

            ||(d==2 & m==January & y==2009) // Special Holiday
            ||(d==9 & m==February & y==2009) // Makha Bucha Day
            ||(d==5 & m==May & y==2009) // Coronation Day
            ||(d==8 & m==May & y==2009) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2009) // Mid Year Closing Day
            ||(d==6 & m==July & y==2009) // Special Holiday
            ||(d==7 & m==July & y==2009) // Asarnha Bucha DayEffective from B.E. 2550 onwards, the Asarnha Bucha Day is prescribed as a holiday, in replacement of the Buddhist Lent Day
            ||(d==23 & m==October & y==2009) // Chulalongkorn Day


            //2010

            ||(d==1 & m==March & y==2010) // Substitution for Makha Bucha Day(Sunday 28 February)
            ||(d==5 & m==May & y==2010) // Coronation Day 
            ||(d==20 & m==May & y==2010) // Special Holiday
            ||(d==21 & m==May & y==2010) // Special Holiday
            ||(d==28 & m==May & y==2010) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2010) // Mid Year Closing Day
            ||(d==26 & m==July & y==2010) // Asarnha Bucha DayEffective from B.E. 2550 onwards, the Asarnha Bucha Day is prescribed as a holiday, in replacement of the Buddhist Lent Day
            ||(d==13 & m==August & y==2010) // Special Holiday
            ||(d==25 & m==October & y==2010) // Substitution for Chulalongkorn Day(Saturday 23 October)


            //2011

            ||(d==18 & m==February & y==2011) // Makha Bucha Day
            ||(d==5 & m==May & y==2011) // Coronation Day 
            ||(d==16 & m==May & y==2011) // Special Holiday
            ||(d==17 & m==May & y==2011) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2011) // Mid Year Closing Day
            ||(d==15 & m==July & y==2011) // Asarnha Bucha DayEffective from B.E. 2550 onwards, the Asarnha Bucha Day is prescribed as a holiday, in replacement of the Buddhist Lent Day
            ||(d==24 & m==October & y==2011) // Substitution for Chulalongkorn Day(Sunday 23 October)


            //2012

            ||(d==3 & m==January & y==2012) // Special Holiday
            ||(d==7 & m==March & y==2012) // Makha Bucha Day 2/
            ||(d==9 & m==April & y==2012) // Special Holiday
            ||(d==7 & m==May & y==2012) // Substitution for Coronation Day(Saturday 5 May) 
            ||(d==4 & m==June & y==2012) // Wisakha Bucha Day
            ||(d==2 & m==August & y==2012) // Asarnha Bucha DayEffective from B.E. 2550 onwards, the Asarnha Bucha Day is prescribed as a holiday, in replacement of the Buddhist Lent Day
            ||(d==23 & m==October & y==2012) // Chulalongkorn Day


            //2013

            ||(d==25 & m==February & y==2013) // Makha Bucha Day
            ||(d==6 & m==May & y==2013) // Substitution for Coronation Day(Sunday 5 May) 
            ||(d==24 & m==May & y==2013) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2013) // Mid Year Closing Day
            ||(d==22 & m==July & y==2013) // Asarnha Bucha Day 2/
            ||(d==23 & m==October & y==2013) // Chulalongkorn Day
            ||(d==30 & m==December & y==2013) // Special Holiday


            //2014

            ||(d==14 & m==February & y==2014) // Makha Bucha Day
            ||(d==5 & m==May & y==2014) // Coronation Day 
            ||(d==13 & m==May & y==2014) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2014) // Mid Year Closing Day
            ||(d==11 & m==July & y==2014) // Asarnha Bucha Day 1/
            ||(d==11 & m==August & y==2014) // Special Holiday
            ||(d==23 & m==October & y==2014) // Chulalongkorn Day


            //2015

            ||(d==2 & m==January & y==2015) // Special Holiday 
            ||(d==4 & m==March & y==2015) // Makha Bucha Day
            ||(d==4 & m==May & y==2015) // Special Holiday
            ||(d==5 & m==May & y==2015) // Coronation Day 
            ||(d==1 & m==June & y==2015) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2015) // Mid Year Closing Day
            ||(d==30 & m==July & y==2015) // Asarnha Bucha Day 1/
            ||(d==23 & m==October & y==2015) // Chulalongkorn Day


            //2016

            ||(d==22 & m==February & y==2016) // Makha Bucha Day
            ||(d==5 & m==May & y==2016) // Coronation Day 
            ||(d==6 & m==May & y==2016) // Special Holiday
            ||(d==20 & m==May & y==2016) // Wisakha Bucha Day
            ||(d==1 & m==July & y==2016) //  Mid Year Closing Day
            ||(d==18 & m==July & y==2016) // Special Holiday ​
            ||(d==19 & m==July & y==2016) // Asarnha Bucha Day 1/
            ||(d==24 & m==October & y==2016) // Substitution for Chulalongkorn Day (Sunday 23rd October) 

            //2018

            ||(d==1 & m==March & y==2018) // Makha Bucha Day
            ||(d==29 & m==May & y==2018) // Wisakha Bucha Day 
            ||(d==27 & m==July & y==2018) // Asarnha Bucha Day1 
            ||(d==23 & m==October & y==2018) // Chulalongkorn Day​
            )
            return false;
        return true;
    }

}

