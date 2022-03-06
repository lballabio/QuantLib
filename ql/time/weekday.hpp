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

/*! \file weekday.hpp
    \brief Weekday enumeration
*/

#ifndef quantlib_weekday_hpp
#define quantlib_weekday_hpp

#include <ql/qldefines.hpp>
#include <iosfwd>

namespace QuantLib {

    /*! Day's serial number MOD 7;
        WEEKDAY Excel function is the same except for Sunday = 7.

        \ingroup datetime
    */
    enum Weekday { Sunday    = 1,
                   Monday    = 2,
                   Tuesday   = 3,
                   Wednesday = 4,
                   Thursday  = 5,
                   Friday    = 6,
                   Saturday  = 7,
                   Sun = 1,
                   Mon = 2,
                   Tue = 3,
                   Wed = 4,
                   Thu = 5,
                   Fri = 6,
                   Sat = 7
    };

    /*! \relates Weekday */
    std::ostream& operator<<(std::ostream&, const Weekday&);

    namespace detail {

        struct long_weekday_holder {
            explicit long_weekday_holder(Weekday d) : d(d) {}
            Weekday d;
        };
        std::ostream& operator<<(std::ostream&, const long_weekday_holder&);

        struct short_weekday_holder {
            explicit short_weekday_holder(Weekday d) : d(d) {}
            Weekday d;
        };
        std::ostream& operator<<(std::ostream&, const short_weekday_holder&);

        struct shortest_weekday_holder {
            explicit shortest_weekday_holder(Weekday d) : d(d) {}
            Weekday d;
        };
        std::ostream& operator<<(std::ostream&,
                                 const shortest_weekday_holder&);

    }

    namespace io {

        //! output weekdays in long format
        /*! \ingroup manips */
        detail::long_weekday_holder long_weekday(Weekday);

        //! output weekdays in short format (three letters)
        /*! \ingroup manips */
        detail::short_weekday_holder short_weekday(Weekday);

        //! output weekdays in shortest format (two letters)
        /*! \ingroup manips */
        detail::shortest_weekday_holder shortest_weekday(Weekday);

    }

}

#endif


#ifndef id_faa597c1dc7d564369fd55316aa1a9a3
#define id_faa597c1dc7d564369fd55316aa1a9a3
inline bool test_faa597c1dc7d564369fd55316aa1a9a3(const int* i) {
    return i != nullptr;
}
#endif
