/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_market_model_basis_functions_hpp
#define quantlib_market_model_basis_functions_hpp

#include <ql/MarketModels/curvestate.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>

namespace QuantLib {

    class MarketModelBasisSystem {
      public:
        virtual ~MarketModelBasisSystem() {}
        virtual Size numberOfExercises() const = 0;
        // possibly different for each exercise
        virtual std::vector<Size> numberOfFunctions() const = 0;
        // including any time at which state should be updated
        virtual EvolutionDescription evolution() const = 0;
        virtual void nextStep(const CurveState&) = 0;
        virtual void reset() = 0;
        // whether or not evolution times are exercise times
        virtual std::vector<bool> isExerciseTime() const = 0;
        virtual void values(const CurveState&,
                            std::vector<Real>& results) const = 0;
    };

}


#endif
