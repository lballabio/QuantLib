/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Katiuscia Manzoni

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

#include <ql/Utilities/null.hpp>

namespace QuantLib {

    Real midEquivalent(const Real bid,
                       const Real ask,
                       const Real last,
                       const Real close) {

        if (bid != Null<Real>() && bid != 0.0) {
            if (ask != Null<Real>() && ask != 0.0) return ((bid+ask)/2.0);
            else                                   return bid;
        } else {
            if (ask != Null<Real>() && ask != 0.0)          return ask;
            else if (last != Null<Real>() && last != 0.0)   return last;
            else if (close != Null<Real>() && close != 0.0) return close;
            else                                            return Null<Real>();
        }
    }
}
