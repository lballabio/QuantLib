/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/bonds/convertiblebonds.hpp>
#include <ql/pricingengines/bond/binomialconvertibleengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <iostream>
#include <iomanip>

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;

int main(int, char* []) {

    try {

        std::cout << std::endl;

        Option::Type type(Option::Put);
        Real underlying = 36.0;
        Real spreadRate = 0.005;

        Spread dividendYield = 0.02;
        Rate riskFreeRate = 0.06;
        Volatility volatility = 0.20;

        Integer settlementDays = 3;
        Integer length = 5;
        Real redemption = 100.0;
        Real conversionRatio = redemption/underlying; // at the money

        // set up dates/schedules
        Calendar calendar = TARGET();
        Date today = calendar.adjust(Date::todaysDate());

        Settings::instance().evaluationDate() = today;
        Date settlementDate = calendar.advance(today, settlementDays, Days);
        Date exerciseDate = calendar.advance(settlementDate, length, Years);
        Date issueDate = calendar.advance(exerciseDate, -length, Years);

        BusinessDayConvention convention = ModifiedFollowing;

        Frequency frequency = Annual;

        Schedule schedule(issueDate, exerciseDate,
                          Period(frequency), calendar,
                          convention, convention,
                          DateGeneration::Backward, false);

        DividendSchedule dividends;
        CallabilitySchedule callability;

        std::vector<Real> coupons(1, 0.05);

        DayCounter bondDayCount = Thirty360(Thirty360::BondBasis);

        Integer callLength[] = { 2, 4 };  // Call dates, years 2, 4.
        Integer putLength[] = { 3 }; // Put dates year 3

        Real callPrices[] = { 101.5, 100.85 };
        Real putPrices[]= { 105.0 };

        // Load call schedules
        for (Size i=0; i<LENGTH(callLength); i++) {
            callability.push_back(
                   ext::make_shared<SoftCallability>(Bond::Price(callPrices[i],
                                                                 Bond::Price::Clean),
                                                     schedule.date(callLength[i]),
                                                     1.20));
        }

        for (Size j=0; j<LENGTH(putLength); j++) {
            callability.push_back(
                   ext::make_shared<Callability>(Bond::Price(putPrices[j],
                                                             Bond::Price::Clean),
                                                 Callability::Put,
                                                 schedule.date(putLength[j])));
        }

        // Assume dividends are paid every 6 months.
        for (Date d = today + 6*Months; d < exerciseDate; d += 6*Months) {
            dividends.push_back(ext::make_shared<FixedDividend>(1.0, d));
        }

        DayCounter dayCounter = Actual365Fixed();
        Time maturity = dayCounter.yearFraction(settlementDate,
                                                exerciseDate);

        std::cout << "option type = "  << type << std::endl;
        std::cout << "Time to maturity = "        << maturity
                  << std::endl;
        std::cout << "Underlying price = "        << underlying
                  << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                  << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;

        std::string method;
        std::cout << std::endl ;

        // write column headings
        Size widths[] = { 35, 14, 14 };
        Size totalWidth = widths[0] + widths[1] + widths[2];
        std::string rule(totalWidth, '-'), dblrule(totalWidth, '=');

        std::cout << dblrule << std::endl;
        std::cout << "Tsiveriotis-Fernandes method" << std::endl;
        std::cout << dblrule << std::endl;
        std::cout << std::setw(widths[0]) << std::left << "Tree type"
                  << std::setw(widths[1]) << std::left << "European"
                  << std::setw(widths[1]) << std::left << "American"
                  << std::endl;
        std::cout << rule << std::endl;

        auto exercise = ext::make_shared<EuropeanExercise>(exerciseDate);
        auto amExercise = ext::make_shared<AmericanExercise>(settlementDate, exerciseDate);

        Handle<Quote> underlyingH(ext::make_shared<SimpleQuote>(underlying));

        Handle<YieldTermStructure> flatTermStructure(
            ext::make_shared<FlatForward>(settlementDate, riskFreeRate, dayCounter));

        Handle<YieldTermStructure> flatDividendTS(
            ext::make_shared<FlatForward>(settlementDate, dividendYield, dayCounter));

        Handle<BlackVolTermStructure> flatVolTS(
            ext::make_shared<BlackConstantVol>(settlementDate, calendar, volatility, dayCounter));

        auto stochasticProcess = ext::make_shared<BlackScholesMertonProcess>(
            underlyingH, flatDividendTS, flatTermStructure, flatVolTS);

        Size timeSteps = 801;

        Handle<Quote> creditSpread(ext::make_shared<SimpleQuote>(spreadRate));

        auto rate = ext::make_shared<SimpleQuote>(riskFreeRate);

        Handle<YieldTermStructure> discountCurve(
                ext::make_shared<FlatForward>(today, Handle<Quote>(rate), dayCounter));

        ConvertibleFixedCouponBond europeanBond(
                            exercise, conversionRatio, callability,
                            issueDate, settlementDays,
                            coupons, bondDayCount, schedule, redemption);

        ConvertibleFixedCouponBond americanBond(
                          amExercise, conversionRatio, callability,
                          issueDate, settlementDays,
                          coupons, bondDayCount, schedule, redemption);

        method = "Jarrow-Rudd";
        auto jrEngine = ext::make_shared<BinomialConvertibleEngine<JarrowRudd>>(stochasticProcess, timeSteps, creditSpread, dividends);
        europeanBond.setPricingEngine(jrEngine);
        americanBond.setPricingEngine(jrEngine);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanBond.NPV()
                  << std::setw(widths[2]) << std::left << americanBond.NPV()
                  << std::endl;

        method = "Cox-Ross-Rubinstein";
        auto crrEngine = ext::make_shared<BinomialConvertibleEngine<CoxRossRubinstein>>(stochasticProcess, timeSteps, creditSpread, dividends);
        europeanBond.setPricingEngine(crrEngine);
        americanBond.setPricingEngine(crrEngine);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanBond.NPV()
                  << std::setw(widths[2]) << std::left << americanBond.NPV()
                  << std::endl;

        method = "Additive equiprobabilities";
        auto aeqpEngine = ext::make_shared<BinomialConvertibleEngine<AdditiveEQPBinomialTree>>(stochasticProcess, timeSteps, creditSpread, dividends);
        europeanBond.setPricingEngine(aeqpEngine);
        americanBond.setPricingEngine(aeqpEngine);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanBond.NPV()
                  << std::setw(widths[2]) << std::left << americanBond.NPV()
                  << std::endl;

        method = "Trigeorgis";
        auto trEngine = ext::make_shared<BinomialConvertibleEngine<Trigeorgis>>(stochasticProcess, timeSteps, creditSpread, dividends);
        europeanBond.setPricingEngine(trEngine);
        americanBond.setPricingEngine(trEngine);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanBond.NPV()
                  << std::setw(widths[2]) << std::left << americanBond.NPV()
                  << std::endl;

        method = "Tian";
        auto tianEngine = ext::make_shared<BinomialConvertibleEngine<Tian>>(stochasticProcess, timeSteps, creditSpread, dividends);
        europeanBond.setPricingEngine(tianEngine);
        americanBond.setPricingEngine(tianEngine);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanBond.NPV()
                  << std::setw(widths[2]) << std::left << americanBond.NPV()
                  << std::endl;

        method = "Leisen-Reimer";
        auto lrEngine = ext::make_shared<BinomialConvertibleEngine<LeisenReimer>>(stochasticProcess, timeSteps, creditSpread, dividends);
        europeanBond.setPricingEngine(lrEngine);
        americanBond.setPricingEngine(lrEngine);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanBond.NPV()
                  << std::setw(widths[2]) << std::left << americanBond.NPV()
                  << std::endl;

        method = "Joshi";
        auto joshiEngine = ext::make_shared<BinomialConvertibleEngine<Joshi4>>(stochasticProcess, timeSteps, creditSpread, dividends);
        europeanBond.setPricingEngine(joshiEngine);
        americanBond.setPricingEngine(joshiEngine);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanBond.NPV()
                  << std::setw(widths[2]) << std::left << americanBond.NPV()
                  << std::endl;

        std::cout << dblrule << std::endl;

        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }

}

