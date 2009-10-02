/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano

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
#include <algorithm>

namespace QuantLib {

    static std::set<Date> knownDateSet;

    const std::set<Date>& ECB::knownDates() {

        // one-off inizialization
        static const BigInteger knownDatesArray[] = {
              38371, 38391, 38420, 38455, 38483, 38511, 38546, 38574, 38602, 38637, 38665, 38692 // 2005
            , 38735, 38756, 38784, 38819, 38847, 38883, 38910, 38938, 38966, 39001, 39029, 39064 // 2006
            , 39099, 39127, 39155, 39190, 39217, 39246, 39274, 39302, 39337, 39365, 39400, 39428 // 2007
            , 39463, 39491, 39519, 39554, 39582, 39610, 39638, 39673, 39701, 39729, 39764, 39792 // 2008
            , 39834, 39855, 39883, 39911, 39946, 39974, 40002, 40037, 40065, 40100, 40128, 40155 // 2009
            , 40198, 40219, 40247, 40282, 40310, 40345, 40373, 40401, 40429, 40464, 40492, 40520 // 2010
            , 40562, 40583, 40611, 40646, 40674, 40709, 40737, 40765, 40800, 40828, 40856, 40891 // 2011
        };
        if (knownDateSet.empty()) {
            Size n = sizeof(knownDatesArray)/sizeof(BigInteger);
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

    Date ECB::nextDate(const Date& date) {
        Date d = (date == Date() ?
                  Settings::instance().evaluationDate() :
                  date);

        std::set<Date>::const_iterator i =
            std::upper_bound(knownDates().begin(), knownDates().end(), d);

        QL_REQUIRE(i!=knownDates().end(),
                   "ECB dates after " << *knownDates().end() << " are unknown");
        return Date(*i);
    }

    std::vector<Date> ECB::nextDates(const Date& date) {
        Date d = (date == Date() ?
                  Settings::instance().evaluationDate() :
                  date);

        std::set<Date>::const_iterator i =
            std::upper_bound(knownDates().begin(), knownDates().end(), d);

        QL_REQUIRE(i!=knownDates().end(),
                   "ECB dates after " << *knownDates().end() << " are unknown");
        return std::vector<Date>(i, knownDates().end());
    }

}
