/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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
          case Once:
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
          case Biweekly:
          case Weekly:
            units_ = Weeks;
            length_ = 52/f;
            break;
          case Daily:
            units_ = Days;
            length_ = 1;
            break;
          default:
            QL_FAIL("unknown frequency (" << Integer(f));
        }
    }

    Frequency Period::frequency() const {
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

    Period& Period::operator+=(const Period& p) {

        if (units_==p.units()) {
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
                    QL_FAIL("impossible addition between "
                             << *this << " and " << p);
                  default:
                    QL_FAIL("unknown units");
                }
                break;

              case Months:
                switch (p.units()) {
                  case Years:
                    length_ += p.length()*12;
                    break;
                  case Weeks:
                  case Days:
                    QL_FAIL("impossible addition between "
                             << *this << " and " << p);
                  default:
                    QL_FAIL("unknown units");
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
                    QL_FAIL("impossible addition between "
                             << *this << " and " << p);
                  default:
                    QL_FAIL("unknown units");
                }
                break;

              case Days:
                switch (p.units()) {
                  case Weeks:
                    length_ += p.length()*7;
                    break;
                  case Years:
                  case Months:
                    QL_FAIL("impossible addition between "
                             << *this << " and " << p);
                  default:
                    QL_FAIL("unknown units");
                }
                break;

              default:
                QL_FAIL("unknown units");
            }
        }
        return *this;
    }

    Period& Period::operator-=(const Period& p) {
        return operator+=(-p);
    }

    bool operator<(const Period& p1, const Period& p2) {
        if (p1.length()==0) return (p2.length()>0);
        if (p2.length()==0) return (p1.length()<0);

        switch (p1.units()) {
          case Days:
            switch (p2.units()) {
              case Days:
                return (p1.length() < p2.length());
              case Weeks:
                return (p1.length() < p2.length() * 7);
              case Months:
                if (p1.length() < p2.length() * 28)
                    return true;
                else
                    QL_FAIL("undecidable comparison between "
                             << p1 << " and " << p2);
              case Years:
                return (p1.length() < p2.length() * 365);
              default:
                QL_FAIL("unknown units");
            }
          case Weeks:
            switch (p2.units()) {
              case Days:
                return (p1.length() * 7 < p2.length());
              case Weeks:
                return (p1.length() < p2.length());
              case Months:
                if (p1.length() * 7 < p2.length() * 28)
                    return true;
                else
                    QL_FAIL("undecidable comparison between "
                             << p1 << " and " << p2);
              case Years:
                if (p1.length() * 7 < p2.length() * 365)
                    return true;
                else
                    QL_FAIL("undecidable comparison between "
                             << p1 << " and " << p2);
              default:
                QL_FAIL("unknown units");
            }
          case Months:
            switch (p2.units()) {
              case Days:
                // Sup[days in p1.length() months] < days in p2
                if (p1.length() * 31 < p2.length())
                    return true;
                // almost 28 days in p1 and less than 28 days in p2
                else if ((p1.length()!=0) && p2.length()< 28)
                    return false;
                else
                    QL_FAIL("undecidable comparison between "
                             << p1 << " and " << p2);
              case Weeks:
                if (p1.length()* 31 < p2.length()  * 7)
                    return true;
                else
                    QL_FAIL("undecidable comparison between "
                             << p1 << " and " << p2);
              case Months:
                return (p1.length() < p2.length());
              case Years:
                return (p1.length() < p2.length() * 12);
              default:
                QL_FAIL("unknown units");
            }
          case Years:
            switch (p2.units()) {
              case Days:
                if (p1.length() * 366 < p2.length())
                    return true;
                // almost 365 days in p1 and less than 365 days in p2
                else if ((p1.length()!=0) && p2.length()< 365)
                    return false;
                else
                    QL_FAIL("undecidable comparison between "
                             << p1 << " and " << p2);
              case Weeks:
                if (p1.length() * 366 < p2.length() * 7)
                    return true;
                else
                    QL_FAIL("undecidable comparison between "
                             << p1 << " and " << p2);
              case Months:
                return (p1.length() * 12 < p2.length());
              case Years:
                return (p1.length() < p2.length());
              default:
                QL_FAIL("unknown units");
            }
          default:
            QL_FAIL("unknown units");
        }
    }


    Period operator+(const Period& p1, const Period& p2) {
        Period result = p1;
        result += p2;
        return result;
    }

    Period operator-(const Period& p1, const Period& p2) {
        return p1+(-p2);
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

}
