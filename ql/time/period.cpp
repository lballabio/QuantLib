/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007, 2008, 2014 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2008 StatPro Italia srl
 Copyright (C) 2014 Paolo Mazzocchi

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

#include <ql/time/period.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Period::Period(Frequency f) {
        switch (f) {
          case NoFrequency:
            // same as Period()
            units_ = Days;
            length_ = 0;
            break;
          case Once:
            units_ = Years;
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
          case Biweekly:
          case Weekly:
            units_ = Weeks;
            length_ = 52/f;
            break;
          case Daily:
            units_ = Days;
            length_ = 1;
            break;
          case OtherFrequency:
            QL_FAIL("unknown frequency");  // no point in showing 999...
          default:
            QL_FAIL("unknown frequency (" << Integer(f) << ")");
        }
    }

    Frequency Period::frequency() const {
        // unsigned version
        Size length = std::abs(length_);

        if (length==0) {
            if (units_==Years) return Once;
            return NoFrequency;
        }

        switch (units_) {
          case Years:
            if (length == 1)
                return Annual;
            else
                return OtherFrequency;
          case Months:
            if (12%length == 0 && length <= 12)
                return Frequency(12/length);
            else
                return OtherFrequency;
          case Weeks:
            if (length==1)
                return Weekly;
            else if (length==2)
                return Biweekly;
            else if (length==4)
                return EveryFourthWeek;
            else
                return OtherFrequency;
          case Days:
            if (length==1)
                return Daily;
            else
                return OtherFrequency;
          default:
            QL_FAIL("unknown time unit (" << Integer(units_) << ")");
        }
    }

    void Period::normalize() {
        if (length_ == 0) {
            units_ = Days;
        } else {
            switch (units_) {
              case Months:
                if ((length_ % 12) == 0) {
                    length_ /= 12;
                    units_ = Years;
                }
                break;
              case Days:
              case Weeks:
              case Years:
                break;
              default:
                QL_FAIL("unknown time unit (" << Integer(units_) << ")");
            }
        }
    }

    Period& Period::operator+=(const Period& p) {

        if (length_==0) {
            length_ = p.length();
            units_ = p.units();
        } else if (units_==p.units()) {
            // no conversion needed
            length_ += p.length();
        } else {
            switch (units_) {

              case Years:
                switch (p.units()) {
                  case Months:
                    units_ = Months;
                    length_ = length_*12 + p.length();
                    break;
                  case Weeks:
                  case Days:
                    QL_REQUIRE(p.length()==0,
                               "impossible addition between " << *this <<
                               " and " << p);
                    break;
                  default:
                    QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
                }
                break;

              case Months:
                switch (p.units()) {
                  case Years:
                    length_ += p.length()*12;
                    break;
                  case Weeks:
                  case Days:
                    QL_REQUIRE(p.length()==0,
                               "impossible addition between " << *this <<
                               " and " << p);
                    break;
                  default:
                    QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
                }
                break;

              case Weeks:
                switch (p.units()) {
                  case Days:
                    units_ = Days;
                    length_ = length_*7 + p.length();
                    break;
                  case Years:
                  case Months:
                    QL_REQUIRE(p.length()==0,
                               "impossible addition between " << *this <<
                               " and " << p);
                    break;
                  default:
                    QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
                }
                break;

              case Days:
                switch (p.units()) {
                  case Weeks:
                    length_ += p.length()*7;
                    break;
                  case Years:
                  case Months:
                    QL_REQUIRE(p.length()==0,
                               "impossible addition between " << *this <<
                               " and " << p);
                    break;
                  default:
                    QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
                }
                break;

              default:
                QL_FAIL("unknown time unit (" << Integer(units_) << ")");
            }
        }

        return *this;
    }

    Period& Period::operator-=(const Period& p) {
        return operator+=(-p);
    }

    Period& Period::operator*=(Integer n) {
        length_ *= n;
        return *this;
    }

    Period& Period::operator/=(Integer n) {
        QL_REQUIRE(n != 0, "cannot be divided by zero");
        if (length_ % n == 0) {
            // keep the original units. If the user created a
            // 24-months period, he'll probably want a 12-months one
            // when he halves it.
            length_ /= n;
        } else {
            // try
            TimeUnit units = units_;
            Integer length = length_;
            switch (units) {
              case Years:
                length *= 12;
                units = Months;
                break;
              case Weeks:
                length *= 7;
                units = Days;
                break;
              default:
                ;
            }
            QL_REQUIRE(length % n == 0,
                       *this << " cannot be divided by " << n);
            length_ = length/n;
            units_ = units;
        }
        return *this;
    }


    namespace {

        std::pair<Integer,Integer> daysMinMax(const Period& p) {
            switch (p.units()) {
              case Days:
                return std::make_pair(p.length(), p.length());
              case Weeks:
                return std::make_pair(7*p.length(), 7*p.length());
              case Months:
                return std::make_pair(28*p.length(), 31*p.length());
              case Years:
                return std::make_pair(365*p.length(), 366*p.length());
              default:
                QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
            }
        }

    }

    Real years(const Period& p) {
        if (p.length()==0) return 0.0;

        switch (p.units()) {
          case Days:
            QL_FAIL("cannot convert Days into Years");
          case Weeks:
            QL_FAIL("cannot convert Weeks into Years");
          case Months:
              return p.length()/12.0;
          case Years:
              return p.length();
          default:
            QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
        }
    }

    Real months(const Period& p) {
        if (p.length()==0) return 0.0;

        switch (p.units()) {
          case Days:
            QL_FAIL("cannot convert Days into Months");
          case Weeks:
            QL_FAIL("cannot convert Weeks into Months");
          case Months:
              return p.length();
          case Years:
              return p.length()*12.0;
          default:
            QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
        }
    }

    Real weeks(const Period& p) {
        if (p.length()==0) return 0.0;

        switch (p.units()) {
          case Days:
              return p.length()/7.0;
          case Weeks:
              return p.length();
          case Months:
            QL_FAIL("cannot convert Months into Weeks");
          case Years:
            QL_FAIL("cannot convert Years into Weeks");
          default:
            QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
        }
    }

    Real days(const Period& p) {
        if (p.length()==0) return 0.0;

        switch (p.units()) {
          case Days:
              return p.length();
          case Weeks:
              return p.length()*7.0;
          case Months:
            QL_FAIL("cannot convert Months into Days");
          case Years:
            QL_FAIL("cannot convert Years into Days");
          default:
            QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
        }
    }

    bool operator<(const Period& p1, const Period& p2) {

        // special cases
        if (p1.length() == 0)
            return p2.length() > 0;
        if (p2.length() == 0)
            return p1.length() < 0;

        // exact comparisons
        if (p1.units() == p2.units())
            return p1.length() < p2.length();
        if (p1.units() == Months && p2.units() == Years)
            return p1.length() < 12*p2.length();
        if (p1.units() == Years && p2.units() == Months)
            return 12*p1.length() < p2.length();

        // inexact comparisons (handled by converting to days and using limits)
        std::pair<Integer, Integer> p1lim = daysMinMax(p1);
        std::pair<Integer, Integer> p2lim = daysMinMax(p2);

        if (p1lim.second < p2lim.first)
            return true;
        else if (p1lim.first > p2lim.second)
            return false;
        else
            QL_FAIL("undecidable comparison between " << p1 << " and " << p2);
    }


    Period operator+(const Period& p1, const Period& p2) {
        Period result = p1;
        result += p2;
        return result;
    }

    Period operator-(const Period& p1, const Period& p2) {
        return p1+(-p2);
    }

    Period operator/(const Period& p, Integer n) {
        Period result = p;
        result /= n;
        return result;
    }

    // period formatting

    std::ostream& operator<<(std::ostream& out, const Period& p) {
        return out << io::short_period(p);
    }

    namespace detail {

        std::ostream& operator<<(std::ostream& out,
                                 const long_period_holder& holder) {
            Integer n = holder.p.length();
            switch (holder.p.units()) {
              case Days:
                return out << n << (n == 1 ? " day" : " days");
              case Weeks:
                return out << n << (n == 1 ? " week" : " weeks");
              case Months:
                return out << n << (n == 1 ? " month" : " months");
              case Years:
                return out << n << (n == 1 ? " year" : " years");
              default:
                QL_FAIL("unknown time unit (" << Integer(holder.p.units()) << ")");
            }
        }

        std::ostream& operator<<(std::ostream& out,
                                 const short_period_holder& holder) {
            Integer n = holder.p.length();
            switch (holder.p.units()) {
              case Days:
                return out << n << "D";
              case Weeks:
                return out << n << "W";
              case Months:
                return out << n << "M";
              case Years:
                return out << n << "Y";
              default:
                QL_FAIL("unknown time unit (" << Integer(holder.p.units()) << ")");
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

}
