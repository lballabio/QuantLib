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

#include <ql/MarketModels/abcdvolatility.hpp>
#include <ql/Math/pseudosqrt.hpp>

namespace QuantLib {

    namespace {

        Real primitive(Time T, Time S, Time u,
                       Real a, Real b, Real c, Real d) {

            const Real k1=std::exp(b*u);
            const Real k2=std::exp(b*S);
            const Real k3=std::exp(b*T);

            return (a*a*(-1 - 2*b*b*S*T - b*(S + T)
                         + k1*k1*(1 + b*(S + T - 2*u) + 2*b*b*(S - u)*(T - u)))
                    + 2*b*b*(2*c*d*(k2 + k3)*(k1 - 1)
                             +d*d*(k1*k1 - 1)+2*b*c*c*k2*k3*u)
                    + 2*a*b*(d*(-1 - b*(S + T) + k1*k1*(1 + b*(S + T - 2*u)))
                             -2*c*(k3*(1 + b*S) + k2*(1 + b*T)
                                   - k1*k3*(1 + b*(S - u))
                                   - k1*k2*(1 + b*(T - u)))
                             )
                    ) / (4*b*b*b*k2*k3);
        }

    }

    AbcdVolatility::AbcdVolatility(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            Real longTermCorr,
            Real beta,
            const Array& rateTimes,
            const Array& evolutionTimes,
            const Size numberOfFactors,
            const Array& initialRates,
            const Array& displacements)
     :  a_(a), b_(b), c_(c), d_(d), ks_(ks),
        longTermCorr_(longTermCorr), beta_(beta), 
        rateTimes_(rateTimes), evolutionTimes_(evolutionTimes),
        numberOfFactors_(numberOfFactors),
        initialRates_(initialRates),displacements_(displacements),
        covariance_(ks.size(),ks.size()),
        pseudoRoots_(evolutionTimes.size())
    {
        Size n=ks.size();
        QL_REQUIRE(n==rateTimes.size()-1, "rateTimes/ks mismatch");

        std::vector<Time> stdDev(n);
      
        Time lastEvolutionTime = 0.0;
        Time currentEvolutionTime=0.0;
        for (Size k=0; k<evolutionTimes.size(); ++k) 
        {
            lastEvolutionTime=currentEvolutionTime;
            currentEvolutionTime=evolutionTimes[k];

            for (Size i=0; i<n; ++i) {
                for (Size j=i; j<n; ++j) {
                    double effStartTime = std::min(rateTimes[i],lastEvolutionTime); 
                    double effStopTime = std::min(rateTimes[i],currentEvolutionTime);
                    Real correlation = longTermCorr + (1.0-longTermCorr) * 
                         std::exp(-beta*std::abs(rateTimes[i]-rateTimes[j]));
                     
                    double covar = primitive(rateTimes[i], rateTimes[j], effStopTime,
                                             b, c, d, a) 
                                 - primitive(rateTimes[i], rateTimes[j], effStartTime,
                                             b, c, d, a);

                     covariance_[j][i] = covariance_[i][j] = ks_[i] * ks_[j] * covar * correlation ;
                 }
             }

            pseudoRoots_[k]=
                pseudoSqrt(covariance_, SalvagingAlgorithm::None);
                //rankReducedSqrt(covariance_, numberOfFactors, 1.0,
                //                SalvagingAlgorithm::Spectral);
       
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

}

