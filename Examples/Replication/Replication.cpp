/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*  This example showcases the CompositeInstrument class. Such class
    is used to build a static replication of a down-and-out barrier
    option, as outlined in Section 10.2 of Mark Joshi's "The Concepts
    and Practice of Mathematical Finance" to which we refer the
    reader.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/compositeinstrument.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/exercise.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;

int main(int, char* []) {

    try {

        std::cout << std::endl;

        Date today(29, May, 2006);
        Settings::instance().evaluationDate() = today;

        // the option to replicate
        Barrier::Type barrierType = Barrier::DownOut;
        Real barrier = 70.0;
        Real rebate = 0.0;
        Option::Type type = Option::Put;
        Real underlyingValue = 100.0;
        auto underlying = ext::make_shared<SimpleQuote>(underlyingValue);
        Real strike = 100.0;
        auto riskFreeRate = ext::make_shared<SimpleQuote>(0.04);
        auto volatility = ext::make_shared<SimpleQuote>(0.20);
        Date maturity = today + 1*Years;

        std::cout << std::endl ;

        // write column headings
        Size widths[] = { 45, 15, 15 };
        Size totalWidth = widths[0]+widths[1]+widths[2];
        std::string rule(totalWidth, '-'), dblrule(totalWidth, '=');

        std::cout << dblrule << std::endl;
        std::cout << "Initial market conditions" << std::endl;
        std::cout << dblrule << std::endl;
        std::cout << std::setw(widths[0]) << std::left << "Option"
                  << std::setw(widths[1]) << std::left << "NPV"
                  << std::setw(widths[2]) << std::left << "Error"
                  << std::endl;
        std::cout << rule << std::endl;

        // bootstrap the yield/vol curves
        DayCounter dayCounter = Actual365Fixed();
        Handle<Quote> h1(riskFreeRate);
        Handle<Quote> h2(volatility);
        Handle<YieldTermStructure> flatRate(
            ext::make_shared<FlatForward>(0, NullCalendar(), h1, dayCounter));
        Handle<BlackVolTermStructure> flatVol(
            ext::make_shared<BlackConstantVol>(0, NullCalendar(), h2, dayCounter));

        // instantiate the option
        auto exercise = ext::make_shared<EuropeanExercise>(maturity);
        auto payoff = ext::make_shared<PlainVanillaPayoff>(type, strike);

        auto bsProcess = ext::make_shared<BlackScholesProcess>(
            Handle<Quote>(underlying), flatRate, flatVol);

        auto barrierEngine = ext::make_shared<AnalyticBarrierEngine>(bsProcess);
        auto europeanEngine = ext::make_shared<AnalyticEuropeanEngine>(bsProcess);

        BarrierOption referenceOption(barrierType, barrier, rebate,
                                      payoff, exercise);
        referenceOption.setPricingEngine(barrierEngine);

        Real referenceValue = referenceOption.NPV();

        std::cout << std::setw(widths[0]) << std::left
                  << "Original barrier option"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << referenceValue
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::endl;

        // Replicating portfolios
        CompositeInstrument portfolio1, portfolio2, portfolio3;

        // Final payoff first (the same for all portfolios):
        // as shown in Joshi, a put struck at K...
        auto put1 = ext::make_shared<EuropeanOption>(payoff, exercise);
        put1->setPricingEngine(europeanEngine);
        portfolio1.add(put1);
        portfolio2.add(put1);
        portfolio3.add(put1);
        // ...minus a digital put struck at B of notional K-B...
        auto digitalPayoff = ext::make_shared<CashOrNothingPayoff>(Option::Put, barrier, 1.0);
        auto digitalPut = ext::make_shared<EuropeanOption>(digitalPayoff, exercise);
        digitalPut->setPricingEngine(europeanEngine);
        portfolio1.subtract(digitalPut, strike-barrier);
        portfolio2.subtract(digitalPut, strike-barrier);
        portfolio3.subtract(digitalPut, strike-barrier);
        // ...minus a put option struck at B.
        auto lowerPayoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, barrier);
        auto put2 = ext::make_shared<EuropeanOption>(lowerPayoff, exercise);
        put2->setPricingEngine(europeanEngine);
        portfolio1.subtract(put2);
        portfolio2.subtract(put2);
        portfolio3.subtract(put2);

        // Now we use puts struck at B to kill the value of the
        // portfolio on a number of points (B,t).  For the first
        // portfolio, we'll use 12 dates at one-month's distance.
        Integer i;
        for (i=12; i>=1; i--) {
            // First, we instantiate the option...
            Date innerMaturity = today + i*Months;
            auto innerExercise = ext::make_shared<EuropeanExercise>(innerMaturity);
            auto innerPayoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, barrier);
            auto putn = ext::make_shared<EuropeanOption>(innerPayoff, innerExercise);
            putn->setPricingEngine(europeanEngine);
            // ...second, we evaluate the current portfolio and the
            // latest put at (B,t)...
            Date killDate = today + (i-1)*Months;
            Settings::instance().evaluationDate() = killDate;
            underlying->setValue(barrier);
            Real portfolioValue = portfolio1.NPV();
            Real putValue = putn->NPV();
            // ...finally, we estimate the notional that kills the
            // portfolio value at that point...
            Real notional = portfolioValue/putValue;
            // ...and we subtract from the portfolio a put with such
            // notional.
            portfolio1.subtract(putn, notional);
        }
        // The portfolio being complete, we return to today's market...
        Settings::instance().evaluationDate() = today;
        underlying->setValue(underlyingValue);
        // ...and output the value.
        Real portfolioValue = portfolio1.NPV();
        Real error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (12 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;

        // For the second portfolio, we'll use 26 dates at two-weeks'
        // distance.
        for (i=52; i>=2; i-=2) {
            // Same as above.
            Date innerMaturity = today + i*Weeks;
            auto innerExercise = ext::make_shared<EuropeanExercise>(innerMaturity);
            auto innerPayoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, barrier);
            auto putn = ext::make_shared<EuropeanOption>(innerPayoff, innerExercise);
            putn->setPricingEngine(europeanEngine);
            Date killDate = today + (i-2)*Weeks;
            Settings::instance().evaluationDate() = killDate;
            underlying->setValue(barrier);
            Real portfolioValue = portfolio2.NPV();
            Real putValue = putn->NPV();
            Real notional = portfolioValue/putValue;
            portfolio2.subtract(putn, notional);
        }
        Settings::instance().evaluationDate() = today;
        underlying->setValue(underlyingValue);
        portfolioValue = portfolio2.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (26 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;

        // For the third portfolio, we'll use 52 dates at one-week's
        // distance.
        for (i=52; i>=1; i--) {
            // Same as above.
            Date innerMaturity = today + i*Weeks;
            auto innerExercise = ext::make_shared<EuropeanExercise>(innerMaturity);
            auto innerPayoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, barrier);
            auto putn = ext::make_shared<EuropeanOption>(innerPayoff, innerExercise);
            putn->setPricingEngine(europeanEngine);
            Date killDate = today + (i-1)*Weeks;
            Settings::instance().evaluationDate() = killDate;
            underlying->setValue(barrier);
            Real portfolioValue = portfolio3.NPV();
            Real putValue = putn->NPV();
            Real notional = portfolioValue/putValue;
            portfolio3.subtract(putn, notional);
        }
        Settings::instance().evaluationDate() = today;
        underlying->setValue(underlyingValue);
        portfolioValue = portfolio3.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (52 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;

        // Now we modify the market condition to see whether the
        // replication holds. First, we change the underlying value so
        // that the option is out of the money.
        std::cout << dblrule << std::endl;
        std::cout << "Modified market conditions: out of the money"
                  << std::endl;
        std::cout << dblrule << std::endl;
        std::cout << std::setw(widths[0]) << std::left << "Option"
                  << std::setw(widths[1]) << std::left << "NPV"
                  << std::setw(widths[2]) << std::left << "Error"
                  << std::endl;
        std::cout << rule << std::endl;

        underlying->setValue(110.0);

        referenceValue = referenceOption.NPV();
        std::cout << std::setw(widths[0]) << std::left
                  << "Original barrier option"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << referenceValue
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::endl;
        portfolioValue = portfolio1.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (12 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;
        portfolioValue = portfolio2.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (26 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;
        portfolioValue = portfolio3.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (52 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;

        // Next, we change the underlying value so that the option is
        // in the money.
        std::cout << dblrule << std::endl;
        std::cout << "Modified market conditions: in the money" << std::endl;
        std::cout << dblrule << std::endl;
        std::cout << std::setw(widths[0]) << std::left << "Option"
                  << std::setw(widths[1]) << std::left << "NPV"
                  << std::setw(widths[2]) << std::left << "Error"
                  << std::endl;
        std::cout << rule << std::endl;

        underlying->setValue(90.0);

        referenceValue = referenceOption.NPV();
        std::cout << std::setw(widths[0]) << std::left
                  << "Original barrier option"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << referenceValue
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::endl;
        portfolioValue = portfolio1.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (12 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;
        portfolioValue = portfolio2.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (26 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;
        portfolioValue = portfolio3.NPV();
        error = portfolioValue - referenceValue;
        std::cout << std::setw(widths[0]) << std::left
                  << "Replicating portfolio (52 dates)"
                  << std::fixed
                  << std::setw(widths[1]) << std::left << portfolioValue
                  << std::setw(widths[2]) << std::left << error
                  << std::endl;

        // Finally, a word of warning for those (shame on them) who
        // run the example but do not read the code.
        std::cout << dblrule << std::endl;
        std::cout
            << std::endl
            << "The replication seems to be less robust when volatility and \n"
            << "risk-free rate are changed. Feel free to experiment with \n"
            << "the example and contribute a patch if you spot any errors."
            << std::endl;

        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
