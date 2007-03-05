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

#include <ql/frequency.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

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
