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


#ifndef quantlib_market_model_evolver_hpp
#define quantlib_market_model_evolver_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    class CurveState;

    //! Market-model evolver
    /*! Abstract base class. The evolver does the actual gritty work of
        evolving the forward rates from one time to the next.
    */
    class MarketModelEvolver {
      public:
        virtual ~MarketModelEvolver() = default;

        virtual const std::vector<Size>& numeraires() const = 0;
        virtual Real startNewPath() = 0;
        virtual Real advanceStep() = 0;
        virtual Size currentStep() const = 0;
        virtual const CurveState& currentState() const = 0;
        virtual void setInitialState(const CurveState&) = 0;
    };

}

#endif


#ifndef id_afb9e2aa8344a63eaf9fd593d33e6e48
#define id_afb9e2aa8344a63eaf9fd593d33e6e48
inline bool test_afb9e2aa8344a63eaf9fd593d33e6e48(int* i) { return i != 0; }
#endif
