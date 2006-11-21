/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti
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

#include <ql/MarketModels/curvestate.hpp>

namespace QuantLib {

    // Computation of coterminal swap rates and annuities
    // from discount ratios and fwd rates
    void CurveState::computeCoterminalSwap(Size i) const {
        Real accumAnn = 0.0;
        if (firstCotSwap_!=nRates_)
            accumAnn = cotAnnuities_[firstCotSwap_];
        while (firstCotSwap_>i) {
            --firstCotSwap_;
            accumAnn += taus_[firstCotSwap_] * discRatios_[firstCotSwap_+1];
            cotAnnuities_[firstCotSwap_] = accumAnn;
            cotSwaps_[firstCotSwap_] =
                (discRatios_[firstCotSwap_]-discRatios_[nRates_])/accumAnn;
        }
    }

}
