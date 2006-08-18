/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/MarketModels/PseudoRoots/abcdvolatility.hpp>
#include <ql/Math/pseudosqrt.hpp>

namespace QuantLib {
    
    AbcdVolatility::AbcdVolatility(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            Real longTermCorr,
            Real beta,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const Array& initialRates,
            const Array& displacements)
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
      
        Time lastEvolutionTime = 0.0;
        Time currentEvolutionTime=0.0;
        for (Size k=0; k<evolutionTimes_.size(); ++k) 
        {
            lastEvolutionTime=currentEvolutionTime;
            currentEvolutionTime=evolutionTimes_[k];

            for (Size i=0; i<n; ++i) {
                for (Size j=i; j<n; ++j) {
                    double effStartTime = std::min(rateTimes_[i],lastEvolutionTime); 
                    double effStopTime = std::min(rateTimes_[i],currentEvolutionTime);
                    Real correlation = longTermCorr + (1.0-longTermCorr) * 
                         std::exp(-beta*std::abs(rateTimes_[i]-rateTimes_[j]));
                    boost::shared_ptr<Abcd> abcd(new Abcd(b, c, d, a, rateTimes_[i], rateTimes_[j]));
                    double covar = abcd->primitive(effStopTime) - abcd->primitive(effStartTime);
                    covariance[j][i] = covariance[i][j] = ks_[i] * ks_[j] * covar * correlation ;
                 }
             }

            pseudoRoots_[k]=
                //rankReducedSqrt(covariance, numberOfFactors, 1.0,
                //                SalvagingAlgorithm::None);
                pseudoSqrt(covariance, SalvagingAlgorithm::None);
       
        }
       
    }



    const Array& AbcdVolatility::initialRates() const
    {
        return initialRates_;
    }

    const Array& AbcdVolatility::displacements() const
    {
        return displacements_;
    }

    Size AbcdVolatility::numberOfRates() const 
    {
        return initialRates_.size();
    }

    Size AbcdVolatility::numberOfFactors() const
    {
        return numberOfFactors_;
    }

    const Matrix& AbcdVolatility::pseudoRoot(Size i) const 
    {
        return pseudoRoots_[i];
    }

    Abcd::Abcd(Real a, Real b, Real c, Real d, Real T, Real S)
    : a_(a), b_(b), c_(c), d_(d), S_(S), T_(T) {
        QL_REQUIRE(a+d>=0, "a+d must be non negative");
        QL_REQUIRE(d>=0, "d must be non negative");
        QL_REQUIRE(c>=0, "c must be non negative");
    }

    Real Abcd::shortTermVolatility() const {
        return a_+d_;
    }

    Real Abcd::longTermVolatility() const {
        return d_;
    }

    Real Abcd::maximumLocation() const {
        return (b_>0.0 ? (b_-c_*a_)/(c_*b_) : 0.0);
    }

    Real Abcd::maximumVolatility() const {
        return (b_>0.0 ? b_/c_*std::exp(-1.0 +c_*a_/b_)+d_ : a_+d_);
    }

}

