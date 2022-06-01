/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

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

#include <ql/time/ecb.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <algorithm>
#include <string>

using boost::algorithm::to_upper_copy;
using std::string;

namespace QuantLib {

    static std::set<Date> knownDateSet;

    const std::set<Date>& ECB::knownDates() {

        // one-off inizialization
        static const Date::serial_type knownDatesArray[] = {
              38371, 38391, 38420, 38455, 38483, 38511, 38546, 38574, 38602, 38637, 38665, 38692 // 2005
            , 38735, 38756, 38784, 38819, 38847, 38883, 38910, 38938, 38966, 39001, 39029, 39064 // 2006
            , 39099, 39127, 39155, 39190, 39217, 39246, 39274, 39302, 39337, 39365, 39400, 39428 // 2007
            , 39463, 39491, 39519, 39554, 39582, 39610, 39638, 39673, 39701, 39729, 39764, 39792 // 2008
            , 39834, 39855, 39883, 39911, 39946, 39974, 40002, 40037, 40065, 40100, 40128, 40155 // 2009
            , 40198, 40219, 40247, 40282, 40310, 40345, 40373, 40401, 40429, 40464, 40492, 40520 // 2010
            , 40562, 40583, 40611, 40646, 40674, 40709, 40737, 40765, 40800, 40828, 40856, 40891 // 2011
            // http://www.ecb.europa.eu/press/pr/date/2011/html/pr110520.en.html
            , 40926, 40954, 40982, 41010, 41038, 41073, 41101, 41129, 41164, 41192, 41227, 41255 // 2012
            , 41290, 41318, 41346, 41374, 41402, 41437, 41465, 41493, 41528, 41556, 41591, 41619 // 2013
            // http://www.ecb.europa.eu/press/pr/date/2013/html/pr130610.en.html
            , 41654, 41682, 41710, 41738, 41773, 41801, 41829, 41864, 41892, 41920, 41955, 41983 // 2014
            // http://www.ecb.europa.eu/press/pr/date/2014/html/pr140717_1.en.html
            , 42032, 42074, 42116, 42165, 42207, 42256, 42305, 42347// 2015
            // https://www.ecb.europa.eu/press/pr/date/2015/html/pr150622.en.html
            , 42396, 42445, 42487, 42529, 42578, 42627, 42669, 42718 // 2016
            // https://www.ecb.europa.eu/press/calendars/reserve/html/index.en.html
            , 42760, 42809, 42858, 42900, 42942, 42991, 43040, 43089 //2017
        };
        if (knownDateSet.empty()) {
            Size n = sizeof(knownDatesArray)/sizeof(Date::serial_type);
            for (Size i=0; i<n; ++i)
                knownDateSet.insert(Date(knownDatesArray[i]));
        }

        return knownDateSet;
    }

    void ECB::addDate(const Date& d) {
        knownDates(); // just to ensure inizialization
        knownDateSet.insert(d);
    }

    void ECB::removeDate(const Date& d) {
        knownDates(); // just to ensure inizialization
        knownDateSet.erase(d);
    }

    Date ECB::date(const string& ecbCode,
                   const Date& refDate) {

        QL_REQUIRE(isECBcode(ecbCode),
                   ecbCode << " is not a valid ECB code");

        string code = to_upper_copy(ecbCode);
        string monthString = code.substr(0, 3);
        Month m;
        if (monthString=="JAN")      m = January;
        else if (monthString=="FEB") m = February;
        else if (monthString=="MAR") m = March;
        else if (monthString=="APR") m = April;
        else if (monthString=="MAY") m = May;
        else if (monthString=="JUN") m = June;
        else if (monthString=="JUL") m = July;
        else if (monthString=="AUG") m = August;
        else if (monthString=="SEP") m = September;
        else if (monthString=="OCT") m = October;
        else if (monthString=="NOV") m = November;
        else if (monthString=="DEC") m = December;
        else QL_FAIL("not an ECB month (and it should have been)");

        Year y = std::stoi(code.substr(3, 2));
        Date referenceDate = (refDate != Date() ?
                              refDate :
                              Date(Settings::instance().evaluationDate()));
        Year referenceYear = (referenceDate.year() % 100);
        y += referenceDate.year() - referenceYear;
        if (y<Date::minDate().year())
            return ECB::nextDate(Date::minDate());

        return ECB::nextDate(Date(1, m, y) - 1);
    }

