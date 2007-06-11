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
            fixingPeriod = fixingPeriod + indexTenor;
        }

        Size nRates = fixingPeriods.size();
        GenericSequenceStatistics<Statistics> statistics(nRates);
        std::vector<Rate> fwdRates(nRates);
        std::vector<Rate> prevFwdRates(nRates);
        std::vector<Rate> fwdRatesDiff(nRates);
        DayCounter indexDayCounter = fwdIndex->dayCounter();
        Calendar cal = fwdIndex->fixingCalendar();

        // Loop over the historical dataset starting with a valid date
        Date currentDate = cal.advance(startDate, 0*Days, Following);
        bool isFirst = true;
        while (currentDate<=endDate) {

            // move the evaluationDate to currentDate 
            // and update ratehelpers dates...
            Settings::instance().evaluationDate() = currentDate;

            // update the quotes...
            for (Size i=0; i<iborIndexes.size(); ++i) {
                Rate fixing = iborIndexes[i]->fixing(currentDate, false);
                iborQuotes[i]->setValue(fixing);
            }
            for (Size i=0; i<swapIndexes.size(); ++i) {
                Rate fixing = swapIndexes[i]->fixing(currentDate, false);
                swapQuotes[i]->setValue(fixing);
            }

            // Bootstrap the yield curve at the currentDate 
            PiecewiseYieldCurve<Traits, Interpolator> yc(currentDate,
                                                         rateHelpers,
                                                         yieldCurveDayCounter,
                                                         yieldCurveAccuracy);         
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
            if (isFirst)
                isFirst = false;
            else {
                for (Size i=0; i<nRates; ++i)
                    fwdRatesDiff[i] = fwdRates[i]/prevFwdRates[i] -1.0;
                // add observation
                statistics.add(fwdRatesDiff.begin(), fwdRatesDiff.end());
            }

            // Store last calculated forward rates
            std::swap(prevFwdRates, fwdRates);

            // Advance date
            currentDate = cal.advance(currentDate, step, Following);
        } 

        return statistics.correlation();
    }


    //Disposable<Matrix> computeHistoricalCorrelations1 (
    //               Date startDate, Date endDate, Period step,
    //               const Calendar& calendar,
    //               const boost::shared_ptr<InterestRateIndex> fwdIndex,
    //               Period forwardHorizon,
    //               const YieldTermStructure& termStructure){
    //        YieldTermStructure& termStructure_ 
    //            = const_cast<YieldTermStructure&>(termStructure);
    //        bool useFixings_ = termStructure_.useFixings();
    //        try {
    //            termStructure_.useFixings() = true;
    //            YieldTermStructure localTermStructure(termStructure);
    //            
    //            // build the tenors at which we will compute the forward rates
    //            std::vector<Period> forwardFixingPeriods;
    //            for (Size i=1; forwardFixingPeriod<forwardHorizon;
    //                    forwardFixingPeriod=i*fwdIndex->tenor(), ++i)
    //                forwardFixingPeriods.push_back(forwardFixingPeriod);
    //            Date currentDate = startDate;

    //            // loop over the dates in the range
    //            while(currentDate<=endDate) {
    //                Settings::instance().evaluationDate() = currentDate;
    //                for(Size i=0; i<fwdRates.size(); ++i)
    //                    fwdRates[i] 
    //                    = localTermStructure.forwardRate(
    //                            currentDate+forwardFixingPeriods[i],
    //                            indexTenor,indexDayCounter, Simple);
    //                statistics.add(fwdRates.begin(), fwdRates.end());
    //                currentDate = calendar.advance(currentDate, step, Unadjusted);
    //            }
    //            termStructure_.useFixings() = useFixings_;
    //            return statistics.correlation();
    //    } catch(QuantLib::Error&e) {
    //        termStructure_.useFixings() = useFixings_;
    //        QL_FAIL("computeHistoricalCorrelations1\n" << e.what());
    //    }
    //}

}

#endif
