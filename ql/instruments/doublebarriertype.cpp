/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

#include <ql/instruments/doublebarriertype.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out,
                             DoubleBarrier::Type type) {
        switch (type) {
          case DoubleBarrier::KnockIn:
            return out << "KnockIn";
          case DoubleBarrier::KnockOut:
            return out << "KnockOut";
          case DoubleBarrier::KIKO:
            return out << "KI lo+KO up";
          case DoubleBarrier::KOKI:
            return out << "KO lo+KI up";
          default:
            QL_FAIL("unknown DoubleBarrier::Type (" << Integer(type) << ")");
        }
    }

}
