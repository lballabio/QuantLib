/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2017 Peter Caspers
 Copyright (C) 2017 Oleg Kulkov

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

/*! \file unitedstates.hpp
    \brief US calendars
*/

#ifndef quantlib_united_states_calendar_hpp
#define quantlib_united_states_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! United States calendars
    /*! Public holidays (see https://www.opm.gov/policy-data-oversight/pay-leave/federal-holidays):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday if
            actually on Sunday, or to Friday if on Saturday)</li>
        <li>Martin Luther King's birthday, third Monday in January (since
            1983)</li>
        <li>Presidents' Day (a.k.a. Washington's birthday),
            third Monday in February</li>
        <li>Memorial Day, last Monday in May</li>
        <li>Juneteenth, June 19th (moved to Monday if Sunday or
            Friday if Saturday)</li>
        <li>Independence Day, July 4th (moved to Monday if Sunday or
            Friday if Saturday)</li>
        <li>Labor Day, first Monday in September</li>
        <li>Columbus Day, second Monday in October</li>
        <li>Veterans' Day, November 11th (moved to Monday if Sunday or
            Friday if Saturday)</li>
        <li>Thanksgiving Day, fourth Thursday in November</li>
        <li>Christmas, December 25th (moved to Monday if Sunday or Friday
            if Saturday)</li>
        </ul>

        Note that since 2015 Independence Day only impacts Libor if it
        falls on a  weekday (see <https://www.theice.com/iba/libor>,
        <https://www.theice.com/marketdata/reports/170> and
        <https://www.theice.com/publicdocs/LIBOR_Holiday_Calendar_2015.pdf>
        for the fixing and value date calendars).

        Holidays for the stock exchange (data from http://www.nyse.com):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday if
            actually on Sunday)</li>
        <li>Martin Luther King's birthday, third Monday in January (since
            1998)</li>
        <li>Presidents' Day (a.k.a. Washington's birthday),
            third Monday in February</li>
        <li>Good Friday</li>
        <li>Memorial Day, last Monday in May</li>
        <li>Independence Day, July 4th (moved to Monday if Sunday or
            Friday if Saturday)</li>
        <li>Labor Day, first Monday in September</li>
        <li>Thanksgiving Day, fourth Thursday in November</li>
        <li>Presidential election day, first Tuesday in November of election
            years (until 1980)</li>
        <li>Christmas, December 25th (moved to Monday if Sunday or Friday
            if Saturday)</li>
        <li>Special historic closings (see
            http://www.nyse.com/pdfs/closings.pdf)</li>
        </ul>

        Holidays for the government bond market (data from
        http://www.bondmarkets.com):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday if
            actually on Sunday)</li>
        <li>Martin Luther King's birthday, third Monday in January (since
            1983)</li>
        <li>Presidents' Day (a.k.a. Washington's birthday),
            third Monday in February</li>
        <li>Good Friday</li>
        <li>Memorial Day, last Monday in May</li>
        <li>Independence Day, July 4th (moved to Monday if Sunday or
            Friday if Saturday)</li>
        <li>Labor Day, first Monday in September</li>
        <li>Columbus Day, second Monday in October</li>
        <li>Veterans' Day, November 11th (moved to Monday if Sunday or
            Friday if Saturday)</li>
        <li>Thanksgiving Day, fourth Thursday in November</li>
        <li>Christmas, December 25th (moved to Monday if Sunday or Friday
            if Saturday)</li>
        </ul>

        Holidays for the North American Energy Reliability Council
        (data from http://www.nerc.com/~oc/offpeaks.html):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday if
            actually on Sunday)</li>
        <li>Memorial Day, last Monday in May</li>
        <li>Independence Day, July 4th (moved to Monday if Sunday)</li>
        <li>Labor Day, first Monday in September</li>
        <li>Thanksgiving Day, fourth Thursday in November</li>
        <li>Christmas, December 25th (moved to Monday if Sunday)</li>
        </ul>

        \ingroup calendars

        \test the correctness of the returned results is tested
              against a list of known holidays.
    */
    class UnitedStates : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "US settlement"; }
            bool isBusinessDay(const Date&) const override;
        };
        class LiborImpactImpl : public SettlementImpl {
          public:
            std::string name() const override { return "US with Libor impact"; }
            bool isBusinessDay(const Date&) const override;
        };
        class NyseImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "New York stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
        class GovernmentBondImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "US government bond market"; }
            bool isBusinessDay(const Date&) const override;
        };
        class NercImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override {
                return "North American Energy Reliability Council";
            }
            bool isBusinessDay(const Date&) const override;
        };
        class FederalReserveImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Federal Reserve Bankwire System"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        //! US calendars
        enum Market { Settlement,     //!< generic settlement calendar
                      NYSE,           //!< New York stock exchange calendar
                      GovernmentBond, //!< government-bond calendar
                      NERC,           //!< off-peak days for NERC
                      LiborImpact,    //!< Libor impact calendar
                      FederalReserve  //!< Federal Reserve Bankwire System
        };

        explicit UnitedStates(Market market);

        /*! \deprecated Use the other constructor.
                        Deprecated in version 1.24.
        */
        QL_DEPRECATED
        UnitedStates()
        : UnitedStates(Settlement) {}
    };

}


#endif


#ifndef id_b2985e0e2c0a01f7b747b6db1540be10
#define id_b2985e0e2c0a01f7b747b6db1540be10
inline bool test_b2985e0e2c0a01f7b747b6db1540be10(int* i) { return i != 0; }
#endif
