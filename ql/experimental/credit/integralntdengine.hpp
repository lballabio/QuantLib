/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#ifndef quantlib_integral_ntd_engine_hpp
#define quantlib_integral_ntd_engine_hpp

#include <ql/experimental/credit/nthtodefault.hpp>
#include <utility>

namespace QuantLib {

    class YieldTermStructure;

    // Varying recoveries allowed, allow now for heterogeneous notionals
    class IntegralNtdEngine : public NthToDefault::engine {
    public:
      IntegralNtdEngine(const Period& integrationStep, Handle<YieldTermStructure> discountCurve)
      : discountCurve_(std::move(discountCurve)), integrationStepSize_(integrationStep) {}
      void calculate() const override;

    protected:
      Handle<YieldTermStructure> discountCurve_;
      Period integrationStepSize_;
    };

}

#endif
