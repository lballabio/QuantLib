/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/time/dategenerationrule.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out, DateGeneration::Rule r) {
        switch (r) {
          case DateGeneration::Backward:
            return out << "Backward";
          case DateGeneration::Forward:
            return out << "Forward";
          case DateGeneration::Zero:
            return out << "Zero";
          case DateGeneration::ThirdWednesday:
            return out << "ThirdWednesday";
          case DateGeneration::ThirdWednesdayInclusive:
            return out << "ThirdWednesdayInclusive";
          case DateGeneration::Twentieth:
            return out << "Twentieth";
          case DateGeneration::TwentiethIMM:
            return out << "TwentiethIMM";
          case DateGeneration::OldCDS:
            return out << "OldCDS";
          case DateGeneration::CDS:
            return out << "CDS";
          case DateGeneration::CDS2015:
            return out << "CDS2015";
          default:
            QL_FAIL("unknown DateGeneration::Rule (" << Integer(r) << ")");
        }
    }

}
