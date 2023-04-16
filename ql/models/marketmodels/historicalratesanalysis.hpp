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

/*! \file historicalratesanalysis.hpp
    \brief Statistical analysis of historical rates
*/

#ifndef quantlib_historical_rates_analysis_hpp
#define quantlib_historical_rates_analysis_hpp

#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/time/date.hpp>

namespace QuantLib {

    class InterestRateIndex;

    void historicalRatesAnalysis(
                SequenceStatistics& statistics,
                std::vector<Date>& skippedDates,
                std::vector<std::string>& skippedDatesErrorMessage,
                const Date& startDate,
                const Date& endDate,
                const Period& step,
                const std::vector<std::shared_ptr<InterestRateIndex> >& indexes);

    //! %Historical rate analysis class
    class HistoricalRatesAnalysis {
      public:
        HistoricalRatesAnalysis(std::shared_ptr<SequenceStatistics> stats,
                                const Date& startDate,
                                const Date& endDate,
                                const Period& step,
                                const std::vector<std::shared_ptr<InterestRateIndex> >& indexes);
        const std::vector<Date>& skippedDates() const;
        const std::vector<std::string>& skippedDatesErrorMessage() const;
        const std::shared_ptr<SequenceStatistics>& stats() const;
      private:
        // calculated data
        std::shared_ptr<SequenceStatistics> stats_;
        std::vector<Date> skippedDates_;
        std::vector<std::string> skippedDatesErrorMessage_;
    };

    // inline

    inline const std::vector<Date>&
    HistoricalRatesAnalysis::skippedDates() const {
        return skippedDates_;
    }

    inline const std::vector<std::string>&
    HistoricalRatesAnalysis::skippedDatesErrorMessage() const {
        return skippedDatesErrorMessage_;
    }

    inline const std::shared_ptr<SequenceStatistics>&
    HistoricalRatesAnalysis::stats() const {
        return stats_;
    }

}

#endif
