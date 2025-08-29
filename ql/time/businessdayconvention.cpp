/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2014 Paolo Mazzocchi

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

#include <ql/time/businessdayconvention.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out,
                             BusinessDayConvention b) {
        switch (b) {
          case Following:
            return out << "Following";
          case ModifiedFollowing:
            return out << "Modified Following";
          case HalfMonthModifiedFollowing:
            return out << "Half-Month Modified Following";
          case Preceding:
            return out << "Preceding";
          case ModifiedPreceding:
            return out << "Modified Preceding";
          case Unadjusted:
            return out << "Unadjusted";
          case Nearest:
            return out << "Nearest";
          default:
            QL_FAIL("unknown BusinessDayConvention (" << Integer(b) << ")");
        }
    }

}
