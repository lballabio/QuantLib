
/*!
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

// This will be included in the library after a bit of redesign
class WeightedPayoff {
    public:
        WeightedPayoff(Option::Type type,
               Time maturity,
               Real strike,
               Real s0,
               Volatility sigma,
               Rate r,
               Rate q)
        : type_(type), maturity_(maturity),
        strike_(strike),
        s0_(s0),
        sigma_(sigma),r_(r), q_(q){}

        Real operator()(Real x) const {
           Real nuT = (r_-q_-0.5*sigma_*sigma_)*maturity_;
           return QL_EXP(-r_*maturity_)
               *PlainVanillaPayoff(type_, strike_)(s0_*QL_EXP(x))
               *QL_EXP(-(x - nuT)*(x -nuT)/(2*sigma_*sigma_*maturity_))
               /QL_SQRT(2.0*M_PI*sigma_*sigma_*maturity_);
        }
private:
    Option::Type type_;
    Time maturity_;
    Real strike_;
    Real s0_;
    Volatility sigma_;
    Rate r_,q_;
};


int main(int, char* [])
{
    try {
        QL_IO_INIT

        std::cout << "Using " << QL_VERSION << std::endl << std::endl;

        // our option
        Option::Type type(Option::Call);
        Real underlying = 7;
        Real strike = 8;
        Spread dividendYield = 0.05;
        Rate riskFreeRate = 0.05;

        Date todaysDate(15, May, 1998);
        Date settlementDate(17, May, 1998);
        Settings::instance().setEvaluationDate(todaysDate);

        Date exerciseDate(17, May, 1999);
        DayCounter dayCounter = Actual365Fixed();
        Time maturity = dayCounter.yearFraction(settlementDate,
                                                exerciseDate);

        Volatility volatility = 0.10;
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
                new FlatForward(settlementDate, riskFreeRate, dayCounter)));
        Handle<YieldTermStructure> flatDividendTS(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, dividendYield, dayCounter)));
        Handle<BlackVolTermStructure> flatVolTS(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, volatility, dayCounter)));

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
        vols[0][0] = volatility*1.1;
                     vols[0][1] = volatility;
                                  vols[0][2] = volatility*0.9;
                                               vols[0][3] = volatility*0.8;
        vols[1][0] = volatility*1.1;
                     vols[1][1] = volatility;
                                  vols[1][2] = volatility*0.9;
                                               vols[1][3] = volatility*0.8;
        vols[2][0] = volatility*1.1;
                     vols[2][1] = volatility;
                                  vols[2][2] = volatility*0.9;
                                               vols[2][3] = volatility*0.8;
        vols[3][0] = volatility*1.1;
                     vols[3][1] = volatility;
                                  vols[3][2] = volatility*0.9;
                                               vols[3][3] = volatility*0.8;

        Handle<BlackVolTermStructure> blackSurface(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackVarianceSurface(settlementDate, dates,
                                         strikes, vols, dayCounter)));


        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(type, strike));

        boost::shared_ptr<BlackScholesProcess> stochasticProcess(new
            BlackScholesProcess(underlyingH, flatDividendTS,
                                flatTermStructure,
                                //  blackSurface
                                flatVolTS));

        EuropeanOption option(stochasticProcess, payoff, exercise);


        std::string method;
        Real value, discrepancy, rightValue, relativeDiscrepancy;

        std::cout << std::endl << std::endl;

        // write column headings
        std::cout << "Method\t\tValue\tEstimatedError\tDiscrepancy"
                     "\tRel. Discr." << std::endl;

        // method: Black-Scholes Engine
        method = "Black-Scholes";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new AnalyticEuropeanEngine()));
        rightValue = value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;


        // method: Integral
        method = "Integral";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new IntegralEngine()));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

/*
        // method: Integral
        method = "Binary Cash";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new IntegralCashOrNothingEngine(1.0)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // method: Integral
        method = "Binary Asset";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new IntegralAssetOrNothingEngine()));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

*/
        Size timeSteps = 801;

        // Binomial Method (JR)
        method = "Binomial (JR)";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<JarrowRudd>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;


        // Binomial Method (CRR)
        method = "Binomial (CRR)";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Equal Probability Additive Binomial Tree (EQP)
        method = "Additive (EQP)";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<AdditiveEQPBinomialTree>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Equal Jumps Additive Binomial Tree (Trigeorgis)
        method = "Bin. Trigeorgis";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Trigeorgis>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Tian Binomial Tree (third moment matching)
        method = "Binomial Tian";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Tian>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Leisen-Reimer Binomial Tree
        method = "Binomial LR";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<LeisenReimer>(timeSteps)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Finite Differences Method: not implemented

        /*method = "Finite Diff.";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FDVanillaEngine()));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;*/


        // Monte Carlo Method
        timeSteps = 1;

        method = "MC (crude)";
        Size mcSeed = 42;

        boost::shared_ptr<PricingEngine> mcengine1;
        mcengine1 =
            MakeMCEuropeanEngine<PseudoRandom>().withStepsPerYear(timeSteps)
                                                .withTolerance(0.02)
                                                .withSeed(mcSeed);
        option.setPricingEngine(mcengine1);

        value = option.NPV();
        Real errorEstimate = option.errorEstimate();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << DecimalFormatter::toString(errorEstimate, 4) << "\t\t"
             << DecimalFormatter::toString(discrepancy, 6) << "\t"
             << DecimalFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        method = "MC (Sobol)";
        timeSteps = 1;
        Size nSamples = 32768;  // 2^15

        boost::shared_ptr<PricingEngine> mcengine2;
        mcengine2 =
            MakeMCEuropeanEngine<LowDiscrepancy>().withStepsPerYear(timeSteps)
                                                  .withSamples(nSamples);
        option.setPricingEngine(mcengine2);

        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DecimalFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
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
