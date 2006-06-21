/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

#include <ql/prices.hpp>

namespace QuantLib {

    IntervalPrice::IntervalPrice()
    : open_(Null<Real>()), close_(Null<Real>()),
      high_(Null<Real>()), low_(Null<Real>()) {}

    IntervalPrice::IntervalPrice(Real open, Real close, Real high, Real low)
    : open_(open), close_(close), high_(high), low_(low) {}

    Real IntervalPrice::value(IntervalPrice::Type t) const {
        switch(t) {
          case Open:
            return open_;
          case Close:
            return close_;
          case High:
            return high_;
          case Low:
            return low_;
          default:
            QL_FAIL("Unknown price type");
        }
    }

    void IntervalPrice::setValue(Real value,
                                 IntervalPrice::Type t) {
        switch(t) {
          case Open:
            open_ = value;
          case Close:
            close_ = value;
          case High:
            high_ = value;
          case Low:
            low_ = value;
          default:
            QL_FAIL("Unknown price type");
        }
    }

    void IntervalPrice::setValues(Real open, Real close, Real high, Real low) {
        open_ = open; close_ = close; high_ = high; low_ = low;
    }


    TimeSeries<IntervalPrice> IntervalPrice::makeSeries(
                                               const std::vector<Date>& d,
                                               const std::vector<Real>& open,
                                               const std::vector<Real>& close,
                                               const std::vector<Real>& high,
                                               const std::vector<Real>& low) {
        Size dsize = d.size();
        QL_REQUIRE((open.size() == dsize && close.size() == dsize &&
                    high.size() == dsize && low.size() == dsize),
                   "size mismatch (" << dsize << ", "
                                     << open.size() << ", "
                                     << close.size() << ", "
                                     << high.size() << ", "
                                     << low.size() << ")");
        TimeSeries<IntervalPrice> retval;
        std::vector<Date>::const_iterator i;
        std::vector<Real>::const_iterator openi, closei, highi, lowi;
        openi = open.begin();
        closei = close.begin();
        highi = high.begin();
        lowi = low.begin();
        for (i = d.begin(); i != d.end(); i++) {
            retval[*i] = IntervalPrice(*openi, *closei, *highi, *lowi);
            ++openi; ++closei; ++highi; ++lowi;
        }
        return retval;
    }

    std::vector<Real> IntervalPrice::extractValues(
                                           const TimeSeries<IntervalPrice>& ts,
                                           IntervalPrice::Type t)  {
        std::vector<Real> returnval;
        for (TimeSeries<IntervalPrice>::const_iterator i = ts.begin();
             i != ts.end(); i++) {
            returnval.push_back(i->second.value(t));
        }
        return returnval;
    }

    TimeSeries<Real> IntervalPrice::extractComponent(
                                          const TimeSeries<IntervalPrice>& ts,
                                          IntervalPrice::Type t) {
        std::vector<Date> dates = ts.dates();
        std::vector<Real> values = extractValues(ts, t);
        return TimeSeries<Real>(dates.begin(), dates.end(), values.begin());
    }

}

