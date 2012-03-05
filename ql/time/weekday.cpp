/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 Toyin Akin

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

#include <ql/time/weekday.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    // weekday formatting

    std::ostream& operator<<(std::ostream& out, const Weekday& w) {
        return out << io::long_weekday(w);
    }

    namespace detail {

        std::ostream& operator<<(std::ostream& out,
                                 const long_weekday_holder& holder) {
            switch (holder.d) {
              case Sunday:
                return out << "Sunday";
              case Monday:
                return out << "Monday";
              case Tuesday:
                return out << "Tuesday";
              case Wednesday:
                return out << "Wednesday";
              case Thursday:
                return out << "Thursday";
              case Friday:
                return out << "Friday";
              case Saturday:
                return out << "Saturday";
              default:
                QL_FAIL("unknown weekday");
            }
        }

        std::ostream& operator<<(std::ostream& out,
                                 const short_weekday_holder& holder) {
            switch (holder.d) {
              case Sunday:
                return out << "Sun";
              case Monday:
                return out << "Mon";
              case Tuesday:
                return out << "Tue";
              case Wednesday:
                return out << "Wed";
              case Thursday:
                return out << "Thu";
              case Friday:
                return out << "Fri";
              case Saturday:
                return out << "Sat";
              default:
                QL_FAIL("unknown weekday");
            }
        }

        std::ostream& operator<<(std::ostream& out,
                                 const shortest_weekday_holder& holder) {
            switch (holder.d) {
              case Sunday:
                return out << "Su";
              case Monday:
                return out << "Mo";
              case Tuesday:
                return out << "Tu";
              case Wednesday:
                return out << "We";
              case Thursday:
                return out << "Th";
              case Friday:
                return out << "Fr";
              case Saturday:
                return out << "Sa";
              default:
                QL_FAIL("unknown weekday");
            }
        }

    }

    namespace io {

        detail::long_weekday_holder long_weekday(Weekday d) {
            return detail::long_weekday_holder(d);
        }

        detail::short_weekday_holder short_weekday(Weekday d) {
            return detail::short_weekday_holder(d);
        }

        detail::shortest_weekday_holder shortest_weekday(Weekday d) {
            return detail::shortest_weekday_holder(d);
        }

    }

}
