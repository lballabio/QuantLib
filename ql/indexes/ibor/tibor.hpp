/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file tibor.hpp
    \brief %JPY %TIBOR rate
*/

#ifndef quantlib_tibor_hpp
#define quantlib_tibor_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/asia.hpp>

namespace QuantLib {

    //! %JPY %TIBOR index
    /*! Tokyo Interbank Offered Rate.

        \warning This is the rate fixed in Tokio by JBA. Use JPYLibor
                 if you're interested in the London fixing by BBA.

        \todo check settlement days and end-of-month adjustment.
    */
    class Tibor : public IborIndex {
      public:
        Tibor(const Period& tenor,
              const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : IborIndex("Tibor", tenor, 2, JPYCurrency(),
                    Japan(), ModifiedFollowing,
                    false, Actual365Fixed(), h) {}
    };

}


#endif


#ifndef id_c5812d9004f159a3dcbcd44ed3eba8f3
#define id_c5812d9004f159a3dcbcd44ed3eba8f3
inline bool test_c5812d9004f159a3dcbcd44ed3eba8f3(int* i) { return i != 0; }
#endif