    string ECB::code(const Date& ecbDate) {

        QL_REQUIRE(isECBdate(ecbDate),
                   ecbDate << " is not a valid ECB date");

        std::ostringstream ECBcode;
        unsigned int y = ecbDate.year() % 100;
        string padding;
        if (y < 10)
            padding = "0";
        switch(ecbDate.month()) {
          case January:
            ECBcode << "JAN" << padding << y;
            break;
          case February:
            ECBcode << "FEB" << padding << y;
            break;
          case March:
            ECBcode << "MAR" << padding << y;
            break;
          case April:
            ECBcode << "APR" << padding << y;
            break;
          case May:
            ECBcode << "MAY" << padding << y;
            break;
          case June:
            ECBcode << "JUN" << padding << y;
            break;
          case July:
            ECBcode << "JUL" << padding << y;
            break;
          case August:
            ECBcode << "AUG" << padding << y;
            break;
          case September:
            ECBcode << "SEP" << padding << y;
            break;
          case October:
            ECBcode << "OCT" << padding << y;
            break;
          case November:
            ECBcode << "NOV" << padding << y;
            break;
          case December:
            ECBcode << "DEC" << padding << y;
            break;
          default:
            QL_FAIL("not an ECB month (and it should have been)");
        }

        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_ENSURE(isECBcode(ECBcode.str()),
                  "the result " << ECBcode.str() <<
                  " is an invalid ECB code");
        #endif
        return ECBcode.str();
    }



    Date ECB::nextDate(const Date& date) {
        Date d = (date == Date() ?
                  Settings::instance().evaluationDate() :
                  date);

        auto i = std::upper_bound(knownDates().begin(), knownDates().end(), d);

        QL_REQUIRE(i!=knownDates().end(),
                   "ECB dates after " << *(--knownDates().end()) << " are unknown");
        return *i;
    }

    std::vector<Date> ECB::nextDates(const Date& date) {
        Date d = (date == Date() ?
                  Settings::instance().evaluationDate() :
                  date);

        auto i = std::upper_bound(knownDates().begin(), knownDates().end(), d);

        QL_REQUIRE(i!=knownDates().end(),
                   "ECB dates after " << *knownDates().end() << " are unknown");
        return std::vector<Date>(i, knownDates().end());
    }


    bool ECB::isECBcode(const std::string& ecbCode) {

        if (ecbCode.length() != 5)
            return false;

        string code = to_upper_copy(ecbCode);

        string str1("0123456789");
        string::size_type loc = str1.find(code.substr(3, 1), 0);
        if (loc == string::npos)
            return false;
        loc = str1.find(code.substr(4, 1), 0);
        if (loc == string::npos)
            return false;

        string monthString = code.substr(0, 3);
        if (monthString=="JAN")      return true;
        else if (monthString=="FEB") return true;
        else if (monthString=="MAR") return true;
        else if (monthString=="APR") return true;
        else if (monthString=="MAY") return true;
        else if (monthString=="JUN") return true;
        else if (monthString=="JUL") return true;
        else if (monthString=="AUG") return true;
        else if (monthString=="SEP") return true;
        else if (monthString=="OCT") return true;
        else if (monthString=="NOV") return true;
        else if (monthString=="DEC") return true;
        else return false;
    }

    string ECB::nextCode(const std::string& ecbCode) {
        QL_REQUIRE(isECBcode(ecbCode),
                   ecbCode << " is not a valid ECB code");

        string code = to_upper_copy(ecbCode);
        std::ostringstream result;

        string monthString = code.substr(0, 3);
        if (monthString=="JAN")      result << "FEB" << code.substr(3, 2);
        else if (monthString=="FEB") result << "MAR" << code.substr(3, 2);
        else if (monthString=="MAR") result << "APR" << code.substr(3, 2);
        else if (monthString=="APR") result << "MAY" << code.substr(3, 2);
        else if (monthString=="MAY") result << "JUN" << code.substr(3, 2);
        else if (monthString=="JUN") result << "JUL" << code.substr(3, 2);
        else if (monthString=="JUL") result << "AUG" << code.substr(3, 2);
        else if (monthString=="AUG") result << "SEP" << code.substr(3, 2);
        else if (monthString=="SEP") result << "OCT" << code.substr(3, 2);
        else if (monthString=="OCT") result << "NOV" << code.substr(3, 2);
        else if (monthString=="NOV") result << "DEC" << code.substr(3, 2);
        else if (monthString=="DEC") {
            unsigned int y = (std::stoi(code.substr(3, 2)) + 1) % 100;
            string padding;
            if (y < 10)
                padding = "0";

            result << "JAN" << padding << y;
        } else QL_FAIL("not an ECB month (and it should have been)");


        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_ENSURE(isECBcode(result.str()),
                  "the result " << result.str() <<
                  " is an invalid ECB code");
        #endif
        return result.str();
    }

}
