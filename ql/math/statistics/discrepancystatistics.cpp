/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/math/statistics/discrepancystatistics.hpp>

namespace QuantLib {

    Real DiscrepancyStatistics::discrepancy() const {
        Size N = samples();
        /*
        Size i;
        Real r_ik, r_jk, cdiscr = adiscr = 0.0, temp = 1.0;

        for (i=0; i<N; i++) {
            Real temp = 1.0;
            for (Size k=0; k<dimension_; k++) {
                r_ik = stats_[k].sampleData()[i].first;
                temp *= (1.0 - r_ik*r_ik);
            }
            cdiscr += temp;
        }

        for (i=0; i<N; i++) {
            for (Size j=0; j<N; j++) {
                Real temp = 1.0;
                for (Size k=0; k<dimension_; k++) {
                    r_jk = stats_[k].sampleData()[j].first;
                    r_ik = stats_[k].sampleData()[i].first;
                    temp *= (1.0 - std::max(r_ik, r_jk));
                }
                adiscr += temp;
            }
        }
        */
        return std::sqrt(adiscr_/(N*N)-bdiscr_/N*cdiscr_+ddiscr_);
    }

}


