/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2006 Marco Bianchetti

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
    \brief Abstract Historical correlation between forward rates computation
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

namespace {
    using namespace QuantLib;
    class SafeEvaluationDateBackUp {
      public:
          SafeEvaluationDateBackUp()
              :date_(Settings::instance().evaluationDate()){}
          ~SafeEvaluationDateBackUp() {
              Settings::instance().evaluationDate() = date_;
          }
      private:
          Date date_; 
    };
}


namespace QuantLib {

  template<class Traits, class Interpolator>
    Disposable<Matrix> computeHistoricalCorrelations(
            Date startDate, Date endDate, Period historicalStep,
            bool rollingForwardRatesTimeGrid,
            const Calendar& calendar,
            const boost::shared_ptr<InterestRateIndex> index,
            Period forwardHorizon,
            const std::vector<boost::shared_ptr<IborIndex> >& iborIndexes,
            const std::vector<boost::shared_ptr<SwapIndex> >& swapIndexes,
            Natural depositSettlementDays, Natural swapSettlementDays,
            const DayCounter& yieldCurveDayCounter,
            Real yieldCurveAccuracy) {
        typedef std::vector<boost::shared_ptr<IborIndex> > IborVector;
        typedef std::vector<boost::shared_ptr<SwapIndex> > SwapVector;
        //backuping the evaluation date
        SafeEvaluationDateBackUp backup;
        std::vector<boost::shared_ptr<SimpleQuote> > iborHistoricFixings;
        std::vector<boost::shared_ptr<SimpleQuote> > swapHistoricFixings;
        std::vector<boost::shared_ptr<RateHelper> > rateHelpers;
        IborVector::const_iterator ibor;
        // Set up Depo historical quotes   
        for(ibor=iborIndexes.begin(); ibor!=iborIndexes.end(); ++ibor) {
            boost::shared_ptr<SimpleQuote> quote(new SimpleQuote);
            iborHistoricFixings.push_back(quote);
            Handle<Quote> quoteHandle(quote); 
            rateHelpers.push_back(boost::shared_ptr<RateHelper> (
                new DepositRateHelper(quoteHandle,  
                                      (*ibor)->tenor(),
                                      depositSettlementDays,
                                      (*ibor)->fixingCalendar(),
                                      (*ibor)->businessDayConvention(),
                                      (*ibor)->endOfMonth(),
                                      (*ibor)->fixingDays(),
                                      (*ibor)->dayCounter())));
        } 
        // Set up Swap historical quotes
        SwapVector::const_iterator swap;
        for(swap=swapIndexes.begin(); swap!=swapIndexes.end(); ++swap) {
            boost::shared_ptr<SimpleQuote> quote(new SimpleQuote);
            swapHistoricFixings.push_back(quote);
            Handle<Quote> quoteHandle(quote);
            rateHelpers.push_back(boost::shared_ptr<RateHelper> (
                new SwapRateHelper(quoteHandle,
                                   (*swap)->tenor(),
                                   swapSettlementDays,
                                   (*swap)->fixingCalendar(),
                                   (*swap)->fixedLegTenor().frequency(),
                                   (*swap)->fixedLegConvention(),
                                   (*swap)->dayCounter(),
                                   (*swap)->iborIndex())));
        }
        // Set up the forward rates time grid
        std::vector<Period> forwardFixingPeriods;
        Period indexTenor = index->tenor(); 
        DayCounter indexDayCounter = index->dayCounter();
        Size i=1;
        Period forwardFixingPeriod = i*indexTenor;
        while (forwardFixingPeriod<forwardHorizon) {
            forwardFixingPeriods.push_back(forwardFixingPeriod);
            ++i;
            forwardFixingPeriod=i*indexTenor;
        }
       
        GenericSequenceStatistics<Statistics> statistics;
        std::vector<Rate> forwardRates(forwardFixingPeriods.size());
        std::vector<Rate> forwardRatesPrevious(forwardFixingPeriods.size());
        std::vector<Rate> forwardRatesDifferences(forwardRates.size());

        // Advance date
        Date today = Settings::instance().evaluationDate();
        Date currentDate = calendar.advance(startDate, Period(0, Days), 
                                            Following);
        // Loop over the historical dataset
        bool isFirst = true;
        while(currentDate<=endDate) {
            // we set the evaluationDate equal to today, so that an 
            // error will be raised if a fixing is missing 
            Settings::instance().evaluationDate() = today;
            //we update the quotes...
            for (Size i=0; i<iborIndexes.size(); ++i)
                iborHistoricFixings[i]
                    ->setValue(iborIndexes[i]->fixing(currentDate, false));
            for (Size i=0; i<swapIndexes.size(); ++i)
                swapHistoricFixings[i]
                    ->setValue(swapIndexes[i]->fixing(currentDate, false));
            //move the evaluationDate to currentDate 
            //and update ratehelpers dates ...
            Settings::instance().evaluationDate() = currentDate;
            // Bootstrap the yield curve at the currentDate 
            PiecewiseYieldCurve<Traits, Interpolator>
                    piecewiseYieldCurve(currentDate, rateHelpers, 
                    yieldCurveDayCounter, yieldCurveAccuracy);         
            // Relative forwards: calculate relevant forward rates 
            // on a rolling time grid
            if (rollingForwardRatesTimeGrid) {
                for(Size i=0; i<forwardRates.size(); ++i) {
                    forwardRates[i] = piecewiseYieldCurve.forwardRate(
                        currentDate + forwardFixingPeriods[i],
                        indexTenor, indexDayCounter, Simple);
                }
            // Absolute forwards: calculate relevant forward rates 
            // on a fixed time grid
            } else {
                for(Size i=0; i<forwardRates.size(); ++i) {
                    forwardRates[i] = piecewiseYieldCurve.forwardRate(
                        today + forwardFixingPeriods[i],
                        indexTenor, indexDayCounter, Simple);
                }
            }
            // From 2nd step onwards, calculate forward rate 
            // relative differences
            if (!isFirst)
                for(Size i=0; i<forwardRates.size(); ++i)
                    forwardRatesDifferences[i] = 
                        forwardRates[i]/forwardRatesPrevious[i] - 1.0;
            else
                isFirst = false;
            // Calculate correlations
            statistics.add(forwardRatesDifferences.begin(), 
                           forwardRatesDifferences.end());
            // Store last calculated forward rates
            std::swap(forwardRatesPrevious, forwardRates);
            // Advance date
            currentDate = calendar.advance(currentDate, historicalStep, 
                                           Following);
        } 
        return statistics.correlation();
    }


    //Disposable<Matrix> computeHistoricalCorrelations1 (
    //               Date startDate, Date endDate, Period historicalStep,
    //               const Calendar& calendar,
    //               const boost::shared_ptr<InterestRateIndex> index,
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
    //                    forwardFixingPeriod=i*index->tenor(), ++i)
    //                forwardFixingPeriods.push_back(forwardFixingPeriod);
    //            Date currentDate = startDate;

    //            // loop over the dates in the range
    //            while(currentDate<=endDate) {
    //                Settings::instance().evaluationDate() = currentDate;
    //                for(Size i=0; i<forwardRates.size(); ++i)
    //                    forwardRates[i] 
    //                    = localTermStructure.forwardRate(
    //                            currentDate+forwardFixingPeriods[i],
    //                            indexTenor,indexDayCounter, Simple);
    //                statistics.add(forwardRates.begin(), forwardRates.end());
    //                currentDate = calendar.advance(currentDate, historicalStep, Unadjusted);
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
