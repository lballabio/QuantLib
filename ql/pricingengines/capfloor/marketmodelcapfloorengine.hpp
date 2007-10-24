/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file marketmodelcapfloorengine.hpp
    \brief Market-model cap/floor engine
*/

#ifndef quantlib_market_model_cap_floor_engine_hpp
#define quantlib_market_model_cap_floor_engine_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>

namespace QuantLib {

    //! Market-model cap/floor engine
    /*! \bug This engine is not yet working correctly (results are off
             the expected ones.)

        \ingroup capfloorengines
    */
    class MarketModelCapFloorEngine : public CapFloor::engine {
      public:
        MarketModelCapFloorEngine(
                             const boost::shared_ptr<MarketModelFactory>&,
                             const Handle<YieldTermStructure>& discountCurve);
        void calculate() const;
      private:
        boost::shared_ptr<MarketModelFactory> factory_;
        Handle<YieldTermStructure> discountCurve_;
    };

}

#endif
