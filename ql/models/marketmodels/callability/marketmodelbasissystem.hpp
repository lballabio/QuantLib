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


#ifndef quantlib_market_model_basis_system_hpp
#define quantlib_market_model_basis_system_hpp

#include <ql/models/marketmodels/callability/nodedataprovider.hpp>
#include <memory>

namespace QuantLib {

    class MarketModelBasisSystem : public MarketModelNodeDataProvider {
      public:
        // possibly different for each exercise
        virtual std::vector<Size> numberOfFunctions() const = 0;
        std::vector<Size> numberOfData() const override { return numberOfFunctions(); }
#if defined(QL_USE_STD_UNIQUE_PTR)
        virtual std::unique_ptr<MarketModelBasisSystem> clone() const = 0;
        #else
        virtual std::auto_ptr<MarketModelBasisSystem> clone() const = 0;
        #endif
    };

}

#endif
