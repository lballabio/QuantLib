/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Neil Firth
 Copyright (C) 2003, 2004, 2007 Ferdinando Ametrano
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

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

#include <ql/instruments/averagetype.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out,
                             Average::Type type) {
        switch (type) {
          case Average::Arithmetic:
            return out << "Arithmetic";
          case Average::Geometric:
            return out << "Geometric";
          default:
            QL_FAIL("unknown Average::Type (" << Integer(type) << ")");
        }
    }

}
