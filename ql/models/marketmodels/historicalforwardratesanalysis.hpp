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

/*! \file historicalforwardratesanalysis.hpp
    \brief Statistical analysis of historical forward rates
*/

#ifndef quantlib_historical_forward_rates_analysis_hpp
#define quantlib_historical_forward_rates_analysis_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>
#include <utility>

namespace QuantLib {

    template<class Traits, class Interpolator>
    void historicalForwardRatesAnalysis(
                SequenceStatistics& statistics,
                std::vector<Date>& skippedDates,
                std::vector<std::string>& skippedDatesErrorMessage,
                std::vector<Date>& failedDates,
                std::vector<std::string>& failedDatesErrorMessage,
                std::vector<Period>& fixingPeriods,
                const Date& startDate,
                const Date& endDate,
                const Period& step,
                const ext::shared_ptr<InterestRateIndex>& fwdIndex,
                const Period& initialGap,
                const Period& horizon,
                const std::vector<ext::shared_ptr<IborIndex> >& iborIndexes,
                const std::vector<ext::shared_ptr<SwapIndex> >& swapIndexes,
                const DayCounter& yieldCurveDayCounter,
                Real yieldCurveAccuracy = 1.0e-12,
                const Interpolator& i = Interpolator()) {


        statistics.reset();
        skippedDates.clear();
        skippedDatesErrorMessage.clear();
        failedDates.clear();
        failedDatesErrorMessage.clear();
        fixingPeriods.clear();

        SavedSettings backup;
        Settings::instance().enforcesTodaysHistoricFixings() = true;

        std::vector<ext::shared_ptr<RateHelper> > rateHelpers;

        // Create DepositRateHelper
        std::vector<ext::shared_ptr<SimpleQuote> > iborQuotes;
        std::vector<ext::shared_ptr<IborIndex> >::const_iterator ibor;
        for (ibor=iborIndexes.begin(); ibor!=iborIndexes.end(); ++ibor) {
            ext::shared_ptr<SimpleQuote> quote(new SimpleQuote);
            iborQuotes.push_back(quote);
            Handle<Quote> quoteHandle(quote);
            rateHelpers.push_back(ext::shared_ptr<RateHelper> (new
                DepositRateHelper(quoteHandle,
                                  (*ibor)->tenor(),
                                  (*ibor)->fixingDays(),
                                  (*ibor)->fixingCalendar(),
                                  (*ibor)->businessDayConvention(),
                                  (*ibor)->endOfMonth(),
                                  (*ibor)->dayCounter())));
        }

        // Create SwapRateHelper
        std::vector<ext::shared_ptr<SimpleQuote> > swapQuotes;
        std::vector<ext::shared_ptr<SwapIndex> >::const_iterator swap;
        for (swap=swapIndexes.begin(); swap!=swapIndexes.end(); ++swap) {
            ext::shared_ptr<SimpleQuote> quote(new SimpleQuote);
            swapQuotes.push_back(quote);
            Handle<Quote> quoteHandle(quote);
            rateHelpers.push_back(ext::shared_ptr<RateHelper> (new
                SwapRateHelper(quoteHandle,
                               (*swap)->tenor(),
                               (*swap)->fixingCalendar(),
                               (*swap)->fixedLegTenor().frequency(),
                               (*swap)->fixedLegConvention(),
                               (*swap)->dayCounter(),
                               (*swap)->iborIndex())));
        }

        // Set up the forward rates time grid
        Period indexTenor = fwdIndex->tenor();
        Period fixingPeriod = initialGap;
        while (fixingPeriod<=horizon) {
            fixingPeriods.push_back(fixingPeriod);
            fixingPeriod += indexTenor;
        }

        Size nRates = fixingPeriods.size();
        statistics.reset(nRates);
        std::vector<Rate> fwdRates(nRates);
        std::vector<Rate> prevFwdRates(nRates);
        std::vector<Rate> fwdRatesDiff(nRates);
        DayCounter indexDayCounter = fwdIndex->dayCounter();
        Calendar cal = fwdIndex->fixingCalendar();

        // Bootstrap the yield curve at the currentDate
        Natural settlementDays = 0;
        typename PiecewiseYieldCurve<Traits, Interpolator>::bootstrap_type bootstrap(yieldCurveAccuracy);
        PiecewiseYieldCurve<Traits, Interpolator> yc(settlementDays,
                                                     cal,
                                                     rateHelpers,
                                                     yieldCurveDayCounter,
                                                     std::vector<Handle<Quote> >(),
                                                     std::vector<Date>(),
                                                     i,
                                                     bootstrap);

        // start with a valid business date
        Date currentDate = cal.advance(startDate, 1*Days, Following);
        bool isFirst = true;
        // Loop over the historical dataset
        for (; currentDate<=endDate;
            currentDate = cal.advance(currentDate, step, Following)) {

            // move the evaluationDate to currentDate
            // and update ratehelpers dates...
            Settings::instance().evaluationDate() = currentDate;

            try {
                // update the quotes...
                for (Size i=0; i<iborIndexes.size(); ++i) {
                    Rate fixing = iborIndexes[i]->fixing(currentDate, false);
                    iborQuotes[i]->setValue(fixing);
                }
                for (Size i=0; i<swapIndexes.size(); ++i) {
                    Rate fixing = swapIndexes[i]->fixing(currentDate, false);
                    swapQuotes[i]->setValue(fixing);
                }
            } catch (std::exception& e) {
                skippedDates.push_back(currentDate);
                skippedDatesErrorMessage.emplace_back(e.what());
                continue;
            }

            try {
                for (Size i=0; i<nRates; ++i) {
                    // Time-to-go forwards
                    Date d = currentDate + fixingPeriods[i];
                    fwdRates[i] = yc.forwardRate(d,
                                                 indexTenor,
                                                 indexDayCounter,
                                                 Simple);
                }
            } catch (std::exception& e) {
                failedDates.push_back(currentDate);
                failedDatesErrorMessage.emplace_back(e.what());
                continue;
            }

            // From 2nd step onwards, calculate forward rate
            // relative differences
            if (!isFirst){
                for (Size i=0; i<nRates; ++i)
                    fwdRatesDiff[i] = fwdRates[i]/prevFwdRates[i] -1.0;
                // add observation
                statistics.add(fwdRatesDiff.begin(), fwdRatesDiff.end());
            }
            else
                isFirst = false;

            // Store last calculated forward rates
            std::swap(prevFwdRates, fwdRates);

        }
    }

