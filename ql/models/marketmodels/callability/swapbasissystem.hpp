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


#ifndef quantlib_swap_basis_system_hpp
#define quantlib_swap_basis_system_hpp

#include <ql/models/marketmodels/callability/marketmodelbasissystem.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>

namespace QuantLib {

    class SwapBasisSystem : public MarketModelBasisSystem {
      public:
        SwapBasisSystem(const std::vector<Time>& rateTimes,
                        const std::vector<Time>& exerciseTimes);
        Size numberOfExercises() const override;
        std::vector<Size> numberOfFunctions() const override;
        const EvolutionDescription& evolution() const override;
        void nextStep(const CurveState&) override;
        void reset() override;
        std::valarray<bool> isExerciseTime() const override;
        void values(const CurveState&, std::vector<Real>& results) const override;
#if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<MarketModelBasisSystem> clone() const override;
#else
        std::auto_ptr<MarketModelBasisSystem> clone() const;
        #endif
      private:
        std::vector<Time> rateTimes_, exerciseTimes_;
        Size currentIndex_;
        std::vector<Size> rateIndex_;
        EvolutionDescription evolution_;
    };

}


#endif
