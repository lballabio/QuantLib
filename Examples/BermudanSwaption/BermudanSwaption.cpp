/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/quantlib.hpp>

#ifdef BOOST_MSVC
/* Uncomment the following lines to unmask floating-point
   exceptions. Warning: unpredictable results can arise...

   See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
   Is there anyone with a definitive word about this?
*/
// #include <float.h>
// namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }

}
#endif


//Number of swaptions to be calibrated to...

Size numRows = 5;
Size numCols = 5;

Integer swapLenghts[] = {
      1,     2,     3,     4,     5};
Volatility swaptionVols[] = {
  0.1490, 0.1340, 0.1228, 0.1189, 0.1148,
  0.1290, 0.1201, 0.1146, 0.1108, 0.1040,
  0.1149, 0.1112, 0.1070, 0.1010, 0.0957,
  0.1047, 0.1021, 0.0980, 0.0951, 0.1270,
  0.1000, 0.0950, 0.0900, 0.1230, 0.1160};

void calibrateModel(
          const boost::shared_ptr<ShortRateModel>& model,
          const std::vector<boost::shared_ptr<CalibrationHelper> >& helpers) {

    LevenbergMarquardt om;
    model->calibrate(helpers, om,
                     EndCriteria(400, 100, 1.0e-8, 1.0e-8, 1.0e-8));

    // Output the implied Black volatilities
    for (Size i=0; i<numRows; i++) {
        Size j = numCols - i -1; // 1x5, 2x4, 3x3, 4x2, 5x1
        Size k = i*numCols + j;
        Real npv = helpers[i]->modelValue();
        Volatility implied = helpers[i]->impliedVolatility(npv, 1e-4,
                                                           1000, 0.05, 0.50);
        Volatility diff = implied - swaptionVols[k];

        std::cout << i+1 << "x" << swapLenghts[j]
                  << std::setprecision(5) << std::noshowpos
                  << ": model " << std::setw(7) << io::volatility(implied)
                  << ", market " << std::setw(7)
                  << io::volatility(swaptionVols[k])
                  << " (" << std::setw(7) << std::showpos
                  << io::volatility(diff) << std::noshowpos << ")\n";
    }
}

