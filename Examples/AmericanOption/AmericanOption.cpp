
/*!
 Copyright (C) 2002 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth
 Adapted from EuropeanOption.cpp

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


int main(int, char* [])
{
    try {
        QL_IO_INIT

        std::cout << "Using " << QL_VERSION << std::endl << std::endl;

        // our option
        Option::Type type(Option::Put);
        Real underlying = 36;
        Real strike = 40;
        Spread dividendYield = 0.00;
        Rate riskFreeRate = 0.06;
        Volatility volatility = 0.20;

        Date todaysDate(15, May, 1998);
        Date settlementDate(17, May, 1998);
        Settings::instance().setEvaluationDate(todaysDate);

        Date exerciseDate(17, May, 1999);
        DayCounter rateDayCounter = Actual365Fixed();
        Time maturity = rateDayCounter.yearFraction(settlementDate,
                                                    exerciseDate);

        std::cout << "option type = "  << OptionTypeFormatter::toString(type)
                  << std::endl;
        std::cout << "Time to maturity = "        << maturity
                  << std::endl;
        std::cout << "Underlying price = "        << underlying
                  << std::endl;
        std::cout << "Strike = "                  << strike
                  << std::endl;
        std::cout << "Risk-free interest rate = " << riskFreeRate
                  << std::endl;
        std::cout << "dividend yield = " << dividendYield
                  << std::endl;
        std::cout << "Volatility = "              << volatility
                  << std::endl;
        std::cout << std::endl;

        std::string method;

        Real value, discrepancy, rightValue, relativeDiscrepancy;
        rightValue = (type == Option::Put ? 4.48667344 : 2.17372645);

        std::cout << std::endl ;

        // write column headings
        std::cout << "Method\t\t\t\t  Value\t\tDiscrepancy"
            "\tRel. Discr." << std::endl;

        Date midlifeDate(19, November, 1998);
        std::vector<Date> exDates(2);
        exDates[0]=midlifeDate;
        exDates[1]=exerciseDate;

        boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));
        boost::shared_ptr<Exercise> amExercise(
                                          new AmericanExercise(settlementDate,
                                                               exerciseDate));
        boost::shared_ptr<Exercise> berExercise(new BermudanExercise(exDates));


        Handle<Quote> underlyingH(
            boost::shared_ptr<Quote>(new SimpleQuote(underlying)));

        // bootstrap the yield/dividend/vol curves
        Handle<YieldTermStructure> flatTermStructure(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate,
                                riskFreeRate, rateDayCounter)));
        Handle<YieldTermStructure> flatDividendTS(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate,
                                dividendYield, rateDayCounter)));
        Handle<BlackVolTermStructure> flatVolTS(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, volatility)));

        std::vector<Date> dates(4);
        dates[0] = settlementDate + 1*Months;
        dates[1] = exerciseDate;
        dates[2] = exerciseDate + 6*Months;
        dates[3] = exerciseDate + 12*Months;
        std::vector<Real> strikes(4);
        strikes[0] = underlying*0.9;
        strikes[1] = underlying;
        strikes[2] = underlying*1.1;
        strikes[3] = underlying*1.2;

        Matrix vols(4,4);
        vols[0][0] = volatility*1.1; vols[0][1] = volatility;
            vols[0][2] = volatility*0.9; vols[0][3] = volatility*0.8;
        vols[1][0] = volatility*1.1; vols[1][1] = volatility;
            vols[1][2] = volatility*0.9; vols[1][3] = volatility*0.8;
        vols[2][0] = volatility*1.1; vols[2][1] = volatility;
            vols[2][2] = volatility*0.9; vols[2][3] = volatility*0.8;
        vols[3][0] = volatility*1.1; vols[3][1] = volatility;
            vols[3][2] = volatility*0.9; vols[3][3] = volatility*0.8;
        Handle<BlackVolTermStructure> blackSurface(
            boost::shared_ptr<BlackVolTermStructure>(new
                BlackVarianceSurface(settlementDate, dates, strikes, vols)));

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(type, strike));

        boost::shared_ptr<BlackScholesProcess> stochasticProcess(new
            BlackScholesProcess(
                underlyingH,
                flatDividendTS,
                flatTermStructure,
                flatVolTS));

        // European option
        VanillaOption euroOption(stochasticProcess, payoff, exercise,
            boost::shared_ptr<PricingEngine>(new AnalyticEuropeanEngine()));

        // method: Black Scholes Engine
        method = "equivalent European option       ";
        value = euroOption.NPV();
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << "N/A\t\t"
             << "N/A\t\t"
             << std::endl;

        // American option
        VanillaOption option(stochasticProcess, payoff, amExercise);

        Size timeSteps = 801;

        // Binomial Method (JR)
        method = "Binomial Jarrow-Rudd             ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<JarrowRudd>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Binomial Method (CRR)
        method = "Binomial Cox-Ross-Rubinstein     ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Equal Probability Additive Binomial Tree (EQP)
        method = "Additive equiprobabilities       ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<AdditiveEQPBinomialTree>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Equal Jumps Additive Binomial Tree (Trigeorgis)
        method = "Binomial Trigeorgis              ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Trigeorgis>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Tian Binomial Tree (third moment matching)
        method = "Binomial Tian                    ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Tian>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Leisen-Reimer Binomial Tree
        method = "Binomial Leisen-Reimer           ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<LeisenReimer>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Barone-Adesi and Whaley approximation
        method = "Barone-Adesi and Whaley approx.  ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BaroneAdesiWhaleyApproximationEngine));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Bjerksund and Stensland approximation
        method = "Bjerksund and Stensland approx.  ";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BjerksundStenslandApproximationEngine));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << " "
             << DecimalFormatter::toString(value, 6) << "\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}
