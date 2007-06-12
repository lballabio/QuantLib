/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Marco Bianchetti

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

/*! \file model.hpp
    \brief Calculation of historical correlation between forward rates
*/

#ifndef quantlib_historical_correlation_hpp
#define quantlib_historical_correlation_hpp

#include <ql/math/matrix.hpp>
#include <ql/time/calendar.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/termstructures/yieldcurves/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yieldcurves/ratehelpers.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>

namespace QuantLib {

    template<class Traits, class Interpolator>
    Disposable<Matrix> historicalCorrelations(
                const Date& startDate,
                const Date& endDate,
                const Period& step,

                const boost::shared_ptr<InterestRateIndex>& fwdIndex,

                const Period& initialGap,
                const Period& horizon,

                const std::vector<boost::shared_ptr<IborIndex> >& iborIndexes,
                const std::vector<boost::shared_ptr<SwapIndex> >& swapIndexes,

                const DayCounter& yieldCurveDayCounter,
                Real yieldCurveAccuracy) {
        //FIXME: this vector should be passed as an argument
        std::vector<Date> skippedDates;
        //skippedDates.clear();

        SafeSettingsBackUp backup;

        Settings::instance().setEnforceTodaysHistoricFixings();

        std::vector<boost::shared_ptr<RateHelper> > rateHelpers;

        // Create DepositRateHelper
        std::vector<boost::shared_ptr<SimpleQuote> > iborQuotes;
        std::vector<boost::shared_ptr<IborIndex> >::const_iterator ibor;
        for (ibor=iborIndexes.begin(); ibor!=iborIndexes.end(); ++ibor) {
            boost::shared_ptr<SimpleQuote> quote(new SimpleQuote);
            iborQuotes.push_back(quote);
            Handle<Quote> quoteHandle(quote); 
            rateHelpers.push_back(boost::shared_ptr<RateHelper> (new
                DepositRateHelper(quoteHandle,
                                  (*ibor)->tenor(),
                                  (*ibor)->fixingDays(),
                                  (*ibor)->fixingCalendar(),
                                  (*ibor)->businessDayConvention(),
                                  (*ibor)->endOfMonth(),
                                  (*ibor)->fixingDays(),
                                  (*ibor)->dayCounter())));
        }

        // Create SwapRateHelper
        std::vector<boost::shared_ptr<SimpleQuote> > swapQuotes;
        std::vector<boost::shared_ptr<SwapIndex> >::const_iterator swap;
        for (swap=swapIndexes.begin(); swap!=swapIndexes.end(); ++swap) {
            boost::shared_ptr<SimpleQuote> quote(new SimpleQuote);
            swapQuotes.push_back(quote);
            Handle<Quote> quoteHandle(quote);
            rateHelpers.push_back(boost::shared_ptr<RateHelper> (new
                SwapRateHelper(quoteHandle,
                               (*swap)->tenor(),
                               (*swap)->fixingDays(),
                               (*swap)->fixingCalendar(),
                               (*swap)->fixedLegTenor().frequency(),
                               (*swap)->fixedLegConvention(),
                               (*swap)->dayCounter(),
                               (*swap)->iborIndex())));
        }

        // Set up the forward rates time grid
        Period indexTenor = fwdIndex->tenor(); 
        std::vector<Period> fixingPeriods;
        Period fixingPeriod = initialGap;
        while (fixingPeriod<horizon) {
            fixingPeriods.push_back(fixingPeriod);
            fixingPeriod += indexTenor;
        }

        Size nRates = fixingPeriods.size();
        GenericSequenceStatistics<Statistics> statistics(nRates);
        std::vector<Rate> fwdRates(nRates);
        std::vector<Rate> prevFwdRates(nRates);
        std::vector<Rate> fwdRatesDiff(nRates);
        DayCounter indexDayCounter = fwdIndex->dayCounter();
        Calendar cal = fwdIndex->fixingCalendar();

        // Bootstrap the yield curve at the currentDate
        Natural settlementDays = 0;
        PiecewiseYieldCurve<Traits, Interpolator> yc(settlementDays,
                                                     cal,
                                                     rateHelpers,
                                                     yieldCurveDayCounter,
                                                     yieldCurveAccuracy); 

        // Loop over the historical dataset starting with a valid date
        Date currentDate = cal.advance(startDate, 1*Days, Following);
        bool isFirst = true;
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
            } catch (...) {
                skippedDates.push_back(currentDate);
                continue;
            }

            for (Size i=0; i<nRates; ++i) {
                // Time-to-go forwards
                Date d = currentDate + fixingPeriods[i];
                fwdRates[i] = yc.forwardRate(d,
                                             indexTenor,
                                             indexDayCounter,
                                             Simple);
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

        return statistics.correlation();
    }

}

#endif
