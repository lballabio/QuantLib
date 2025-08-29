/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/callability/swapforwardbasissystem.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>

namespace QuantLib
{

    SwapForwardBasisSystem::SwapForwardBasisSystem(const std::vector<Time>& rateTimes,
                                     const std::vector<Time>& exerciseTimes)
                                     : 
                                    rateTimes_(rateTimes), exerciseTimes_(exerciseTimes),
                                    rateIndex_(exerciseTimes.size()),
                                    evolution_(rateTimes, exerciseTimes) 
     {
        Size j = 0;
        for (Size i=0; i<exerciseTimes.size(); ++i) 
        {
            while (j < rateTimes.size() && rateTimes[j] < exerciseTimes[i])
                ++j;
            rateIndex_[i] = j;
        }
    }

    Size SwapForwardBasisSystem::numberOfExercises() const 
    {
        return exerciseTimes_.size();
    }

    std::vector<Size> SwapForwardBasisSystem::numberOfFunctions() const 
    {
        std::vector<Size> sizes(exerciseTimes_.size(), 10);
        
        if (rateIndex_[exerciseTimes_.size()-1] == rateTimes_.size()-3)
            sizes.back() = 6;

        if (rateIndex_[exerciseTimes_.size()-1] == rateTimes_.size()-2)
            sizes.back() = 3;

        return sizes;
    }

    const EvolutionDescription& SwapForwardBasisSystem::evolution() const 
    {
        return evolution_;
    }

    void SwapForwardBasisSystem::nextStep(const CurveState&) 
    {
        ++currentIndex_;
    }

    void SwapForwardBasisSystem::reset() 
    {
        currentIndex_ = 0;
    }

    std::valarray<bool> SwapForwardBasisSystem::isExerciseTime() const 
    {
        return std::valarray<bool>(true, exerciseTimes_.size());
    }

    void SwapForwardBasisSystem::values(const CurveState& currentState,
                                 std::vector<Real>& results) const 
    {
        Size rateIndex = rateIndex_[currentIndex_-1];

        if (rateIndex < rateTimes_.size() -3)
        {
            results.resize(10);

            Real x= currentState.forwardRate(rateIndex);
            Real y = currentState.coterminalSwapRate(rateIndex+1);
            Real z = currentState.discountRatio(rateIndex,rateTimes_.size()-1);

            results[0] = 1.0;
            results[1] = x;
            results[2] = y;
            results[3] = z;
            results[4] = x*y;
            results[5] = y*z;
            results[6] = z*x;
            results[7] = x*x;
            results[8] = y*y;
            results[9] = z*z;
        }
        else
            if ( rateIndex == rateTimes_.size() -3)
            {
                 Real x= currentState.forwardRate(rateIndex);
                  Real y = currentState.forwardRate(rateIndex+1);
                  results.resize(6);
                 results[0] = 1.0;
                 results[1] = x;
                 results[2] = y;
                 results[3] = x*x;
                 results[4] = x*y;
                 results[5] = y*y;
     
            }
            else
            {
                 Real x= currentState.forwardRate(rateIndex);
                 results.resize(3);
                 results[0] =1.0;
                 results[1] = x;
                 results[2] = x*x;

            }
    }

    std::unique_ptr<MarketModelBasisSystem>
    SwapForwardBasisSystem::clone() const {
        return std::unique_ptr<MarketModelBasisSystem>(new SwapForwardBasisSystem(*this));
    }

}

