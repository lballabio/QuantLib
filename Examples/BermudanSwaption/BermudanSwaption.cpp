/*!
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

//! $Id$

#include <ql/quantlib.hpp>

using namespace QuantLib;
using namespace QuantLib::Instruments;
using namespace QuantLib::ShortRateModels;
using namespace QuantLib::Pricers;

using CalibrationHelpers::CapHelper;
using CalibrationHelpers::SwaptionHelper;

using DayCounters::ActualActual;
using DayCounters::Actual360;
using DayCounters::Thirty360;
using Indexes::Xibor;
using Indexes::Euribor;

using TermStructures::PiecewiseFlatForward;
using TermStructures::RateHelper;
using TermStructures::DepositRateHelper;
using TermStructures::SwapRateHelper;

//Number of swaptions to be calibrated to...

Size numRows = 5;
Size numCols = 10;

unsigned int swaptionLengths[] = {1,2,3,4,5,7,10,15,20,25,30};
double swaptionVols[] = {
    23.92, 22.80, 19.8, 18.1, 16.0, 14.26, 13.56, 12.79, 12.3, 11.09,
    21.85, 21.50, 19.5, 17.2, 14.7, 13.23, 12.59, 12.29, 11.1, 10.30,
    19.46, 19.40, 17.9, 15.9, 13.9, 12.69, 12.15, 11.83, 10.8, 10.00,
    17.97, 17.80, 16.7, 14.9, 13.4, 12.28, 11.89, 11.48, 10.5,  9.80,
    16.29, 16.40, 15.1, 14.0, 12.9, 12.01, 11.46, 11.08, 10.4,  9.77,
    14.71, 14.90, 14.3, 13.2, 12.3, 11.49, 11.12, 10.70, 10.1,  9.57,
    12.93, 13.30, 12.8, 12.2, 11.6, 10.82, 10.47, 10.21,  9.8,  9.51,
    12.70, 12.10, 11.9, 11.2, 10.8, 10.40, 10.20, 10.00,  9.5,  9.00,
    12.30, 11.60, 11.6, 10.9, 10.5, 10.30, 10.00,  9.80,  9.3,  8.80,
    12.00, 11.40, 11.5, 10.8, 10.3, 10.00,  9.80,  9.60,  9.5,  9.10,
    11.50, 11.20, 11.3, 10.6, 10.2, 10.10,  9.70,  9.50,  9.4,  8.60};

void calibrateModel(const Handle<Model>& model,
                    CalibrationSet& calibs,
                    double lambda) {

    Handle<Optimization::Method> om(new Optimization::Simplex(lambda, 1e-9));
    om->setEndCriteria(Optimization::EndCriteria(10000, 1e-7));
    model->calibrate(calibs, om);

    //Output the implied Black volatilities
    Size i;
    for (i=0; i<numRows; i++) {
        std::cout << IntegerFormatter::toString(swaptionLengths[i],2) << "y|";
        for (Size j=0; j<numCols; j++) {
            Size k = i*numCols + j;
            double npv = calibs[k]->modelValue();
            double implied = calibs[k]->impliedVolatility(npv, 1e-4,
                1000, 0.05, 0.50)*100.0;
            std::cout << DoubleFormatter::toString(implied,1,4) << " (";
            k = i*10 + j;
            double diff = implied - swaptionVols[k];
            std::cout << DoubleFormatter::toString(diff,1,4)
                      << ")|";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[])
{
    try {
        Date todaysDate(15, February, 2002);
        Calendar calendar = Calendars::TARGET();
        int settlementDays = 2;
        Currency currency = EUR;

        //Instruments used to bootstrap the yield curve:
        std::vector<Handle<RateHelper> > instruments;

        //Deposit rates
        DayCounter depositDayCounter = Thirty360();

        Rate weekRates[3] = {3.295, 3.3, 3.3};
        Size i;
        for (i=0; i<3; i++) {
            Handle<MarketElement> depositRate(
                new SimpleMarketElement(weekRates[i]*0.01));
            Handle<RateHelper> depositHelper(new DepositRateHelper(
                depositRate, settlementDays, i+1, Weeks, calendar,
                ModifiedFollowing, depositDayCounter));
            instruments.push_back(depositHelper);
        }

        Rate depositRates[12] = {
            3.31, 3.32, 3.29, 3.35, 3.315, 3.33,
            3.395, 3.41, 3.41, 3.49, 3.54, 3.53};

        for (i=0; i<11; i++) {
            Handle<MarketElement> depositRate(
                new SimpleMarketElement(depositRates[i]*0.01));
            Handle<RateHelper> depositHelper(new DepositRateHelper(
                depositRate, settlementDays, i+1, Months, calendar,
                ModifiedFollowing, depositDayCounter));
            instruments.push_back(depositHelper);
        }

        //Swap rates
        Rate swapRates[13] = {
            3.6425, 4.0875, 4.38, 4.5815, 4.74325, 4.87375,
            4.9775, 5.07, 5.13, 5.1825, 5.36, 5.45125, 5.43875};
        int swapYears[13] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30};

        int swFixedLegFrequency = 1;
        bool swFixedLegIsAdjusted = false;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
        int swFloatingLegFrequency = 2;

        for (i=0; i<13; i++) {
            Handle<MarketElement> swapRate(
                new SimpleMarketElement(swapRates[i]*0.01));
            Handle<RateHelper> swapHelper(new SwapRateHelper(
                RelinkableHandle<MarketElement>(swapRate), settlementDays,
                swapYears[i], calendar, ModifiedFollowing, swFixedLegFrequency,
                swFixedLegIsAdjusted, swFixedLegDayCounter,
                swFloatingLegFrequency));
            instruments.push_back(swapHelper);
        }


        // bootstrapping the yield curve
        Handle<PiecewiseFlatForward> myTermStructure(new
            PiecewiseFlatForward(currency, depositDayCounter,
            todaysDate, calendar, settlementDays, instruments));

        RelinkableHandle<TermStructure > rhTermStructure;
        rhTermStructure.linkTo(myTermStructure);

        //Define the ATM/OTM/ITM swaps
        int fixedLegFrequency = 1;
        bool fixedLegIsAdjusted = false;
        RollingConvention roll = ModifiedFollowing;
        DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);
        int floatingLegFrequency = 2;
        bool payFixedRate = true;
        int fixingDays = 0;
        Rate dummyFixedRate = 0.03;
        Handle<Xibor> indexSixMonths(new Euribor(6, Months, rhTermStructure));

        Handle<SimpleSwap> swap(new SimpleSwap(
            payFixedRate, todaysDate.plusYears(1), 5, Years,
            calendar, roll, 1000.0, fixedLegFrequency, dummyFixedRate,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            indexSixMonths, fixingDays, 0.0, rhTermStructure));
        Rate fixedATMRate = swap->fairRate();

        Handle<SimpleSwap> atmSwap(new SimpleSwap(
            payFixedRate, todaysDate.plusYears(1), 5, Years,
            calendar, roll, 1000.0, fixedLegFrequency, fixedATMRate,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            indexSixMonths, fixingDays, 0.0, rhTermStructure));
        Handle<SimpleSwap> otmSwap(new SimpleSwap(
            payFixedRate, Date(15, February, 2003), 5, Years,
            calendar, roll, 1000.0, fixedLegFrequency, fixedATMRate * 1.2,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            indexSixMonths, fixingDays, 0.0, rhTermStructure));
        Handle<SimpleSwap> itmSwap(new SimpleSwap(
            payFixedRate, Date(15, February, 2003), 5, Years,
            calendar, roll, 1000.0, fixedLegFrequency, fixedATMRate * 0.8,
            fixedLegIsAdjusted, fixedLegDayCounter, floatingLegFrequency,
            indexSixMonths, fixingDays, 0.0, rhTermStructure));

        std::vector<Period> swaptionMaturities;
        swaptionMaturities.push_back(Period(1, Months));
        swaptionMaturities.push_back(Period(3, Months));
        swaptionMaturities.push_back(Period(6, Months));
        swaptionMaturities.push_back(Period(1, Years));
        swaptionMaturities.push_back(Period(2, Years));
        swaptionMaturities.push_back(Period(3, Years));
        swaptionMaturities.push_back(Period(4, Years));
        swaptionMaturities.push_back(Period(5, Years));
        swaptionMaturities.push_back(Period(7, Years));
        swaptionMaturities.push_back(Period(10, Years));

        CalibrationSet swaptions;

        //List of times that have to be included in the timegrid
        std::list<Time> times;

        for (i=0; i<numRows; i++) {
            for (unsigned int j=0; j<numCols; j++) {
                unsigned int k = i*10 + j;
                Handle<MarketElement> vol(
                    new SimpleMarketElement(swaptionVols[k]*0.01));
                swaptions.push_back(Handle<CalibrationHelper>(
                    new SwaptionHelper(swaptionMaturities[j],
                                       Period(swaptionLengths[i], Years),
                                       RelinkableHandle<MarketElement>(vol),
                                       indexSixMonths,
                                       rhTermStructure)));
                swaptions.back()->addTimes(times);
            }
        }
        const std::vector<Time> termTimes = myTermStructure->times();
        for (i=0; i<termTimes.size(); i++)
            times.push_back(termTimes[i]);
        times.sort();
        times.unique();
        //Building time-grid
        TimeGrid grid(times, 30);

        Handle<Model> modelHW(new HullWhite(rhTermStructure));
        Handle<Model> modelHW2(new HullWhite(rhTermStructure));
        Handle<Model> modelBK(new BlackKarasinski(rhTermStructure));

        std::cout << "Calibrating to swaptions" << std::endl;

        std::cout << "Hull-White (analytic formulae):" << std::endl;
        swaptions.setPricingEngine(
            Handle<OptionPricingEngine>(new JamshidianSwaption(modelHW)));


        calibrateModel(modelHW, swaptions, 0.25);
        std::cout << "calibrated to "
                  << ArrayFormatter::toString(modelHW->params(),6)
                  << std::endl
                  << std::endl;

        std::cout << "Hull-White (numerical calibration):" << std::endl;
        swaptions.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW2, grid)));


        calibrateModel(modelHW2, swaptions, 0.25);
        std::cout << "calibrated to "
                  << ArrayFormatter::toString(modelHW2->params(),6)
                  << std::endl
                  << std::endl;

        std::cout << "Black-Karasinski: " << std::endl;
        swaptions.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelBK, grid)));
        calibrateModel(modelBK, swaptions, 0.25);
        std::cout << "calibrated to "
                  << ArrayFormatter::toString(modelBK->params())
                  << std::endl
                  << std::endl;

        std::cout << "Pricing an ATM bermudan swaption" << std::endl;

        //Define the bermudan swaption
        std::vector<Date> bermudanDates;
        const std::vector<Handle<CashFlow> >& leg = swap->floatingLeg();
        for (i=0; i<leg.size(); i++) {
            Handle<CashFlows::Coupon> coupon = leg[i];
            bermudanDates.push_back(coupon->accrualStartDate());
        }

        Instruments::Swaption bermudanSwaption(atmSwap,
            BermudanExercise(bermudanDates), rhTermStructure,
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW, 100)));

        //Do the pricing for each model
        bermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW, 100)));
        std::cout << "HW:       " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW2, 100)));
        std::cout << "HW (num): " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelBK, 100)));
        std::cout << "BK:       " << bermudanSwaption.NPV() << std::endl;

        std::cout << "Pricing an OTM bermudan swaption" << std::endl;

        Instruments::Swaption otmBermudanSwaption(otmSwap,
            BermudanExercise(bermudanDates), rhTermStructure,
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW, 100)));

        //Do the pricing for each model
        otmBermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW, 100)));
        std::cout << "HW:       " << otmBermudanSwaption.NPV() << std::endl;

        otmBermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW2, 100)));
        std::cout << "HW (num): " << otmBermudanSwaption.NPV() << std::endl;

        otmBermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelBK, 100)));
        std::cout << "BK:       " << otmBermudanSwaption.NPV() << std::endl;

        std::cout << "Pricing an ITM bermudan swaption" << std::endl;

        Instruments::Swaption itmBermudanSwaption(itmSwap,
            BermudanExercise(bermudanDates), rhTermStructure,
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW, 100)));

        //Do the pricing for each model
        itmBermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW, 100)));
        std::cout << "HW:       " << itmBermudanSwaption.NPV() << std::endl;
        itmBermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelHW2, 100)));
        std::cout << "HW (num): " << itmBermudanSwaption.NPV() << std::endl;
        itmBermudanSwaption.setPricingEngine(
            Handle<OptionPricingEngine>(new TreeSwaption(modelBK, 100)));
        std::cout << "BK:       " << itmBermudanSwaption.NPV() << std::endl;

        return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}

