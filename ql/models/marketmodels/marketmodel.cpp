/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Mark Joshi

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

#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>


namespace QuantLib {

    const Matrix& MarketModel::covariance(Size i) const {
        if (covariance_.empty()) {
            covariance_.resize(numberOfSteps());
            for (Size j=0; j<numberOfSteps(); ++j)
                covariance_[j] = pseudoRoot(j) * transpose(pseudoRoot(j));
        }
        QL_REQUIRE(i<covariance_.size(),
                   "i (" << i <<
                   ") must be less than covariance_.size() (" << covariance_.size() << ")")
        return covariance_[i];
    }

    const Matrix& MarketModel::totalCovariance(Size endIndex) const {
        if (totalCovariance_.empty()) {
            totalCovariance_.resize(numberOfSteps());
            // call to covariance(0) triggers calculation, if necessary
            // while covariance_[0] would not
            totalCovariance_[0] = covariance(0);
            for (Size j=1; j<numberOfSteps(); ++j)
                totalCovariance_[j] = totalCovariance_[j-1] + covariance_[j];
        }
        QL_REQUIRE(endIndex<covariance_.size(),
                   "endIndex (" << endIndex <<
                   ") must be less than covariance_.size() (" << covariance_.size() << ")")
        return totalCovariance_[endIndex];
    }

   std::vector<Volatility> MarketModel::timeDependentVolatility(Size i) const
   {
        QL_REQUIRE(i<numberOfRates(),
                   "index (" << i << ") must less than number of rates (" <<
                   numberOfRates() << ")");
      
        std::vector<Volatility> result(numberOfSteps());
        const std::vector<Time>& evolutionTime = evolution().evolutionTimes();

        Time lastTime=0.0;
        for (Size j=0; j<numberOfSteps(); ++j) {
            Time tau = evolutionTime[j]-lastTime;
            Real thisVariance = covariance(j)[i][i];
            Real thisVol = std::sqrt(thisVariance/tau);
            result[j] = thisVol;
            lastTime =  evolutionTime[j];
        }
        return result;
   }
     
}
