/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/MarketModels/PseudoRoots/expcorrabcdvol.hpp>
#include <ql/Math/pseudosqrt.hpp>

namespace QuantLib {
    
    ExpCorrAbcdVol::ExpCorrAbcdVol(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            Real longTermCorr,
            Real beta,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Rate>& displacements)
     :  a_(a), b_(b), c_(c), d_(d), ks_(ks),
        longTermCorr_(longTermCorr), beta_(beta), 
        rateTimes_(evolution.rateTimes()),
        evolutionTimes_(evolution.evolutionTimes()),
        numberOfFactors_(numberOfFactors),
        initialRates_(initialRates),
        displacements_(displacements),
        pseudoRoots_(evolution.evolutionTimes().size())
    {

        Size n=ks.size();
        QL_REQUIRE(n==rateTimes_.size()-1, "rateTimes/ks mismatch");

        Matrix covariance(n, n);
        std::vector<Time> stdDev(n);
      
        Time effStartTime;
        Real correlation, covar;
        for (Size k=0; k<evolutionTimes_.size(); ++k) {
            for (Size i=0; i<n; ++i) {
                for (Size j=i; j<n; ++j) {
                    correlation = longTermCorr + (1.0-longTermCorr) * 
                         std::exp(-beta*std::abs(rateTimes_[i]-rateTimes_[j]));
                    boost::shared_ptr<Abcd> abcd(new Abcd(a, b, c, d, rateTimes_[i], rateTimes_[j]));
                    effStartTime = k>0 ? evolution.effectiveStopTime()[k-1][i] : 0.0;
                    covar = abcd->covariance(effStartTime,evolution.effectiveStopTime()[k][i]);
                    covariance[j][i] = covariance[i][j] = ks_[i] * ks_[j] * covar * correlation ;
                 }
             }

            pseudoRoots_[k]=
                //rankReducedSqrt(covariance, numberOfFactors, 1.0,
                //                SalvagingAlgorithm::None);
                pseudoSqrt(covariance, SalvagingAlgorithm::None);
       
        }
       
    }

}