int main(int, char* []) {

    try {

        boost::timer timer;
        std::cout << std::endl;

        Date todaysDate(15, February, 2002);
        Calendar calendar = TARGET();
        Date settlementDate(19, February, 2002);
        Settings::instance().evaluationDate() = todaysDate;

        // flat yield term structure impling 1x5 swap at 5%
        boost::shared_ptr<Quote> flatRate(new SimpleQuote(0.04875825));
        Handle<YieldTermStructure> rhTermStructure(
            boost::shared_ptr<FlatForward>(
                      new FlatForward(settlementDate, Handle<Quote>(flatRate),
                                      Actual365Fixed())));

        // Define the ATM/OTM/ITM swaps
        Frequency fixedLegFrequency = Annual;
        BusinessDayConvention fixedLegConvention = Unadjusted;
        BusinessDayConvention floatingLegConvention = ModifiedFollowing;
        DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);
        Frequency floatingLegFrequency = Semiannual;
        VanillaSwap::Type type = VanillaSwap::Payer;
        Rate dummyFixedRate = 0.03;
        boost::shared_ptr<IborIndex> indexSixMonths(new
            Euribor6M(rhTermStructure));

        Date startDate = calendar.advance(settlementDate,1,Years,
                                          floatingLegConvention);
        Date maturity = calendar.advance(startDate,5,Years,
                                         floatingLegConvention);
        Schedule fixedSchedule(startDate,maturity,Period(fixedLegFrequency),
                               calendar,fixedLegConvention,fixedLegConvention,
                               DateGeneration::Forward,false);
        Schedule floatSchedule(startDate,maturity,Period(floatingLegFrequency),
                               calendar,floatingLegConvention,floatingLegConvention,
                               DateGeneration::Forward,false);

        boost::shared_ptr<VanillaSwap> swap(new VanillaSwap(
            type, 1000.0,
            fixedSchedule, dummyFixedRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter()));
        swap->setPricingEngine(boost::shared_ptr<PricingEngine>(
                                 new DiscountingSwapEngine(rhTermStructure)));
        Rate fixedATMRate = swap->fairRate();
        Rate fixedOTMRate = fixedATMRate * 1.2;
        Rate fixedITMRate = fixedATMRate * 0.8;

        boost::shared_ptr<VanillaSwap> atmSwap(new VanillaSwap(
            type, 1000.0,
            fixedSchedule, fixedATMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter()));
        boost::shared_ptr<VanillaSwap> otmSwap(new VanillaSwap(
            type, 1000.0,
            fixedSchedule, fixedOTMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter()));
        boost::shared_ptr<VanillaSwap> itmSwap(new VanillaSwap(
            type, 1000.0,
            fixedSchedule, fixedITMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter()));

        // defining the swaptions to be used in model calibration
        std::vector<Period> swaptionMaturities;
        swaptionMaturities.push_back(Period(1, Years));
        swaptionMaturities.push_back(Period(2, Years));
        swaptionMaturities.push_back(Period(3, Years));
        swaptionMaturities.push_back(Period(4, Years));
        swaptionMaturities.push_back(Period(5, Years));

        std::vector<boost::shared_ptr<CalibrationHelper> > swaptions;

        // List of times that have to be included in the timegrid
        std::list<Time> times;

        Size i;
        for (i=0; i<numRows; i++) {
            Size j = numCols - i -1; // 1x5, 2x4, 3x3, 4x2, 5x1
            Size k = i*numCols + j;
            boost::shared_ptr<Quote> vol(new SimpleQuote(swaptionVols[k]));
            swaptions.push_back(boost::shared_ptr<CalibrationHelper>(new
                SwaptionHelper(swaptionMaturities[i],
                               Period(swapLenghts[j], Years),
                               Handle<Quote>(vol),
                               indexSixMonths,
                               indexSixMonths->tenor(),
                               indexSixMonths->dayCounter(),
                               indexSixMonths->dayCounter(),
                               rhTermStructure)));
            swaptions.back()->addTimesTo(times);
        }

        // Building time-grid
        TimeGrid grid(times.begin(), times.end(), 30);


        // defining the models
        boost::shared_ptr<G2> modelG2(new G2(rhTermStructure));
        boost::shared_ptr<HullWhite> modelHW(new HullWhite(rhTermStructure));
        boost::shared_ptr<HullWhite> modelHW2(new HullWhite(rhTermStructure));
        boost::shared_ptr<BlackKarasinski> modelBK(
                                        new BlackKarasinski(rhTermStructure));


        // model calibrations

        std::cout << "G2 (analytic formulae) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new G2SwaptionEngine(modelG2, 6.0, 16)));

        calibrateModel(modelG2, swaptions);
        std::cout << "calibrated to:\n"
                  << "a     = " << modelG2->params()[0] << ", "
                  << "sigma = " << modelG2->params()[1] << "\n"
                  << "b     = " << modelG2->params()[2] << ", "
                  << "eta   = " << modelG2->params()[3] << "\n"
                  << "rho   = " << modelG2->params()[4]
                  << std::endl << std::endl;



        std::cout << "Hull-White (analytic formulae) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new JamshidianSwaptionEngine(modelHW)));

        calibrateModel(modelHW, swaptions);
        std::cout << "calibrated to:\n"
                  << "a = " << modelHW->params()[0] << ", "
                  << "sigma = " << modelHW->params()[1]
                  << std::endl << std::endl;

        std::cout << "Hull-White (numerical) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(boost::shared_ptr<PricingEngine>(
                                     new TreeSwaptionEngine(modelHW2, grid)));

        calibrateModel(modelHW2, swaptions);
        std::cout << "calibrated to:\n"
                  << "a = " << modelHW2->params()[0] << ", "
                  << "sigma = " << modelHW2->params()[1]
                  << std::endl << std::endl;

        std::cout << "Black-Karasinski (numerical) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(boost::shared_ptr<PricingEngine>(
                                      new TreeSwaptionEngine(modelBK, grid)));

        calibrateModel(modelBK, swaptions);
        std::cout << "calibrated to:\n"
                  << "a = " << modelBK->params()[0] << ", "
                  << "sigma = " << modelBK->params()[1]
                  << std::endl << std::endl;


        // ATM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
                  << "struck at " << io::rate(fixedATMRate)
                  << " (ATM)" << std::endl;

        std::vector<Date> bermudanDates;
        const std::vector<boost::shared_ptr<CashFlow> >& leg =
            swap->fixedLeg();
        for (i=0; i<leg.size(); i++) {
            boost::shared_ptr<Coupon> coupon =
                boost::dynamic_pointer_cast<Coupon>(leg[i]);
            bermudanDates.push_back(coupon->accrualStartDate());
        }

        boost::shared_ptr<Exercise> bermudanExercise(
                                         new BermudanExercise(bermudanDates));

        Swaption bermudanSwaption(atmSwap, bermudanExercise);

        // Do the pricing for each model

        // G2 price the European swaption here, it should switch to bermudan
        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelG2, 50)));
        std::cout << "G2 (tree):      " << bermudanSwaption.NPV() << std::endl;
        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdG2SwaptionEngine(modelG2)));
        std::cout << "G2 (fdm) :      " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW, 50)));
        std::cout << "HW (tree):      " << bermudanSwaption.NPV() << std::endl;
        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHullWhiteSwaptionEngine(modelHW)));
        std::cout << "HW (fdm) :      " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW2, 50)));
        std::cout << "HW (num, tree): " << bermudanSwaption.NPV() << std::endl;
        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHullWhiteSwaptionEngine(modelHW2)));
        std::cout << "HW (num, fdm) : " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelBK, 50)));
        std::cout << "BK:             " << bermudanSwaption.NPV() << std::endl;


        // OTM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
                  << "struck at " << io::rate(fixedOTMRate)
                  << " (OTM)" << std::endl;

        Swaption otmBermudanSwaption(otmSwap,bermudanExercise);

        // Do the pricing for each model
        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelG2, 300)));
        std::cout << "G2 (tree):       " << otmBermudanSwaption.NPV()
                  << std::endl;
        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdG2SwaptionEngine(modelG2)));
        std::cout << "G2 (fdm) :       " << otmBermudanSwaption.NPV()
                  << std::endl;

        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW, 50)));
        std::cout << "HW (tree):       " << otmBermudanSwaption.NPV()
                  << std::endl;
        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHullWhiteSwaptionEngine(modelHW)));
        std::cout << "HW (fdm) :       " << otmBermudanSwaption.NPV()
                  << std::endl;

        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW2, 50)));
        std::cout << "HW (num, tree):  " << otmBermudanSwaption.NPV()
                  << std::endl;
        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHullWhiteSwaptionEngine(modelHW2)));
        std::cout << "HW (num, fdm):   " << otmBermudanSwaption.NPV()
                  << std::endl;

        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelBK, 50)));
        std::cout << "BK:              " << otmBermudanSwaption.NPV()
                  << std::endl;


        // ITM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
                  << "struck at " << io::rate(fixedITMRate)
                  << " (ITM)" << std::endl;

        Swaption itmBermudanSwaption(itmSwap,bermudanExercise);

        // Do the pricing for each model
        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelG2, 50)));
        std::cout << "G2 (tree):       " << itmBermudanSwaption.NPV()
                  << std::endl;
        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdG2SwaptionEngine(modelG2)));
        std::cout << "G2 (fdm) :       " << itmBermudanSwaption.NPV()
                  << std::endl;

        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW, 50)));
        std::cout << "HW (tree):       " << itmBermudanSwaption.NPV()
                  << std::endl;
        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHullWhiteSwaptionEngine(modelHW)));
        std::cout << "HW (fdm) :       " << itmBermudanSwaption.NPV()
                  << std::endl;

        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW2, 50)));
        std::cout << "HW (num, tree):  " << itmBermudanSwaption.NPV()
                  << std::endl;
        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FdHullWhiteSwaptionEngine(modelHW2)));
        std::cout << "HW (num, fdm) :  " << itmBermudanSwaption.NPV()
                  << std::endl;

        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelBK, 50)));
        std::cout << "BK:              " << itmBermudanSwaption.NPV()
                  << std::endl;

        Real seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        std::cout << " \nRun completed in ";
        if (hours > 0)
            std::cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            std::cout << minutes << " m ";
        std::cout << std::fixed << std::setprecision(0)
                  << seconds << " s\n" << std::endl;

        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

