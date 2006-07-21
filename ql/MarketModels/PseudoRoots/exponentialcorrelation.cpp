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

#include <ql/MarketModels/PseudoRoots/exponentialcorrelation.hpp>
#include <ql/Math/pseudosqrt.hpp>


namespace QuantLib
{
    ExponentialCorrelation::ExponentialCorrelation(
            Real longTermCorr,
            Real beta,
            const std::vector<Volatility>& volatilities,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const Array& initialRates,
            const Array& displacements)
     :  longTermCorr_(longTermCorr), beta_(beta), 
        volatilities_(volatilities),
        rateTimes_(evolution.rateTimes()),
        evolutionTimes_(evolution.evolutionTimes()),
        numberOfFactors_(numberOfFactors),
        initialRates_(initialRates),
        displacements_(displacements),
        pseudoRoots_(evolution.evolutionTimes().size())
    {
        Size n=volatilities.size();
        QL_REQUIRE(n==rateTimes_.size()-1, "resetTimes/vol mismatch");

        Matrix covariance(n, n);
        std::vector<Time> stdDev(n);
      
        Time lastEvolutionTime = 0.0;
        Time currentEvolutionTime=0.0;
        for (Size k=0; k<evolutionTimes_.size(); ++k) 
        {
            lastEvolutionTime=currentEvolutionTime;
            currentEvolutionTime=evolutionTimes_[k];
            for (Size i=0; i<n; ++i) 
            {
                double effStartTime =
                    std::min(lastEvolutionTime,    rateTimes_[i]);
                double effStopTime =
                    std::min(currentEvolutionTime, rateTimes_[i]);
                stdDev[i]= volatilities[i]*std::sqrt(effStopTime-effStartTime);
            }

            for (Size i=0; i<n; ++i) {
                for (Size j=i; j<n; ++j) {
                     Real correlation = longTermCorr + (1.0-longTermCorr) * 
                         std::exp(-beta*std::abs(rateTimes_[i]-rateTimes_[j]));
                     covariance[i][j] =  covariance[j][i] = 
                         stdDev[j] * correlation * stdDev[i];
                 }
            }

            pseudoRoots_[k]=
                //rankReducedSqrt(covariance, numberOfFactors, 1.0,
                //                SalvagingAlgorithm::None);
                pseudoSqrt(covariance, SalvagingAlgorithm::None);
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




