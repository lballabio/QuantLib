/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

#include <ql/models/volatility/garch.hpp>
#include <ql/errors.hpp>

namespace QuantLib {
  TimeSeries<Volatility>
  Garch11::calculate(const TimeSeries<Volatility>& qs) {
    return calculate(qs, alpha_, beta_, gamma_* vl_);
  }

  TimeSeries<Volatility>
  Garch11::calculate(const TimeSeries<Volatility>& qs,
             Real alpha, Real beta, Real omega) {
    TimeSeries<Volatility> retval;
    TimeSeries<Volatility>::const_iterator cur = qs.begin();
    retval[cur->first] = cur->second;
    Real sigma2 = cur->second * cur->second;
    ++cur;
    while (cur != qs.end()) {
      Real u = cur->second;
      sigma2 = omega + alpha * u * u +
          beta * sigma2;
      retval[cur->first] = std::sqrt(sigma2);
      ++cur;
    }
    return retval;
  }

  void Garch11::calibrate(const TimeSeries<Volatility>&) {}

  Real Garch11::costFunction(const TimeSeries<Volatility>& qs,
                             Real alpha, Real beta, Real omega) {
      Real retval(0.0);
      TimeSeries<Volatility> test =
          calculate(qs, alpha, beta, omega);
      std::vector<Volatility> testValues = test.values();
      std::vector<Volatility> quoteValues = qs.values();
      QL_REQUIRE(testValues.size() == quoteValues.size(),
                 "quote and test values do not match");
      std::vector<Volatility>::const_iterator i_t, i_q;
      i_t = testValues.begin();
      i_q = quoteValues.begin();
      while (i_t != testValues.end()) {
          Real v = (*i_q) * (*i_q);
          Real u2 = (*i_t) * (*i_t);
          retval += 2.0 * std::log(v) + u2/ (v*v);
          i_t++;
          i_q++;
      }
      return retval;
  }
}

