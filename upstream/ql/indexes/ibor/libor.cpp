/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2005, 2006, 2008 StatPro Italia srl

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

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    namespace {

        BusinessDayConvention liborConvention(const Period& p) {
            switch (p.units()) {
              case Days:
              case Weeks:
                return Following;
              case Months:
              case Years:
                return ModifiedFollowing;
              default:
                QL_FAIL("invalid time units");
            }
        }

        bool liborEOM(const Period& p) {
            switch (p.units()) {
              case Days:
              case Weeks:
                return false;
              case Months:
              case Years:
                return true;
              default:
                QL_FAIL("invalid time units");
            }
        }

    }


    Libor::Libor(const std::string& familyName,
                 const Period& tenor,
                 Natural settlementDays,
                 const Currency& currency,
                 const Calendar& financialCenterCalendar,
                 const DayCounter& dayCounter,
                 const Handle<YieldTermStructure>& h)
    : IborIndex(familyName, tenor, settlementDays, currency,
                // http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1412 :
                // UnitedKingdom::Exchange is the fixing calendar for
                // a) all currencies but EUR
                // b) all indexes but o/n and s/n
                UnitedKingdom(UnitedKingdom::Exchange),
                liborConvention(tenor), liborEOM(tenor),
                dayCounter, h),
      financialCenterCalendar_(financialCenterCalendar),
      jointCalendar_(JointCalendar(UnitedKingdom(UnitedKingdom::Exchange),
                                   financialCenterCalendar,
                                   JoinHolidays)) {
        QL_REQUIRE(this->tenor().units()!=Days,
                   "for daily tenors (" << this->tenor() <<
                   ") dedicated DailyTenor constructor must be used");
        QL_REQUIRE(currency!=EURCurrency(),
                   "for EUR Libor dedicated EurLibor constructor must be used");
    }

    Date Libor::valueDate(const Date& fixingDate) const {

        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");

        // http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1412 :
        // For all currencies other than EUR and GBP the period between
        // Fixing Date and Value Date will be two London business days
        // after the Fixing Date, or if that day is not both a London
        // business day and a business day in the principal financial centre
        // of the currency concerned, the next following day which is a
        // business day in both centres shall be the Value Date.
        Date d = fixingCalendar().advance(fixingDate, fixingDays_, Days);
        return jointCalendar_.adjust(d);
    }

    Date Libor::maturityDate(const Date& valueDate) const {
        // Where a deposit is made on the final business day of a
        // particular calendar month, the maturity of the deposit shall
        // be on the final business day of the month in which it matures
        // (not the corresponding date in the month of maturity). Or in
        // other words, in line with market convention, BBA LIBOR rates
        // are dealt on an end-end basis. For instance a one month
        // deposit for value 28th February would mature on 31st March,
        // not the 28th of March.
        return jointCalendar_.advance(valueDate, tenor_, convention_,
                                                         endOfMonth());
    }

    Calendar Libor::jointCalendar() const {
        return jointCalendar_;
    }

    ext::shared_ptr<IborIndex> Libor::clone(
                                  const Handle<YieldTermStructure>& h) const {
        return ext::make_shared<Libor>(familyName(),
                                       tenor(),
                                       fixingDays(),
                                       currency(),
                                       financialCenterCalendar_,
                                       dayCounter(),
                                       h);
    }


    DailyTenorLibor::DailyTenorLibor(
                 const std::string& familyName,
                 Natural settlementDays,
                 const Currency& currency,
                 const Calendar& financialCenterCalendar,
                 const DayCounter& dayCounter,
                 const Handle<YieldTermStructure>& h)
    : IborIndex(familyName, 1*Days, settlementDays, currency,
                // http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1412 :
                // no o/n or s/n fixings (as the case may be) will take place
                // when the principal centre of the currency concerned is
                // closed but London is open on the fixing day.
                JointCalendar(UnitedKingdom(UnitedKingdom::Exchange),
                              financialCenterCalendar,
                              JoinHolidays),
                liborConvention(1*Days), liborEOM(1*Days),
                dayCounter, h) {
        QL_REQUIRE(currency!=EURCurrency(),
                   "for EUR Libor dedicated EurLibor constructor must be used");
    }

}
