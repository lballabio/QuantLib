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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/time/ecb.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cctype>

using std::string;

namespace QuantLib {

    namespace {
        // case-insensitive comparison. answers: lhs < rhs.
        struct is_iless {
          bool operator()(const boost::string_view lhs,
                          const boost::string_view rhs) const {
            char lhsUpper[3];
            char rhsUpper[3];
            for (int i = 0; i < 3; ++i) {
              lhsUpper[i] = std::toupper(lhs[i]);
              rhsUpper[i] = std::toupper(rhs[i]);
            }
            return boost::string_view(lhsUpper, 3) <
                   boost::string_view(rhsUpper, 3);
          }
        };

        using MonthBimap_t = boost::bimaps::bimap<
            boost::bimaps::set_of<boost::string_view, is_iless>, Month>;

        // bimap: generalization of map. can be queried by string_view or Month.
        const MonthBimap_t MONTHS = []() {
          MonthBimap_t months;
          months.insert({"JAN", January});
          months.insert({"FEB", February});
          months.insert({"MAR", March});
          months.insert({"APR", April});
          months.insert({"MAY", May});
          months.insert({"JUN", June});
          months.insert({"JUL", July});
          months.insert({"AUG", August});
          months.insert({"SEP", September});
          months.insert({"OCT", October});
          months.insert({"NOV", November});
          months.insert({"DEC", December});
          return months;
        }();

        //clang-format off
        // Start of maintenance period
        // source: https://web.archive.org/web/20230610050642/https://www.ecb.europa.eu/press/calendars/reserve/html/index.en.html
        std::set<Date> ecbKnownDateSet = {
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
            Date(43040), Date(43089),

            // 2018
            Date(43131), Date(43167), Date(43216), Date(43265), Date(43307), Date(43356),
            Date(43398), Date(43447),

            // 2019
            Date(43495), Date(43537), Date(43572), Date(43628), Date(43677), Date(43726),
            Date(43768), Date(43817),

            // 2020
            Date(43859), Date(43908), Date(43957), Date(43992), Date(44034), Date(44090),
            Date(44139), Date(44181),

            // 2021
            Date(44223), Date(44272), Date(44314), Date(44363), Date(44405), Date(44454),
            Date(44503), Date(44552),

            // 2022
            Date(44601), Date(44636), Date(44671), Date(44727), Date(44769), Date(44818),
            Date(44867), Date(44916),

            // 2023
            Date(44965), Date(45007), Date(45056), Date(45098), Date(45140), Date(45189),
            Date(45231), Date(45280),

            // 2024
            Date(45322), Date(45364), Date(45399), Date(45455), Date(45497), Date(45553),
            Date(45588), Date(45644)
        };
        //clang-format on
    }

    const std::set<Date>& ECB::knownDates() {
        return ecbKnownDateSet;
    }

    void ECB::addDate(const Date& d) {
        ecbKnownDateSet.insert(d);
    }

    void ECB::removeDate(const Date& d) {
        ecbKnownDateSet.erase(d);
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

        // convert first 3 characters to `Month m`
        const boost::string_view monthCode(ecbCode.data(), 3);
        const Month m = MONTHS.left.at(monthCode);

        // convert 4th, 5th characters to `Year y`
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

        // 3 characters for the month
        const boost::string_view month = MONTHS.right.at(ecbDate.month());

        // last two digits of the year
        const unsigned int y = ecbDate.year() % 100;

        // c-style string. length: 6 == (3 for month + 2 for year + 1 for terminating null)
        char ECBcode[6];
        std::snprintf(ECBcode, 6, "%3s%02u", month.data(), y);

        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_ENSURE(isECBcode(ECBcode),
                  "the result " << ECBcode <<
                  " is an invalid ECB code");
        #endif
        return ECBcode;
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

        // first 3 characters need to represent month, case insensitive
        {
            const boost::string_view month(ecbCode.data(), 3);
            if (MONTHS.left.find(month) == MONTHS.left.end())
                return false;
        }

        // 4th, 5th characters need to be digit
        return (std::isdigit(static_cast<unsigned char>(ecbCode[3])) != 0)
            && (std::isdigit(static_cast<unsigned char>(ecbCode[4])) != 0);
    }

    string ECB::nextCode(const std::string& ecbCode) {
        QL_REQUIRE(isECBcode(ecbCode),
                   ecbCode << " is not a valid ECB code");

        const boost::string_view month(ecbCode.data(), 3);
        const Month monthEnum = MONTHS.left.at(month);

        string nextCodeStr;
        nextCodeStr.reserve(5);
        if (monthEnum != December) {
            // use next month
            const auto nextMonthEnum = static_cast<Month>(monthEnum + 1);
            const boost::string_view nextMonth = MONTHS.right.at(nextMonthEnum);
            nextCodeStr.append(nextMonth.data(), 3);

            // copy year
            nextCodeStr += {ecbCode[3], ecbCode[4]};
        } else {
            // previous month was DEC
            nextCodeStr.append("JAN");

            // init with previous year
            nextCodeStr += { ecbCode[3], ecbCode[4] };

            // increment year's last digit (e.g. '22' -> '23').
            // if overflow (e.g. '29' -> '20'), then also increment 2nd digit (e.g. '20' -> '30').
            const auto incrementAndCheckForOverlow = [](char& dig) -> bool {
                if (dig == '9') {
                    dig = '0';
                    return true;
                } else {
                    ++dig;
                    return false;
                }
            };
            if (incrementAndCheckForOverlow(nextCodeStr[4]))
                incrementAndCheckForOverlow(nextCodeStr[3]);
        }

        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_ENSURE(isECBcode(nextCodeStr),
                  "the result " << nextCodeStr <<
                  " is an invalid ECB code");
        #endif
        return nextCodeStr;
    }

}
