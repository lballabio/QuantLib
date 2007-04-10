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

#include <ql/marketmodels/models/correlations.hpp>
#include <ql/marketmodels/evolutiondescription.hpp>


namespace QuantLib {
    Disposable<Matrix> exponentialCorrelations(Real longTermCorr,
                                   Real beta,
                                   const EvolutionDescription& evolution){
        const std::vector<Rate>& rateTimes = evolution.rateTimes();
        Size nbRates = rateTimes.size();
        Matrix correlations(nbRates, nbRates);
        for (Size i = 0; i < nbRates; ++i)
            for (Size j = 0; j < nbRates; ++j)
                correlations[i][j] = longTermCorr + (1.0-longTermCorr) *
                    std::exp(-beta*std::fabs(rateTimes[i]-rateTimes[j]));
        return correlations;
    }
}
