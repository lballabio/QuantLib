
/*!
 2003 Neil Firth 
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


int main(int argc, char* argv[])
{
    try {
        QL_IO_INIT

        std::cout << "Using " << QL_VERSION << std::endl << std::endl;

        // our option
        Option::Type type(Option::Put);
        double underlying = 36;
        double strike = 40;
        Spread dividendYield = 0.00;
        Rate riskFreeRate = 0.06;
        double volatility = 0.20;

        Date todaysDate(15, May, 1998);
        Date settlementDate(17, May, 1998);
        Date exerciseDate(17, May, 1999);
        DayCounter rateDayCounter = Actual365();
        Time maturity = rateDayCounter.yearFraction(settlementDate,
                                                    exerciseDate);

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

        double value, discrepancy, rightValue, relativeDiscrepancy;
        rightValue = (type == Option::Put ? 4.4867 : 2.1737);

        std::cout << std::endl ;

        // write column headings
        std::cout << "Method\t\tValue\tEstimatedError\tDiscrepancy"
            "\tRel. Discr." << std::endl;

        Date midlifeDate(19, November, 1998);
        std::vector<Date> exDates(2);
        exDates[0]=midlifeDate;
        exDates[1]=exerciseDate;

        EuropeanExercise exercise(exerciseDate);
        AmericanExercise amExercise(settlementDate, exerciseDate);
        BermudanExercise berExercise(exDates);


        RelinkableHandle<Quote> underlyingH(
            Handle<Quote>(new SimpleQuote(underlying)));

        // bootstrap the yield/dividend/vol curves
        RelinkableHandle<TermStructure> flatTermStructure(
            Handle<TermStructure>(
                new FlatForward(todaysDate, settlementDate,
                                riskFreeRate, rateDayCounter)));
        RelinkableHandle<TermStructure> flatDividendTS(
            Handle<TermStructure>(
                new FlatForward(todaysDate, settlementDate,
                                dividendYield, rateDayCounter)));
        RelinkableHandle<BlackVolTermStructure> flatVolTS(
            Handle<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, volatility)));

        std::vector<Date> dates(4);
        dates[0] = settlementDate.plusMonths(1);
        dates[1] = exerciseDate;
        dates[2] = exerciseDate.plusMonths(6);
        dates[3] = exerciseDate.plusMonths(12);
        std::vector<double> strikes(4);
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
        RelinkableHandle<BlackVolTermStructure> blackSurface(
            Handle<BlackVolTermStructure>(
                new BlackVarianceSurface(settlementDate, dates, strikes, vols)));

        // European option
        VanillaOption euroOption(
            type,
            underlyingH,
            strike,
            flatDividendTS,
            flatTermStructure,
            exercise,
            flatVolTS,
            Handle<PricingEngine>(new AnalyticEuropeanEngine()));

        // method: Black Scholes Engine
        method = "european ";
        value = euroOption.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // American option
        VanillaOption option(
            type,
            underlyingH,
            strike,
            flatDividendTS,
            flatTermStructure,
            amExercise,
            flatVolTS);

        Size timeSteps = 800;

        // Binomial Method (JR)
        method = "Binomial (JR)";
        option.setPricingEngine(Handle<PricingEngine>(
            new BinomialVanillaEngine<JarrowRudd>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;


        // Binomial Method (CRR)
        method = "Binomial (CRR)";
        option.setPricingEngine(Handle<PricingEngine>(
            new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Equal Probability Additive Binomial Tree (EQP)
        method = "Additive (EQP)";
        option.setPricingEngine(Handle<PricingEngine>(
            new BinomialVanillaEngine<AdditiveEQPBinomialTree>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Equal Jumps Additive Binomial Tree (Trigeorgis)
        method = "Bin. Trigeorgis";
        option.setPricingEngine(Handle<PricingEngine>(
            new BinomialVanillaEngine<Trigeorgis>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Tian Binomial Tree (third moment matching)
        method = "Binomial Tian";
        option.setPricingEngine(Handle<PricingEngine>(
            new BinomialVanillaEngine<Tian>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Least Squares Monte Carlo: Longstaff Schwartz
        method = "LSMC (monomial)";
        Size mcSeed = 12345;
        Size nSamples = 5000;
        timeSteps = 100;
        option.setPricingEngine(Handle<PricingEngine>(
            new AmericanMCVanillaEngine(nSamples, timeSteps, mcSeed)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
            << DoubleFormatter::toString(value, 4) << "\t"
            << "N/A\t\t"
            << DoubleFormatter::toString(discrepancy, 6) << "\t"
            << DoubleFormatter::toString(relativeDiscrepancy, 6)
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
