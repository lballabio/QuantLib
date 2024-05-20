/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 StatPro Italia srl

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

/*! \file fedfunds.hpp
    \brief %FedFunds index
*/

#ifndef quantlib_fed_funds_hpp
#define quantlib_fed_funds_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %Fed Funds rate fixed by the FED.
    /*! (for balances held at the Federal Reserve) */
    class FedFunds : public OvernightIndex {
      public:
        explicit FedFunds(Handle<YieldTermStructure> h = {});
    };

}


#endif
