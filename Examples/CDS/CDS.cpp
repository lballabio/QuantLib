/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio

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

#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }

}
#endif

int main(int, char* []) {

    try {

        boost::timer timer;
        std::cout << std::endl;

        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();
        Date todaysDate(15, May, 2007);
        // must be a business day
        todaysDate = calendar.adjust(todaysDate);

        Settings::instance().evaluationDate() = todaysDate;

        // dummy curve
        boost::shared_ptr<Quote> flatRate(new SimpleQuote(0.01));
        Handle<YieldTermStructure> tsCurve(
              boost::shared_ptr<FlatForward>(
                      new FlatForward(todaysDate, Handle<Quote>(flatRate),
                                      Actual365Fixed())));

        /*
          In Lehmans Brothers "guide to exotic credit derivatives"
          p. 32 there's a simple case, zero flat curve with a flat CDS
          curve with constant market spreads of 150 bp and RR = 50%
          corresponds to a flat 3% hazard rate. The implied 1-year
          survival probability is 97.04% and the 2-years is 94.18%
        */

        // market
        Real recovery_rate = 0.5;
        Real quoted_spreads[] = { 0.0150, 0.0150, 0.0150, 0.0150 };
        vector<Period> tenors;
        tenors.push_back(3*Months);
        tenors.push_back(6*Months);
        tenors.push_back(1*Years);
        tenors.push_back(2*Years);
        vector<Date> maturities;
        for (Size i=0; i<4; i++) {
            maturities.push_back(calendar.adjust(todaysDate + tenors[i],
                                                 Following));
        }

        std::vector<boost::shared_ptr<DefaultProbabilityHelper> > instruments;
        for (Size i=0; i<4; i++) {
            instruments.push_back(boost::shared_ptr<DefaultProbabilityHelper>(
                new SpreadCdsHelper(
                              Handle<Quote>(boost::shared_ptr<Quote>(
                                         new SimpleQuote(quoted_spreads[i]))),
                              tenors[i],
                              0,
                              calendar,
                              Quarterly,
                              Following,
                              DateGeneration::TwentiethIMM,
                              Actual365Fixed(),
                              recovery_rate,
                              tsCurve)));
        }

        // Bootstrap hazard rates
        boost::shared_ptr<PiecewiseDefaultCurve<HazardRate, BackwardFlat> >
           hazardRateStructure(
               new PiecewiseDefaultCurve<HazardRate, BackwardFlat>(
                                                           todaysDate,
                                                           instruments,
                                                           Actual365Fixed()));
        vector<pair<Date, Real> > hr_curve_data = hazardRateStructure->nodes();

        cout << "Calibrated hazard rate values: " << endl ;
        for (Size i=0; i<hr_curve_data.size(); i++) {
            cout << "hazard rate on " << hr_curve_data[i].first << " is "
                 << hr_curve_data[i].second << endl;
        }
        cout << endl;

        cout << "Some survival probability values: " << endl ;
        cout << "1Y survival probability: "
             << io::percent(hazardRateStructure->survivalProbability(
                                                        todaysDate + 1*Years))
             << endl
             << "               expected: "
             << io::percent(0.9704)
             << endl;
        cout << "2Y survival probability: "
             << io::percent(hazardRateStructure->survivalProbability(
                                                        todaysDate + 2*Years))
             << endl
             << "               expected: "
             << io::percent(0.9418)
             << endl;

        cout << endl << endl;

        // reprice instruments
        Real nominal = 1000000.0;
        Handle<DefaultProbabilityTermStructure> probability(hazardRateStructure);
        boost::shared_ptr<PricingEngine> engine(
                  new MidPointCdsEngine(probability, recovery_rate, tsCurve));

        Schedule cdsSchedule =
            MakeSchedule().from(todaysDate).to(maturities[0])
                          .withFrequency(Quarterly)
                          .withCalendar(calendar)
                          .withTerminationDateConvention(Unadjusted)
                          .withRule(DateGeneration::TwentiethIMM);
        CreditDefaultSwap cds_3m(Protection::Seller,
                                 nominal,
                                 quoted_spreads[0],
                                 cdsSchedule,
                                 Following,
                                 Actual365Fixed());

        cdsSchedule =
            MakeSchedule().from(todaysDate).to(maturities[1])
                          .withFrequency(Quarterly)
                          .withCalendar(calendar)
                          .withTerminationDateConvention(Unadjusted)
                          .withRule(DateGeneration::TwentiethIMM);
        CreditDefaultSwap cds_6m(Protection::Seller,
                                 nominal,
                                 quoted_spreads[1],
                                 cdsSchedule,
                                 Following,
                                 Actual365Fixed());

        cdsSchedule =
            MakeSchedule().from(todaysDate).to(maturities[2])
                          .withFrequency(Quarterly)
                          .withCalendar(calendar)
                          .withTerminationDateConvention(Unadjusted)
                          .withRule(DateGeneration::TwentiethIMM);
        CreditDefaultSwap cds_1y(Protection::Seller,
                                 nominal,
                                 quoted_spreads[2],
                                 cdsSchedule,
                                 Following,
                                 Actual365Fixed());

        cdsSchedule =
            MakeSchedule().from(todaysDate).to(maturities[3])
                          .withFrequency(Quarterly)
                          .withCalendar(calendar)
                          .withTerminationDateConvention(Unadjusted)
                          .withRule(DateGeneration::TwentiethIMM);
        CreditDefaultSwap cds_2y(Protection::Seller,
                                 nominal,
                                 quoted_spreads[3],
                                 cdsSchedule,
                                 Following,
                                 Actual365Fixed());

        cds_3m.setPricingEngine(engine);
        cds_6m.setPricingEngine(engine);
        cds_1y.setPricingEngine(engine);
        cds_2y.setPricingEngine(engine);

        cout << "Repricing of quoted CDSs employed for calibration: " << endl;
        cout << "3M fair spread: " << io::rate(cds_3m.fairSpread()) << endl
             << "   NPV:         " << cds_3m.NPV() << endl
             << "   default leg: " << cds_3m.defaultLegNPV() << endl
             << "   coupon leg:  " << cds_3m.couponLegNPV() << endl
             << endl;

        cout << "6M fair spread: " << io::rate(cds_6m.fairSpread()) << endl
             << "   NPV:         " << cds_6m.NPV() << endl
             << "   default leg: " << cds_6m.defaultLegNPV() << endl
             << "   coupon leg:  " << cds_6m.couponLegNPV() << endl
             << endl;

        cout << "1Y fair spread: " << io::rate(cds_1y.fairSpread()) << endl
             << "   NPV:         " << cds_1y.NPV() << endl
             << "   default leg: " << cds_1y.defaultLegNPV() << endl
             << "   coupon leg:  " << cds_1y.couponLegNPV() << endl
             << endl;

        cout << "2Y fair spread: " << io::rate(cds_2y.fairSpread()) << endl
             << "   NPV:         " << cds_2y.NPV() << endl
             << "   default leg: " << cds_2y.defaultLegNPV() << endl
             << "   coupon leg:  " << cds_2y.couponLegNPV() << endl
             << endl;

        cout << endl << endl;

        Real seconds  = timer.elapsed();
        Integer hours = Integer(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = Integer(seconds/60);
        seconds -= minutes * 60;
        cout << "Run completed in ";
        if (hours > 0)
            cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            cout << minutes << " m ";
        cout << fixed << setprecision(0)
             << seconds << " s" << endl;

        return 0;
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}

