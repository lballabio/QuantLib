
/*!
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/quantlib.hpp>

using namespace QuantLib;

//Number of swaptions to be calibrated to...

Size numRows = 5;
Size numCols = 5;

Integer swapLenghts[] = {
      1,     2,     3,     4,     5};
Volatility swaptionVols[] = {
  14.90, 13.40, 12.28, 11.89, 11.48,
  12.90, 12.01, 11.46, 11.08, 10.40,
  11.49, 11.12, 10.70, 10.10,  9.57,
  10.47, 10.21,  9.80,  9.51, 12.70,
  10.00,  9.50,  9.00, 12.30, 11.60};

void calibrateModel(const boost::shared_ptr<ShortRateModel>& model,
                    const std::vector<boost::shared_ptr<CalibrationHelper> >&
                                                                      helpers,
                    Real lambda) {

    Simplex om(lambda, 1e-9);
    om.setEndCriteria(EndCriteria(10000, 1e-7));
    model->calibrate(helpers, om);

    #if defined(QL_PATCH_DARWIN)
    // to be investigated
    return;
    #endif

    // Output the implied Black volatilities
    for (Size i=0; i<numRows; i++) {
        Size j = numCols - i -1; // 1x5, 2x4, 3x3, 4x2, 5x1
        Size k = i*numCols + j;
        Real npv = helpers[i]->modelValue();
        Volatility implied = helpers[i]->impliedVolatility(npv, 1e-4,
            1000, 0.05, 0.50)*100.0;
        Volatility diff = implied - swaptionVols[k];

        std::cout << i+1 << "x" << swapLenghts[j]
            << ": model " << DecimalFormatter::toString(implied,2,5)
            << ", market " << DecimalFormatter::toString(swaptionVols[k],2,5)
            << " (" << DecimalFormatter::toString(diff,2,5) << ")\n";
    }
}

int main(int, char* [])
{
    try {
        QL_IO_INIT

        Date todaysDate(15, February, 2002);
        Calendar calendar = TARGET();
        Date settlementDate(19, February, 2002);
        Settings::instance().setEvaluationDate(todaysDate);

        // flat yield term structure impling 1x5 swap at 5%
        boost::shared_ptr<Quote> flatRate(new SimpleQuote(0.04875825));
        boost::shared_ptr<FlatForward> myTermStructure(
                      new FlatForward(settlementDate, Handle<Quote>(flatRate),
                                      Actual365Fixed()));
        Handle<YieldTermStructure> rhTermStructure;
        rhTermStructure.linkTo(myTermStructure);

        // Define the ATM/OTM/ITM swaps
        Frequency fixedLegFrequency = Annual;
        BusinessDayConvention fixedLegConvention = Unadjusted;
        BusinessDayConvention floatingLegConvention = ModifiedFollowing;
        DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);
        Frequency floatingLegFrequency = Semiannual;
        bool payFixedRate = true;
        Integer fixingDays = 2;
        Rate dummyFixedRate = 0.03;
        boost::shared_ptr<Xibor> indexSixMonths(new
            Euribor(6, Months, rhTermStructure));

        Date startDate = calendar.advance(settlementDate,1,Years,
                                          floatingLegConvention);
        Date maturity = calendar.advance(startDate,5,Years,
                                         floatingLegConvention);
        Schedule fixedSchedule(calendar,startDate,maturity,
                               fixedLegFrequency,fixedLegConvention);
        Schedule floatSchedule(calendar,startDate,maturity,
                               floatingLegFrequency,floatingLegConvention);
        boost::shared_ptr<SimpleSwap> swap(new SimpleSwap(
            payFixedRate, 1000.0,
            fixedSchedule, dummyFixedRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, fixingDays, 0.0,
            rhTermStructure));
        Rate fixedATMRate = swap->fairRate();
        Rate fixedOTMRate = fixedATMRate * 1.2;
        Rate fixedITMRate = fixedATMRate * 0.8;

        boost::shared_ptr<SimpleSwap> atmSwap(new SimpleSwap(
            payFixedRate, 1000.0,
            fixedSchedule, fixedATMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, fixingDays, 0.0,
            rhTermStructure));
        boost::shared_ptr<SimpleSwap> otmSwap(new SimpleSwap(
            payFixedRate, 1000.0,
            fixedSchedule, fixedOTMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, fixingDays, 0.0,
            rhTermStructure));
        boost::shared_ptr<SimpleSwap> itmSwap(new SimpleSwap(
            payFixedRate, 1000.0,
            fixedSchedule, fixedITMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, fixingDays, 0.0,
            rhTermStructure));

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
            boost::shared_ptr<Quote> vol(new
                SimpleQuote(swaptionVols[k]*0.01));
            swaptions.push_back(boost::shared_ptr<CalibrationHelper>(new
                SwaptionHelper(swaptionMaturities[i],
                               Period(swapLenghts[j], Years),
                               Handle<Quote>(vol),
                               indexSixMonths,
                               rhTermStructure)));
            swaptions.back()->addTimesTo(times);
        }

        // Building time-grid
        TimeGrid grid(times.begin(), times.end(), 30);


        // defining the models
        boost::shared_ptr<G2> modelG2(new G2(rhTermStructure));
        boost::shared_ptr<HullWhite> modelHW(new HullWhite(rhTermStructure));

#define ALSO_NUMERICAL_MODELS

#ifdef ALSO_NUMERICAL_MODELS
        boost::shared_ptr<HullWhite> modelHW2(new HullWhite(rhTermStructure));
        boost::shared_ptr<BlackKarasinski> modelBK(new
            BlackKarasinski(rhTermStructure));
#endif


        // model calibrations

        std::cout << "G2 (analytic formulae) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new G2SwaptionEngine(modelG2, 6.0, 16)));

        calibrateModel(modelG2, swaptions, 0.05);
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

        calibrateModel(modelHW, swaptions, 0.05);
        std::cout << "calibrated to:\n"
                  << "a = " << modelHW->params()[0] << ", "
                  << "sigma = " << modelHW->params()[1]
                  << std::endl << std::endl;

#ifdef ALSO_NUMERICAL_MODELS
        std::cout << "Hull-White (numerical) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new TreeSwaptionEngine(modelHW2,grid)));

        calibrateModel(modelHW2, swaptions, 0.05);
        std::cout << "calibrated to:\n"
                  << "a = " << modelHW2->params()[0] << ", "
                  << "sigma = " << modelHW2->params()[1]
                  << std::endl << std::endl;


        std::cout << "Black-Karasinski (numerical) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new TreeSwaptionEngine(modelBK,grid)));

        calibrateModel(modelBK, swaptions, 0.05);
        std::cout << "calibrated to:\n"
                  << "a = " << modelBK->params()[0] << ", "
                  << "sigma = " << modelBK->params()[1]
                  << std::endl << std::endl;
#endif


        // ATM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
            << "struck at " << RateFormatter::toString(fixedATMRate)
            << " (ATM)" << std::endl;

        std::vector<Date> bermudanDates;
        const std::vector<boost::shared_ptr<CashFlow> >& leg =
            swap->fixedLeg();
        for (i=0; i<leg.size(); i++) {
            boost::shared_ptr<Coupon> coupon =
                boost::dynamic_pointer_cast<Coupon>(leg[i]);
            bermudanDates.push_back(coupon->accrualStartDate());
        }

        boost::shared_ptr<Exercise> bermudaExercise(new
            BermudanExercise(bermudanDates));

        Swaption bermudanSwaption(atmSwap, bermudaExercise, rhTermStructure,
                                  boost::shared_ptr<PricingEngine>());

        // Do the pricing for each model

        // G2 price the European swaption here, it should switch to bermudan
        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(new
            TreeSwaptionEngine(modelG2, 50)));
        std::cout << "G2:       " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
           new TreeSwaptionEngine(modelHW, 50)));
        std::cout << "HW:       " << bermudanSwaption.NPV() << std::endl;

#ifdef ALSO_NUMERICAL_MODELS
        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(new
            TreeSwaptionEngine(modelHW2, 50)));
        std::cout << "HW (num): " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(new
            TreeSwaptionEngine(modelBK, 50)));
        std::cout << "BK:       " << bermudanSwaption.NPV() << std::endl;
#endif


        // OTM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
            << "struck at " << RateFormatter::toString(fixedOTMRate)
            << " (OTM)" << std::endl;

        Swaption otmBermudanSwaption(otmSwap, bermudaExercise, rhTermStructure,
                                     boost::shared_ptr<PricingEngine>());

        // Do the pricing for each model
        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelG2, 50)));
        std::cout << "G2:       " << otmBermudanSwaption.NPV() << std::endl;

        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW, 50)));
        std::cout << "HW:       " << otmBermudanSwaption.NPV() << std::endl;

#ifdef ALSO_NUMERICAL_MODELS
        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW2, 50)));
        std::cout << "HW (num): " << otmBermudanSwaption.NPV() << std::endl;

        otmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelBK, 50)));
        std::cout << "BK:       " << otmBermudanSwaption.NPV() << std::endl;
#endif


        // ITM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
            << "struck at " << RateFormatter::toString(fixedITMRate)
            << " (ITM)" << std::endl;

        Swaption itmBermudanSwaption(itmSwap, bermudaExercise, rhTermStructure,
                                     boost::shared_ptr<PricingEngine>());

        // Do the pricing for each model
        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelG2, 50)));
        std::cout << "G2:       " << itmBermudanSwaption.NPV() << std::endl;

        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW, 50)));
        std::cout << "HW:       " << itmBermudanSwaption.NPV() << std::endl;

#ifdef ALSO_NUMERICAL_MODELS
        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelHW2, 50)));
        std::cout << "HW (num): " << itmBermudanSwaption.NPV() << std::endl;

        itmBermudanSwaption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new TreeSwaptionEngine(modelBK, 50)));
        std::cout << "BK:       " << itmBermudanSwaption.NPV() << std::endl;
#endif

        return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}