    class HistoricalForwardRatesAnalysis {
      public:
        virtual ~HistoricalForwardRatesAnalysis() = default;
        virtual const std::vector<Date>& skippedDates() const = 0;
        virtual const std::vector<std::string>& skippedDatesErrorMessage() const = 0;
        virtual const std::vector<Date>& failedDates() const = 0;
        virtual const std::vector<std::string>& failedDatesErrorMessage() const = 0;
        virtual const std::vector<Period>& fixingPeriods() const = 0;
    };

    //! %Historical correlation class
    template<class Traits, class Interpolator>
    class HistoricalForwardRatesAnalysisImpl : public HistoricalForwardRatesAnalysis {
      public:
        HistoricalForwardRatesAnalysisImpl(
            ext::shared_ptr<SequenceStatistics> stats,
            const Date& startDate,
            const Date& endDate,
            const Period& step,
            const ext::shared_ptr<InterestRateIndex>& fwdIndex,
            const Period& initialGap,
            const Period& horizon,
            const std::vector<ext::shared_ptr<IborIndex> >& iborIndexes,
            const std::vector<ext::shared_ptr<SwapIndex> >& swapIndexes,
            const DayCounter& yieldCurveDayCounter,
            Real yieldCurveAccuracy);
        HistoricalForwardRatesAnalysisImpl() = default;
        ;
        const std::vector<Date>& skippedDates() const override;
        const std::vector<std::string>& skippedDatesErrorMessage() const override;
        const std::vector<Date>& failedDates() const override;
        const std::vector<std::string>& failedDatesErrorMessage() const override;
        const std::vector<Period>& fixingPeriods() const override;
        //const ext::shared_ptr<SequenceStatistics>& stats() const;
      private:
        // calculated data
        ext::shared_ptr<SequenceStatistics> stats_;
        std::vector<Date> skippedDates_;
        std::vector<std::string> skippedDatesErrorMessage_;
        std::vector<Date> failedDates_;
        std::vector<std::string> failedDatesErrorMessage_;
        std::vector<Period> fixingPeriods_;
    };

    // inline
    template<class Traits, class Interpolator>
    const std::vector<Period>&
    HistoricalForwardRatesAnalysisImpl<Traits, Interpolator>::fixingPeriods() const {
        return fixingPeriods_;
    }

    template<class Traits, class Interpolator>
    inline const std::vector<Date>&
    HistoricalForwardRatesAnalysisImpl<Traits, Interpolator>::skippedDates() const {
        return skippedDates_;
    }

    template<class Traits, class Interpolator>
    inline const std::vector<std::string>&
    HistoricalForwardRatesAnalysisImpl<Traits, Interpolator>::skippedDatesErrorMessage() const {
        return skippedDatesErrorMessage_;
    }

    template<class Traits, class Interpolator>
    inline const std::vector<Date>&
    HistoricalForwardRatesAnalysisImpl<Traits, Interpolator>::failedDates() const {
        return failedDates_;
    }

    template<class Traits, class Interpolator>
    inline const std::vector<std::string>&
    HistoricalForwardRatesAnalysisImpl<Traits, Interpolator>::failedDatesErrorMessage() const {
        return failedDatesErrorMessage_;
    }

    //inline const ext::shared_ptr<SequenceStatistics>&
    //HistoricalForwardRatesAnalysis::stats() const {
    //    return stats_;
    //}
    template <class Traits, class Interpolator>
    HistoricalForwardRatesAnalysisImpl<Traits, Interpolator>::HistoricalForwardRatesAnalysisImpl(
        ext::shared_ptr<SequenceStatistics> stats,
        const Date& startDate,
        const Date& endDate,
        const Period& step,
        const ext::shared_ptr<InterestRateIndex>& fwdIndex,
        const Period& initialGap,
        const Period& horizon,
        const std::vector<ext::shared_ptr<IborIndex> >& iborIndexes,
        const std::vector<ext::shared_ptr<SwapIndex> >& swapIndexes,
        const DayCounter& yieldCurveDayCounter,
        Real yieldCurveAccuracy)
    : stats_(std::move(stats)) {
        historicalForwardRatesAnalysis<Traits,
                                       Interpolator>(
                    *stats_,
                    skippedDates_, skippedDatesErrorMessage_,
                    failedDates_, failedDatesErrorMessage_,
                    fixingPeriods_, startDate, endDate, step,
                    fwdIndex, initialGap, horizon,
                    iborIndexes, swapIndexes,
                    yieldCurveDayCounter, yieldCurveAccuracy);
    }
}

#endif


#ifndef id_38d57f4fe85b163c863b5aeb95c4421b
#define id_38d57f4fe85b163c863b5aeb95c4421b
inline bool test_38d57f4fe85b163c863b5aeb95c4421b(int* i) { return i != 0; }
#endif
