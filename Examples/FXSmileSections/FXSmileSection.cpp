/*!
 Copyright (C) 2006 Allen Kuo

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

/*  This example shows how to set up a term structure and price a simple
    forward-rate agreement.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#    include <ql/auto_link.hpp>
#endif
#include <ql/termstructures/volatility/fxsmilesection.hpp>
#include <ql/termstructures/volatility/fxsmilesectionbystrike.hpp>
#include <ql/termstructures/volatility/fxsmilesectionbydelta.hpp>
#include <ql/termstructures/volatility/fxcostsmilesection.hpp>
#include <ql/termstructures/tradingtimetermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/fxvariancesurface.hpp>

#include <ql/time/date.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>

#include <iostream>

using namespace std;
using namespace QuantLib;

int main(int, char*[]) {

    try {

        std::cout << std::endl;

        /*********************
         ***     Cubic     ***
         *********************/
        /*cubic c(std::vector<Real>{8., 12., 6., 1.});
        std::vector<Real> z;
        Integer n = c.roots(z);

        std::cout << "number of roots: " << n << std::endl;
        
        for (int i = 0; i < z.size(); i++) 
        {
            std::cout << z[i] << ":\t" << c(z[i]) << std::endl;
        }*/

        /*********************
         ***    Quartic    ***
         *********************/
        /*quartic c(std::vector<Real>{10., 8., 6., 4., 2.});
        std::vector<Real> z;
        Integer n = c.roots(z);

        std::cout << "number of roots: " << n << std::endl;

        for (int i = 0; i < z.size(); i++) {
            std::cout << z[i] << ":\t" << c(z[i]) << std::endl;
        }*/

        Date refDate(14, February, 2025, 20, 30, 00);

        Handle<Quote> spot = makeQuoteHandle(1.7554);
        Handle<YieldTermStructure> forDiscount(ext::make_shared<FlatForward>(refDate, 0.02, Actual365Fixed()));
        Handle<YieldTermStructure> domDiscount(ext::make_shared<FlatForward>(refDate, 0.04, Actual365Fixed()));
        Handle<tradingTimeTermStructure> eventCurve(ext::make_shared<tradingTimeTermStructure>(refDate));
        Calendar cal = WeekendsOnly();

        std::vector<Date> dates;
        std::vector<Handle<Quote>> atmVols;
        std::vector<std::vector<Handle<Quote>>> rrs;
        std::vector<std::vector<Handle<Quote>>> bfs;
        std::vector<Real> deltas{0.25, 0.1};
        std::vector<Date> events;
        std::vector<Real> weights;

        // ON
        dates.push_back(Date(17, Feb, 2025));
        atmVols.push_back(makeQuoteHandle(0.1));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.009), makeQuoteHandle(-0.0162)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.0065), makeQuoteHandle(0.0234)});

        // 1W
        dates.push_back(Date(21, Feb, 2025));
        atmVols.push_back(makeQuoteHandle(0.085));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.0085), makeQuoteHandle(-0.0153)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.006), makeQuoteHandle(0.0216)});

        // 2W
        dates.push_back(Date(28, Feb, 2025));
        atmVols.push_back(makeQuoteHandle(0.075));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.0075), makeQuoteHandle(-0.0135)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.0055), makeQuoteHandle(0.0198)});

        // 1M
        dates.push_back(Date(14, Mar, 2025));
        atmVols.push_back(makeQuoteHandle(0.07));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.007), makeQuoteHandle(-0.0126)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.005), makeQuoteHandle(0.018)});

        // 2M
        dates.push_back(Date(14, Apr, 2025));
        atmVols.push_back(makeQuoteHandle(0.08));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.0078), makeQuoteHandle(-0.01404)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.0058), makeQuoteHandle(0.02088)});

        // 3M
        dates.push_back(Date(14, May, 2025));
        atmVols.push_back(makeQuoteHandle(0.087));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.0088), makeQuoteHandle(-0.01584)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.0064), makeQuoteHandle(0.02304)});

        // 6M
        dates.push_back(Date(14, Aug, 2025));
        atmVols.push_back(makeQuoteHandle(0.091));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.0088), makeQuoteHandle(-0.01584)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.0064), makeQuoteHandle(0.02304)});

        // 9M
        dates.push_back(Date(14, Nov, 2025));
        atmVols.push_back(makeQuoteHandle(0.105));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.011), makeQuoteHandle(-0.0198)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.0081), makeQuoteHandle(0.02916)});

        // 1Y
        dates.push_back(Date(16, Feb, 2026));
        atmVols.push_back(makeQuoteHandle(0.112));
        rrs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(-0.016), makeQuoteHandle(-0.0288)});
        bfs.push_back(std::vector<Handle<Quote>>{makeQuoteHandle(0.0095), makeQuoteHandle(0.0342)});

        Settings::instance().evaluationDate() = refDate;
        fxVarianceSurfaceNCP<fxCostSmileSectionFlatDynamics> vs(refDate, spot, dates, 
                                                                  atmVols, rrs, bfs, deltas, 
                                                                  forDiscount, domDiscount, eventCurve,
                                                                  DeltaVolQuote::PaSpot, DeltaVolQuote::AtmDeltaNeutral, 
                                                                  fxSmileSection::SmileStrangle);

        std::cout << vs.atmVol(Date(13, Nov, 2025)) << std::endl;

        Real f = vs.fwd(Date(14, Nov, 2025));
        std::cout << f << std::endl;
        std::cout << vs.blackVol(Date(13, Nov, 2025), f) << std::endl;

        Date d(28, Feb, 2025, 16, 30, 00);
        Time t = eventCurve->timeFromReference(d);
        std::cout << vs.timeFromReference(d) << std::endl;
        std::cout << t << std::endl;
        std::cout << eventCurve->tradingTime(d) << std::endl;
        std::cout << eventCurve->tradingTime(t) << std::endl;

        
        /*********************
         ***  MARKET DATA  ***
         *********************/
        /*
        Date todaysDate(2, January, 2024);
        Date expiryDate(2, January, 2025);
        Settings::instance().evaluationDate() = todaysDate;
        todaysDate = Settings::instance().evaluationDate();

        Handle<YieldTermStructure> forDiscount(ext::make_shared<FlatForward>(todaysDate, 0.05, Actual365Fixed()));
        Handle<YieldTermStructure> domDiscount(ext::make_shared<FlatForward>(todaysDate, 0.03, Actual365Fixed()));

        Handle<Quote> spot = makeQuoteHandle(1.7554);
        Handle<Quote> v_atm = makeQuoteHandle(0.14483);
        Handle<Quote> v_25rr = makeQuoteHandle(0.05770);
        Handle<Quote> v_10rr = makeQuoteHandle(0.101575);
        Handle<Quote> v_25bf = makeQuoteHandle(0.007425);
        Handle<Quote> v_10bf = makeQuoteHandle(0.016125);
        std::vector<Real> dlts{0.25, 0.1};
        DeltaVolQuote::DeltaType delta_type = DeltaVolQuote::PaSpot;
        DeltaVolQuote::AtmType atm_type = DeltaVolQuote::AtmFwd;

        fxCostSmileSectionFlatDynamics sf(expiryDate, spot, v_atm, std::vector<Handle<Quote>>{v_25rr, v_10rr},
                                          std::vector<Handle<Quote>>{v_25bf, v_10bf}, dlts, forDiscount, domDiscount,
                                          DeltaVolQuote::PaSpot, DeltaVolQuote::AtmFwd, fxSmileSection::SmileStrangle, 
                                          Actual365Fixed(), Date(), true);
        std::cout << "Atm:" << sf.atmLevel() << std::endl;
        std::cout << "IVol:" << sf.volByStrike(1.745) << std::endl;
        std::cout << "IVol:" << sf.volByStrike(1.755) << std::endl;
        std::cout << "IVol:" << sf.volByStrike(1.765) << std::endl << std::endl;

        std::cout << "IVol:" << sf.volByDelta(-0.10, Option::Put) << std::endl;
        std::cout << "IVol:" << sf.volByDelta(-0.25, Option::Put) << std::endl;
        std::cout << "IVol:" << sf.atm()->value() << std::endl;
        std::cout << "IVol:" << sf.volByDelta(0.25, Option::Call) << std::endl;
        std::cout << "IVol:" << sf.volByDelta(0.10, Option::Call) << std::endl;
        std::cout << std::endl << std::endl;

        std::cout << "ncp:" << sf.normedCallPrice(spot->value())<< std::endl;
        Real np = sf.normedProbability(spot->value(), 1e-10);
        std::cout << "ndp:" << sf.normedProbability(spot->value(), 10e-10) << std::endl;
        std::cout << "ndp:" << sf.normedProbability(spot->value(), 10e-10) << std::endl;
        std::cout << "ndp:" << sf.normedProbability(spot->value(), 10e-8) << std::endl;
        std::cout << "ndp:" << sf.normedProbability(spot->value(), 10e-6) << std::endl;
        std::cout << "ndp:" << sf.normedProbability(spot->value(), 10e-4) << std::endl;
        std::cout << "nsk:" << sf.strikeFromNormProb(np) << std::endl;
        std::cout << std::endl << std::endl;

        fxCostSmileSectionScaledDynamics ss(expiryDate, spot, v_atm, std::vector<Handle<Quote>>{v_25rr, v_10rr},
                                            std::vector<Handle<Quote>>{v_25bf, v_10bf}, dlts, forDiscount, domDiscount,
                                            DeltaVolQuote::PaSpot, DeltaVolQuote::AtmFwd, fxSmileSection::SmileStrangle,
                                            Actual365Fixed(), Date(), true);
        std::cout << "IVol:" << ss.volByStrike(1.745) << std::endl;
        std::cout << "IVol:" << ss.volByStrike(1.755) << std::endl;
        std::cout << "IVol:" << ss.volByStrike(1.765) << std::endl << std::endl;

        std::cout << "IVol:" << ss.volByDelta(-0.10, Option::Put) << std::endl;
        std::cout << "IVol:" << ss.volByDelta(-0.25, Option::Put) << std::endl;
        std::cout << "IVol:" << ss.atm()->value() << std::endl;
        std::cout << "IVol:" << ss.volByDelta(0.25, Option::Call) << std::endl;
        std::cout << "IVol:" << ss.volByDelta(0.10, Option::Call) << std::endl;
        */

        return 0;

    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}
