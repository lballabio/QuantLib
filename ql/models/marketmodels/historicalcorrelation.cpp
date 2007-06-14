/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Katiuscia Manzoni

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

#include <ql/models/marketmodels/historicalcorrelation.hpp>

namespace QuantLib {

      HistoricalCorrelation::HistoricalCorrelation(const Date& startDate,
                          const Date& endDate,
                          const Period& step,
                          const boost::shared_ptr<InterestRateIndex>& fwdIndex,
                          const Period& initialGap,
                          const Period& horizon,
                          const std::vector<boost::shared_ptr<IborIndex> >& iborIndexes,
                          const std::vector<boost::shared_ptr<SwapIndex> >& swapIndexes,
                          const DayCounter& yieldCurveDayCounter,
                          Real yieldCurveAccuracy) {
        historicalForwardRatesAnalysis<QuantLib::ZeroYield, QuantLib::Linear>(
            stats_,
            skippedDates_, failedDates_, fixingPeriods_,
            startDate, endDate, step, fwdIndex, initialGap, horizon,
            iborIndexes, swapIndexes,
            yieldCurveDayCounter, yieldCurveAccuracy);
      }

}
