/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2003 RiskMap srl

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

#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/math/functional.hpp>

namespace QuantLib {

    Real GeneralStatistics::weightSum() const {
        Real result = 0.0;
        std::vector<std::pair<Real,Real> >::const_iterator it;
        for (it=samples_.begin(); it!=samples_.end(); ++it) {
            result += it->second;
        }
        return result;
    }

    Real GeneralStatistics::mean() const {
        Size N = samples();
        QL_REQUIRE(N != 0, "empty sample set");
        // eat our own dog food
        return expectationValue(identity<Real>(),
                                everywhere()).first;
    }

    Real GeneralStatistics::variance() const {
        Size N = samples();
        QL_REQUIRE(N > 1,
                   "sample number <=1, unsufficient");
        // Subtract the mean and square. Repeat on the whole range.
        // Hopefully, the whole thing will be inlined in a single loop.
        Real s2 = expectationValue(compose(square<Real>(),
                                           subtract<Real>(mean())),
                                   everywhere()).first;
        return s2*N/(N-1.0);
    }

    Real GeneralStatistics::skewness() const {
        Size N = samples();
        QL_REQUIRE(N > 2,
                   "sample number <=2, unsufficient");

        Real x = expectationValue(compose(cube<Real>(),
                                          subtract<Real>(mean())),
                                  everywhere()).first;
        Real sigma = standardDeviation();

        return (x/(sigma*sigma*sigma))*(N/(N-1.0))*(N/(N-2.0));
    }

    Real GeneralStatistics::kurtosis() const {
        Size N = samples();
        QL_REQUIRE(N > 3,
                   "sample number <=3, unsufficient");

        Real x = expectationValue(compose(fourth_power<Real>(),
                                          subtract<Real>(mean())),
                                  everywhere()).first;
        Real sigma2 = variance();

        Real c1 = (N/(N-1.0)) * (N/(N-2.0)) * ((N+1.0)/(N-3.0));
        Real c2 = 3.0 * ((N-1.0)/(N-2.0)) * ((N-1.0)/(N-3.0));

        return c1*(x/(sigma2*sigma2))-c2;
    }

    Real GeneralStatistics::percentile(Real percent) const {

        QL_REQUIRE(percent > 0.0 && percent <= 1.0,
                   "percentile (" << percent << ") must be in (0.0, 1.0]");

        Real sampleWeight = weightSum();
        QL_REQUIRE(sampleWeight>0.0,
                   "empty sample set");

        sort();

        std::vector<std::pair<Real,Real> >::iterator k, l;
        k = samples_.begin();
        l = samples_.end()-1;
        /* the sum of weight is non null, therefore there's
           at least one sample */
        Real integral = k->second, target = percent*sampleWeight;
        while (integral < target && k != l) {
            ++k;
            integral += k->second;
        }
        return k->first;
    }

    Real GeneralStatistics::topPercentile(Real percent) const {

        QL_REQUIRE(percent > 0.0 && percent <= 1.0,
                   "percentile (" << percent << ") must be in (0.0, 1.0]");

        Real sampleWeight = weightSum();
        QL_REQUIRE(sampleWeight > 0.0,
                   "empty sample set");

        sort();

        std::vector<std::pair<Real,Real> >::reverse_iterator k, l;
        k = samples_.rbegin();
        l = samples_.rend()-1;
        /* the sum of weight is non null, therefore there's
           at least one sample */
        Real integral = k->second, target = percent*sampleWeight;
        while (integral < target && k != l) {
            ++k;
            integral += k->second;
        }
        return k->first;
    }

}
