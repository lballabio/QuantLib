/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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


#ifndef quantlib_market_model_differences_hpp
#define quantlib_market_model_differences_hpp

#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <vector>

namespace QuantLib {

    Disposable<std::vector<Real> > totalCovariancesDifferences(
                                           const MarketModel& marketModel1,
                                           const MarketModel& marketModel2) {
        QL_ENSURE(marketModel1.initialRates() == marketModel2.initialRates(),
                  "initialRates do not match");
        const EvolutionDescription& evolutionDescription1 
                                           = marketModel1.evolution();
        const EvolutionDescription& evolutionDescription2 
                                           = marketModel2.evolution();
        QL_ENSURE(evolutionDescription1.evolutionTimes()
                  == evolutionDescription2.evolutionTimes(),
                  "Evolution times do not match");
        
        const Matrix& totalCovariance1 
            = marketModel1.totalCovariance(marketModel1.numberOfSteps()-1);
        const Matrix& totalCovariance2 
            = marketModel2.totalCovariance(marketModel2.numberOfSteps()-1);
        std::vector<Real> differences(totalCovariance1.columns());
        Time maturity = evolutionDescription1.evolutionTimes().back();
        for(Size i=0; i< totalCovariance1.columns(); ++i)
            differences[i] = (totalCovariance1[i][i]-totalCovariance2[i][i])
            /std::sqrt(maturity);
        return differences;
    }       

    Disposable<std::vector<Real> > forwardRateCovariancesDifferences(
                                           const MarketModel& marketModel1,
                                           const MarketModel& marketModel2, 
                                           Size index) {
        QL_ENSURE(marketModel1.initialRates() == marketModel2.initialRates(),
                  "initialRates do not match");
        const EvolutionDescription& evolutionDescription1 
                                           = marketModel1.evolution();
        const EvolutionDescription& evolutionDescription2 
                                           = marketModel2.evolution();
        QL_ENSURE(evolutionDescription1.evolutionTimes()
                  == evolutionDescription2.evolutionTimes(),
                  "Evolution times do not match");
        QL_ENSURE(index<evolutionDescription1.numberOfSteps(), 
            "the index given is greater than the number of steps");
        const std::vector<Time>& evolutionTimes 
            = evolutionDescription1.evolutionTimes();
        std::vector<Real> differences(evolutionTimes.size());
        Time previousEvolutionTime = 0;
        for(Size i=0; i<evolutionTimes.size(); ++i) {
            Time currentEvolutionTime = evolutionTimes[i];
            const Matrix& covariance1 = marketModel1.covariance(i);
            const Matrix& covariance2 = marketModel2.covariance(i);
            differences[i] 
                = (covariance1[index][index] - covariance2[index][index])
                / std::sqrt(currentEvolutionTime - previousEvolutionTime);
            previousEvolutionTime = currentEvolutionTime;
        }
        return differences;
    }

}

#endif
