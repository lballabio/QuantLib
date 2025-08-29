/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

#include <ql/interestrate.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <iomanip>
#include <sstream>
#include <utility>

namespace QuantLib {

    // constructors

    InterestRate::InterestRate()
    : r_(Null<Real>()) {}

    InterestRate::InterestRate(Rate r, DayCounter dc, Compounding comp, Frequency freq)
    : r_(r), dc_(std::move(dc)), comp_(comp), freqMakesSense_(false) {

        if (comp_==Compounded || comp_==SimpleThenCompounded || comp_==CompoundedThenSimple) {
            freqMakesSense_ = true;
            QL_REQUIRE(freq!=Once && freq!=NoFrequency,
                       "frequency not allowed for this interest rate");
            freq_ = Real(freq);
        }
    }

    Real InterestRate::compoundFactor(Time t) const {

        QL_REQUIRE(t>=0.0, "negative time (" << t << ") not allowed");
        QL_REQUIRE(r_ != Null<Rate>(), "null interest rate");
        switch (comp_) {
          case Simple:
            return 1.0 + r_*t;
          case Compounded:
            return std::pow(1.0+r_/freq_, freq_*t);
          case Continuous:
            return std::exp(r_*t);
          case SimpleThenCompounded:
            if (t<=1.0/Real(freq_))
                return 1.0 + r_*t;
            else
                return std::pow(1.0+r_/freq_, freq_*t);
          case CompoundedThenSimple:
            if (t>1.0/Real(freq_))
                return 1.0 + r_*t;
            else
                return std::pow(1.0+r_/freq_, freq_*t);
          default:
            QL_FAIL("unknown compounding convention");
        }
    }

    InterestRate InterestRate::impliedRate(Real compound,
                                           const DayCounter& resultDC,
                                           Compounding comp,
                                           Frequency freq,
                                           Time t) {

        QL_REQUIRE(compound>0.0, "positive compound factor required");

        Rate r;
        if (compound==1.0) {
            QL_REQUIRE(t>=0.0, "non negative time (" << t << ") required");
            r = 0.0;
        } else {
            QL_REQUIRE(t>0.0, "positive time (" << t << ") required");
            switch (comp) {
              case Simple:
                r = (compound - 1.0)/t;
                break;
              case Compounded:
                r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
                break;
              case Continuous:
                r = std::log(compound)/t;
                break;
              case SimpleThenCompounded:
                if (t<=1.0/Real(freq))
                    r = (compound - 1.0)/t;
                else
                    r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
                break;
              case CompoundedThenSimple:
                if (t>1.0/Real(freq))
                    r = (compound - 1.0)/t;
                else
                    r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
                break;
              default:
                QL_FAIL("unknown compounding convention ("
                        << Integer(comp) << ")");
            }
        }
        return InterestRate(r, resultDC, comp, freq);
    }


    std::ostream& operator<<(std::ostream& out, const InterestRate& ir) {
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
          case CompoundedThenSimple:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << "compounding up to "
                    << Integer(12/ir.frequency()) << " months, then "
                    << ir.frequency() << " simple compounding";
            }
            break;
          default:
            QL_FAIL("unknown compounding convention ("
                    << Integer(ir.compounding()) << ")");
        }
        return out;
    }

}
