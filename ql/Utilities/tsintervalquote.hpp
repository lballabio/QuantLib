/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file tsintervalquote.hpp
    \brief helper functions to create and manipulate time series of
    interval quotes
*/

#ifndef quantlib_time_series_interval_quote_hpp
#define quantlib_time_series_interval_quote_hpp

#include <ql/quote.hpp>
#include <ql/timeseries.hpp>
#include <vector>
#include <map>

namespace QuantLib {
  class TimeSeriesIntervalQuoteHelper {  
  public:
    static 
    TimeSeries<IntervalQuote> create(const std::vector<Date>& d,
				     const std::vector<Real>& open,
				     const std::vector<Real>& close,
				     const std::vector<Real>& high,
				     const std::vector<Real>& low) {
      Size dsize = d.size();
      QL_REQUIRE((open.size() == dsize &&
		 close.size() == dsize &&
		 high.size() == dsize &&
		 low.size() == dsize),
		 "size mismatch (" << dsize << ", "
		 << open.size() << ", "
		 << close.size() << ", "
		 << high.size() << ", "
		 << low.size() << ")");
      TimeSeries<IntervalQuote> retval;
      std::vector<Date>::const_iterator i;
      std::vector<Real>::const_iterator openi, closei, highi, lowi;
      openi = open.begin();
      closei = close.begin();
      highi = high.begin();
      lowi = low.begin();
      for (i = d.begin(); i != d.end(); i++) {
          retval.insert(*i,
                        IntervalQuote(*openi,
                                      *closei,
                                      *highi,
                                      *lowi));
          openi++; closei++; highi++; lowi++;
      }
      return retval;
    };
  };
  static std::vector<Real> extract(TimeSeries<IntervalQuote> ts,
			      IntervalQuote::price_type pt) {
    std::vector<Real> retval;
    for (TimeSeries<IntervalQuote>::const_valid_iterator 
             i = ts.vbegin();
         i != ts.vend(); i++) {
        retval.push_back(i->second.value(pt));
    }
    return retval;
  };
}
#endif

