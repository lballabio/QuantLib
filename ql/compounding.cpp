/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Ferdinando Ametrano

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

#include <ql/compounding.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <sstream>
#include <iomanip>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out, Compounding c) {
        if (ir.rate() == Null<Rate>())
            return out << "null interest rate";

        out << io::rate(ir.rate()) << " " << ir.dayCounter().name() << " ";
        switch (ir.compounding()) {
          case Simple:
            out << "simple compounding";
            break;
          case Compounded:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << ir.frequency() <<" compounding";
            }
            break;
          case Continuous:
            out << "continuous compounding";
            break;
          case SimpleThenCompounded:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << "simple compounding up to "
                    << Integer(12/ir.frequency()) << " months, then "
                    << ir.frequency() << " compounding";
            }
            break;
          default:
            QL_FAIL("unknown compounding convention ("
                    << Integer(ir.compounding()) << ")");
        }
        return out;
    }

}
