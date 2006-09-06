/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/period.hpp>

namespace QuantLib {

    Period::Period(Frequency f) {
        switch (f) {
          case Once:
            QL_FAIL("cannot instantiate a Period from frequency " << f);
          case NoFrequency:
            // same as Period()
            units_ = Days;
            length_ = 0;
            break;
          case Annual:
            units_ = Years;
            length_ = 1;
            break;
          case Semiannual:
          case EveryFourthMonth:
          case Quarterly:
          case Bimonthly:
          case Monthly:
            units_ = Months;
            length_ = 12/f;
            break;
          case EveryFourthWeek:
          case EveryThirdWeek:
          case Biweekly:
          case Weekly:
            units_ = Weeks;
            length_ = 52/f; // it does work for EveryThirdWeek too
            break;
          case Daily:
            units_ = Days;
            length_ = 1;
            break;
          default:
            QL_FAIL("unknown frequency (" << Integer(f));
        }
    }

    Frequency Period::frequency() const
    {
        // unsigned version
        Size length = std::abs(length_);

        if (length==0) return NoFrequency;

        switch (units_) {
          case Years:
            QL_REQUIRE(length==1,
                       "cannot instantiate a Frequency from " << *this);
            return Annual;
          case Months:
            QL_REQUIRE((12%length)==0 && length<=12,
                       "cannot instantiate a Frequency from " << *this);
            return Frequency(12/length);
          case Weeks:
              if (length==1)
                  return Weekly;
              else if (length==2)
                  return Biweekly;
              else if (length==3)
                  return EveryThirdWeek;
              else if (length==4)
                  return EveryFourthWeek;
              else
                  QL_FAIL("cannot instantiate a Frequency from " << *this);
          case Days:
            QL_REQUIRE(length==1,
                       "cannot instantiate a Frequency from " << *this);
            return Daily;
          default:
            QL_FAIL("unknown time unit (" << Integer(units_));
        }
    }

    bool operator<(const Period& p1, const Period& p2) {
        if (p1.units() == p2.units())
            return (p1.length() < p2.length());
        if (p1.units() == Days) {
            if (p2.units() == Weeks)
                return (p1.length() < p2.length() * 7);
            else if (p2.units() == Years)
                return (p1.length() < p2.length() * 365);
            else if (p2.units() == Months)
                QL_FAIL("undecidable comparison between days and months");
            else
                QL_FAIL("unknown units");
        } else if (p1.units() == Weeks) {
            if (p2.units() == Days)
                return (p1.length() * 7 < p2.length());
            else if (p2.units() == Months || p2.units() == Years)
                QL_FAIL("undecidable comparison between "
                        "weeks and months/years");
            else
                QL_FAIL("unknown units");
        } else if (p1.units() == Months) {
            if (p2.units() == Years)
                return (p1.length() < p2.length() * 12);
            else if (p2.units() == Days || p2.units() == Weeks)
                QL_FAIL("undecidable comparison between "
                            "months and days/weeks");
            else
                QL_FAIL("unknown units");
        } else if (p1.units() == Years) {
            if (p2.units() == Days)
                return (p1.length() * 365 < p2.length());
            else if (p2.units() == Months)
                return (p1.length() * 12 < p2.length());
            else if (p2.units() == Weeks)
                QL_FAIL("undecidable outcome comparing years and weeks");
            else
                QL_FAIL("unknown units");
        } else {
            QL_FAIL("unknown units");
        }
    }

    // period formatting

    std::ostream& operator<<(std::ostream& out, const Period& p) {
        return out << io::short_period(p);
    }

    namespace detail {

        std::ostream& operator<<(std::ostream& out,
                                 const long_period_holder& holder) {
            Integer n = holder.p.length();
            out << n << " ";
            switch (holder.p.units()) {
              case Days:
                return out << (n == 1 ? "day" : "days");
              case Weeks:
                return out << (n == 1 ? "week" : "weeks");
              case Months:
                return out << (n == 1 ? "month" : "months");
              case Years:
                return out << (n == 1 ? "year" : "years");
              default:
                QL_FAIL("unknown time unit");
            }
        }

        std::ostream& operator<<(std::ostream& out,
                                 const short_period_holder& holder) {
            Integer n = holder.p.length();
            out << n;
            switch (holder.p.units()) {
              case Days:
                return out << "D";
              case Weeks:
                return out << "W";
              case Months:
                return out << "M";
              case Years:
                return out << "Y";
              default:
                QL_FAIL("unknown time unit");
            }
        }

    }

    namespace io {

        detail::long_period_holder long_period(const Period& p) {
            return detail::long_period_holder(p);
        }

        detail::short_period_holder short_period(const Period& p) {
            return detail::short_period_holder(p);
        }

    }


   // frequency formatting

    std::ostream& operator<<(std::ostream& out, Frequency f) {
        switch (f) {
          case NoFrequency:
            return out << "no-frequency";
          case Once:
            return out << "once";
          case Annual:
            return out << "annual";
          case Semiannual:
            return out << "semiannual";
          case EveryFourthMonth:
            return out << "every-fourth-month";
          case Quarterly:
            return out << "quarterly";
          case Bimonthly:
            return out << "bimonthly";
          case Monthly:
            return out << "monthly";
          case EveryFourthWeek:
            return out << "every-fourth-week";
          case EveryThirdWeek:
            return out << "every-third-week";
          case Biweekly:
            return out << "biweekly";
          case Weekly:
            return out << "weekly";
          case Daily:
            return out << "daily";
          default:
            QL_FAIL("unknown frequency (" << Integer(f) << ")");
        }
    }

}
