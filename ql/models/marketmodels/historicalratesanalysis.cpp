/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/models/marketmodels/historicalratesanalysis.hpp>
#include <ql/time/calendar.hpp>
#include <ql/indexes/interestrateindex.hpp>

namespace QuantLib {

    void historicalRatesAnalysis(
                SequenceStatistics& statistics,
                std::vector<Date>& skippedDates,
                std::vector<std::string>& skippedDatesErrorMessage,
                const Date& startDate,
                const Date& endDate,
                const Period& step,
                const std::vector<boost::shared_ptr<InterestRateIndex> >& indexes) {

        skippedDates.clear();
        skippedDatesErrorMessage.clear();

        Size nRates = indexes.size();
        statistics.reset(nRates);

        std::vector<Rate> sample(nRates);
        std::vector<Rate> prevSample(nRates);
        std::vector<Rate> sampleDiff(nRates);

        Calendar cal = indexes[0]->fixingCalendar();
        // start with a valid business date
        Date currentDate = cal.advance(startDate, 1*Days, Following);
        bool isFirst = true;
        // Loop over the historical dataset
        for (; currentDate<=endDate;
            currentDate = cal.advance(currentDate, step, Following)) {

            try {
                for (Size i=0; i<nRates; ++i) {
                    Rate fixing = indexes[i]->fixing(currentDate, false);
                    sample[i] = fixing;
                }
            } catch (std::exception& e) {
                skippedDates.push_back(currentDate);
                skippedDatesErrorMessage.push_back(e.what());
                continue;
            }

            // From 2nd step onwards, calculate forward rate
            // relative differences
            if (!isFirst){
                for (Size i=0; i<nRates; ++i)
                    sampleDiff[i] = sample[i]/prevSample[i] -1.0;
                // add observation
                statistics.add(sampleDiff.begin(), sampleDiff.end());
            }
            else
                isFirst = false;

            // Store last calculated forward rates
            std::swap(prevSample, sample);

        }
    }

    HistoricalRatesAnalysis::HistoricalRatesAnalysis(
                const boost::shared_ptr<SequenceStatistics>& stats,
                const Date& startDate,
                const Date& endDate,
                const Period& step,
                const std::vector<boost::shared_ptr<InterestRateIndex> >& indexes)
    : stats_(stats) {
        historicalRatesAnalysis(
                    *stats_,
                    skippedDates_, skippedDatesErrorMessage_,
                    startDate, endDate, step,
                    indexes);
      }
}
