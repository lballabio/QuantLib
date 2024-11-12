/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file bjerksundstenslandspreadengine.hpp
    \brief Bjerksund and Stensland formulae (2006)
*/

#ifndef quantlib_bjerksund_stensland_spread_engine_hpp
#define quantlib_bjerksund_stensland_spread_engine_hpp

#include <ql/pricingengines/basket/spreadblackscholesvanillaengine.hpp>

namespace QuantLib {

    //! Pricing engine for spread option on two futures
    /*! P. Bjerksund and G. Stensland,
        Closed form spread option valuation,
        Quantitative Finance, 14 (2014), pp. 1785–1794.

        \ingroup basketengines
    */
    class BjerksundStenslandSpreadEngine : public SpreadBlackScholesVanillaEngine {
      public:
        BjerksundStenslandSpreadEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
            ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
            Real correlation);

      protected:
        Real calculate(Real f1, Real f2, Real strike, Option::Type optionType,
            Real variance1, Real variance2, DiscountFactor df) const override;
    };
}


#endif
