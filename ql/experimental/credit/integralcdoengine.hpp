/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009, 2014 Jose Aparicio

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


#ifndef quantlib_integral_cdo_engine_hpp
#define quantlib_integral_cdo_engine_hpp

#include <ql/qldefines.hpp>

#ifndef QL_PATCH_SOLARIS

#include <ql/experimental/credit/syntheticcdo.hpp>
#    include <utility>

namespace QuantLib {

    class YieldTermStructure;

    class IntegralCDOEngine : public SyntheticCDO::engine {
    public:
      explicit IntegralCDOEngine(Handle<YieldTermStructure> discountCurve,
                                 Period stepSize = 3 * Months)
      : stepSize_(stepSize), discountCurve_(std::move(discountCurve)) {}
      void calculate() const override;

    protected:
      Period stepSize_;
      Handle<YieldTermStructure> discountCurve_;
    };

}

#endif

#endif


#ifndef id_3ed8918c3bc922908823c0cdd2eae480
#define id_3ed8918c3bc922908823c0cdd2eae480
inline bool test_3ed8918c3bc922908823c0cdd2eae480(const int* i) {
    return i != nullptr;
}
#endif
