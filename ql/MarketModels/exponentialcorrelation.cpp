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

#include <ql/MarketModels/exponentialcorrelation.hpp>
#include <ql/Math/pseudosqrt.hpp>


namespace QuantLib {
    ExponentialCorrelation::ExponentialCorrelation(
            Real asyntothicCorr,
            Real beta,
            const std::vector<Volatility>& volatilities,
            const std::vector<Time>& resetTimes,
            const std::vector<Time>& evolutionTimes,
            const Size numberOfFactors,
            const Array& initialRates,
            const Array& displacements)
     :  asyntothicCorr_(asyntothicCorr), beta_(beta), 
        volatilities_(volatilities),resetTimes_(resetTimes),
        evolutionTimes_(evolutionTimes),numberOfFactors_(numberOfFactors),
        initialRates_(initialRates),displacements_(displacements),
        covariance_(volatilities.size(),volatilities.size()),
        pseudoRoots_(evolutionTimes.size())
    {
        Size n=volatilities.size();
        QL_REQUIRE(n==resetTimes.size(), "resetTimes/vol mismatch");

        std::vector<Time> stdDev(resetTimes.size());
      
        Time lastEvolutionTime = 0.0;
        Time currentEvolutionTime=0.0;
        for (Size k=0; k<evolutionTimes.size(); ++k) 
        {
            lastEvolutionTime=currentEvolutionTime;
            currentEvolutionTime=evolutionTimes[k];
            for (Size i=0; i<n; ++i) 
            {
                double effStartTime =std::min(lastEvolutionTime,resetTimes[i]);
                double effStopTime =std::min(currentEvolutionTime,resetTimes[i]);
                stdDev[i]= volatilities[i]*std::sqrt(effStopTime-effStartTime);
            }

            for (Size i=0; i<n; ++i) {
                for (Size j=0; j<n; ++j) {
                     Real correlation = asyntothicCorr + (1.0-asyntothicCorr) * 
                         std::exp(-beta*std::abs(resetTimes[i]-resetTimes[j]));
                     covariance_[i][j] =  stdDev[j] * correlation * stdDev[i];
                 }
             }

            pseudoRoots_[k]=
                rankReducedSqrt(covariance_, numberOfFactors, 1.0, SalvagingAlgorithm::None);
       
        }
       
    }



    const Array& ExponentialCorrelation::initialRates() const
    {
        return initialRates_;
    }

    const Array& ExponentialCorrelation::displacements() const
    {
        return displacements_;
    }

    Size ExponentialCorrelation::numberOfRates() const 
    {
        return initialRates_.size();
    }

    Size ExponentialCorrelation::numberOfFactors() const
    {
        return numberOfFactors_;
    }

    const Matrix& ExponentialCorrelation::pseudoRoot(Size i) const 
    {
        return pseudoRoots_[i];
    }





}




