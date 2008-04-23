/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file midpointcdsengine.hpp
    \brief Mid-point engine for credit default swaps
*/

#ifndef quantlib_mid_point_cds_engine_hpp
#define quantlib_mid_point_cds_engine_hpp

#include <ql/instruments/creditdefaultswap.hpp>

namespace QuantLib {

    class MidPointCdsEngine : public CreditDefaultSwap::engine {
      public:
        MidPointCdsEngine(const Issuer& issuer,
                          const Handle<YieldTermStructure>& discountCurve);
        void calculate() const;
      private:
        Issuer issuer_;
        Handle<YieldTermStructure> discountCurve_;
    };

}


#endif
