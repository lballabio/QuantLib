/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

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

#include <ql/instruments/softbarriertype.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out,
                             SoftBarrier::Type type) {
        switch (type) {
          case SoftBarrier::DownIn:
            return out << "DownIn";
          case SoftBarrier::DownOut:
            return out << "DownOut";
          case SoftBarrier::UpOut:
            return out << "UpOut";
          case SoftBarrier::UpIn:
            return out << "UpIn";
          default:
            QL_FAIL("unknown SoftBarrier::Type (" << Integer(type) << ")");
        }
    }

}
