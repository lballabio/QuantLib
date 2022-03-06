/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_market_model_discounter_hpp
#define quantlib_market_model_discounter_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    class CurveState;

    class MarketModelDiscounter {
      public:
        MarketModelDiscounter(Time paymentTime,
                              const std::vector<Time>& rateTimes);
        Real numeraireBonds(const CurveState&,
                            Size numeraire) const;
      private:
        Size before_;
        Real beforeWeight_;
    };

}

#endif


#ifndef id_8b0d9b913d141b8b9cdd5dc675ef12e4
#define id_8b0d9b913d141b8b9cdd5dc675ef12e4
inline bool test_8b0d9b913d141b8b9cdd5dc675ef12e4(int* i) { return i != 0; }
#endif
