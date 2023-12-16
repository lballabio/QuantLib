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
#include <boost/utility/string_view.hpp>
#include <algorithm>
#include <array>
#include <string>

using boost::algorithm::to_upper_copy;
using std::string;

namespace QuantLib {

    namespace {
        const std::array<boost::string_view, 12> MONTHS{
            "JAN" ,"FEB" ,"MAR" ,"APR" ,"MAY" ,"JUN",
            "JUL" ,"AUG" ,"SEP" ,"OCT" ,"NOV" ,"DEC"};
    }

    namespace detail {
        //clang-format off
        // Start of maintenance period
        // source: https://web.archive.org/web/20230610050642/https://www.ecb.europa.eu/press/calendars/reserve/html/index.en.html
        static std::set<Date> ecbKnownDateSet = {
            // 2005
            Date(38371), Date(38391), Date(38420), Date(38455), Date(38483), Date(38511),
            Date(38546), Date(38574), Date(38602), Date(38637), Date(38665), Date(38692),

            // 2006
            Date(38735), Date(38756), Date(38784), Date(38819), Date(38847), Date(38883),
            Date(38910), Date(38938), Date(38966), Date(39001), Date(39029), Date(39064),

            // 2007
            Date(39099), Date(39127), Date(39155), Date(39190), Date(39217), Date(39246),
            Date(39274), Date(39302), Date(39337), Date(39365), Date(39400), Date(39428),

            // 2008
            Date(39463), Date(39491), Date(39519), Date(39554), Date(39582), Date(39610),
            Date(39638), Date(39673), Date(39701), Date(39729), Date(39764), Date(39792),

            // 2009
            Date(39834), Date(39855), Date(39883), Date(39911), Date(39946), Date(39974),
            Date(40002), Date(40037), Date(40065), Date(40100), Date(40128), Date(40155),

            // 2010
            Date(40198), Date(40219), Date(40247), Date(40282), Date(40310), Date(40345),
            Date(40373), Date(40401), Date(40429), Date(40464), Date(40492), Date(40520),

            // 2011
            Date(40562), Date(40583), Date(40611), Date(40646), Date(40674), Date(40709),
            Date(40737), Date(40765), Date(40800), Date(40828), Date(40856), Date(40891),

            // 2012
            Date(40926), Date(40954), Date(40982), Date(41010), Date(41038), Date(41073),
            Date(41101), Date(41129), Date(41164), Date(41192), Date(41227), Date(41255),

            // 2013
            Date(41290), Date(41318), Date(41346), Date(41374), Date(41402), Date(41437),
            Date(41465), Date(41493), Date(41528), Date(41556), Date(41591), Date(41619),

            // 2014
            Date(41654), Date(41682), Date(41710), Date(41738), Date(41773), Date(41801),
            Date(41829), Date(41864), Date(41892), Date(41920), Date(41955), Date(41983),

            // 2015
            Date(42032), Date(42074), Date(42116), Date(42165), Date(42207), Date(42256),
            Date(42305), Date(42347),

            // 2016
            Date(42396), Date(42445), Date(42487), Date(42529), Date(42578), Date(42627),
            Date(42669), Date(42718),

            // 2017
            Date(42760), Date(42809), Date(42858), Date(42900), Date(42942), Date(42991),
            Date(43040), Date(43089)
        };
        //clang-format on
    }

    const std::set<Date>& ECB::knownDates() {
        return detail::ecbKnownDateSet;
    }

    void ECB::addDate(const Date& d) {
        detail::ecbKnownDateSet.insert(d);
    }

    void ECB::removeDate(const Date& d) {
        detail::ecbKnownDateSet.erase(d);
    }

    namespace {
        int ToInteger(const char c) {
            const int i = static_cast<int>(c) - static_cast<int>('0');
            QL_ASSERT((i >= 0) && (i <= 9), "Character does not represent a digit. char: " << c);
            return i;
        }
    }

    Date ECB::date(const string& ecbCode,
                   const Date& refDate) {

        QL_REQUIRE(isECBcode(ecbCode),
                   ecbCode << " is not a valid ECB code");

        std::array<char, 3> upperMonthCode;
        for (int i=0; i<3; ++i)
            upperMonthCode[i] = std::toupper(ecbCode[i]);
        const boost::string_view monthString(upperMonthCode.data(), 3);
        const auto it = std::find(MONTHS.begin(), MONTHS.end(), monthString);
        QL_ASSERT(it != MONTHS.end() ,"not an ECB month (and it should have been). code: " + ecbCode);

        // QuantLib::Month is 1-based!
        const Month m = static_cast<QuantLib::Month>(std::distance(MONTHS.begin(), it) + 1);

        Year y = ToInteger(ecbCode[3])*10 + ToInteger(ecbCode[4]);
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

        QL_REQUIRE(i != knownDates().end(),
                   "ECB dates after " << *knownDates().rbegin() << " are unknown");
        return *i;
    }

    std::vector<Date> ECB::nextDates(const Date& date) {
        Date d = (date == Date() ?
                  Settings::instance().evaluationDate() :
                  date);

        auto i = std::upper_bound(knownDates().begin(), knownDates().end(), d);

        QL_REQUIRE(i != knownDates().end(),
                   "ECB dates after " << *knownDates().rbegin() << " are unknown");
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
