
/*!
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

/*  This example computes profit and loss of a discrete interval hedging
    strategy and compares with the results of Derman & Kamal's (Goldman Sachs
    Equity Derivatives Research) Research Note: "When You Cannot Hedge
    Continuously: The Corrections to Black-Scholes"
    http://www.ederman.com/emanuelderman/GSQSpapers/when_you_cannot_hedge.pdf

    Suppose an option hedger sells an European option and receives the
    Black-Scholes value as the options premium.
    Then he follows a Black-Scholes hedging strategy, rehedging at discrete,
    evenly spaced time intervals as the underlying stock changes. At
    expiration, the hedger delivers the option payoff to the option holder,
    and unwinds the hedge. We are interested in understanding the final
    profit or loss of this strategy.

    If the hedger had followed the exact Black-Scholes replication strategy,
    re-hedging continuously as the underlying stock evolved towards its final
    value at expiration, then, no matter what path the stock took, the final
    P&L would be exactly zero. When the replication strategy deviates from
    the exact Black-Scholes method, the final P&L may deviate from zero. This
    deviation is called the replication error. When the hedger rebalances at
    discrete rather than continuous intervals, the hedge is imperfect and the
    replication is inexact. The more often hedging occurs, the smaller the
    replication error.

    We examine the range of possibilities, computing the replication error.
*/

// the only header you need to use QuantLib
#include <ql/quantlib.hpp>

using namespace QuantLib;

/* The ReplicationError class carries out Monte Carlo simulations to evaluate
   the outcome (the replication error) of the discrete hedging strategy over
   different, randomly generated scenarios of future stock price evolution.
*/
class ReplicationError
{
public:
    ReplicationError(Option::Type type,
                     Time maturity,
                     Real strike,
                     Real s0,
                     Volatility sigma,
                     Rate r)
    : maturity_(maturity), payoff_(type, strike), s0_(s0),
      sigma_(sigma), r_(r) {

        // value of the option
        DiscountFactor rDiscount = QL_EXP(-r_*maturity_);
        DiscountFactor qDiscount = 1.0;
        Real forward = s0_*qDiscount/rDiscount;
        Real variance = sigma_*sigma_*maturity_;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                             new PlainVanillaPayoff(payoff_));
        BlackFormula black(forward,rDiscount,variance,payoff);
        std::cout << "Option value: " << black.value() << std::endl;

        // store option's vega, since Derman and Kamal's formula needs it
        vega_ = black.vega(maturity_);

        std::cout << std::endl;
        std::cout <<
            "        |        | P&L  \t|  P&L    | Derman&Kamal | P&L"
            "      \t| P&L" << std::endl;

        std::cout <<
            "samples | trades | Mean \t| Std Dev | Formula      |"
            " skewness \t| kurt." << std::endl;

        std::cout << "---------------------------------"
            "----------------------------------------------" << std::endl;
    }

    // the actual replication error computation
    void compute(Size nTimeSteps, Size nSamples);
private:
    Time maturity_;
    PlainVanillaPayoff payoff_;
    Real s0_;
    Volatility sigma_;
    Rate r_;
    Real vega_;
};

// The key for the MonteCarlo simulation is to have a PathPricer that
// implements a value(const Path& path) method.
// This method prices the portfolio for each Path of the random variable
class ReplicationPathPricer : public PathPricer<Path>
{
  public:
    // real constructor
    ReplicationPathPricer(Option::Type type,
                          Real underlying,
                          Real strike,
                          Rate r,
                          Time maturity,
                          Volatility sigma)
    : type_(type), underlying_(underlying),
      strike_(strike), r_(r), maturity_(maturity), sigma_(sigma) {
        QL_REQUIRE(strike_ > 0.0, "strike must be positive");
        QL_REQUIRE(underlying_ > 0.0, "underlying must be positive");
        QL_REQUIRE(r_ >= 0.0,
                   "risk free rate (r) must be positive or zero");
        QL_REQUIRE(maturity_ > 0.0, "maturity must be positive");
        QL_REQUIRE(sigma_ >= 0.0,
                   "volatility (sigma) must be positive or zero");

    }
    // The value() method encapsulates the pricing code
    Real operator()(const Path& path) const;

  private:
    Option::Type type_;
    Real underlying_, strike_;
    Rate r_;
    Time maturity_;
    Volatility sigma_;
};


// Compute Replication Error as in the Derman and Kamal's research note
int main(int, char* [])
{
    try {
        QL_IO_INIT

        Time maturity = 1.0/12.0;   // 1 month
        Real strike = 100;
        Real underlying = 100;
        Volatility volatility = 0.20; // 20%
        Rate riskFreeRate = 0.05; // 5%
        ReplicationError rp(Option::Call, maturity, strike, underlying,
                volatility, riskFreeRate);

        Size scenarios = 50000;
        Size hedgesNum;

        hedgesNum = 21;
        rp.compute(hedgesNum, scenarios);

        hedgesNum = 84;
        rp.compute(hedgesNum, scenarios);

        return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}


/* The actual computation of the Profit&Loss for each single path.

   In each scenario N rehedging trades spaced evenly in time over
   the life of the option are carried out, using the Black-Scholes
   hedge ratio.
*/
Real ReplicationPathPricer::operator()(const Path& path) const
{

    // path is an instance of QuantLib::Path
    // It contains the list of variations.
    // It can be used as an array: it has a size() method
    Size n = path.size();
    QL_REQUIRE(n>0, "the path cannot be empty");

    // discrete hedging interval
    Time dt = maturity_/n;

    // For simplicity, we assume the stock pays no dividends.
    Rate stockDividendYield = 0.0;

    // let's start
    Time t = 0;

    // stock value at t=0
    Real stock = underlying_;
    Real stockLogGrowth = 0.0;

    // money account at t=0
    Real money_account = 0.0;

    /************************/
    /*** the initial deal ***/
    /************************/
    // option fair price (Black-Scholes) at t=0
    DiscountFactor rDiscount = QL_EXP(-r_*maturity_);
    DiscountFactor qDiscount = QL_EXP(-stockDividendYield*maturity_);
    Real forward = stock*qDiscount/rDiscount;
    Real variance = sigma_*sigma_*maturity_;
    boost::shared_ptr<StrikedTypePayoff> payoff(
                                       new PlainVanillaPayoff(type_,strike_));
    BlackFormula black(forward,rDiscount,variance,payoff);
    // sell the option, cash in its premium
    money_account += black.value();
    // compute delta
    Real delta = black.delta(stock);
    // delta-hedge the option buying stock
    Real stockAmount = delta;
    money_account -= stockAmount*stock;

    /**********************************/
    /*** hedging during option life ***/
    /**********************************/
    for (Size step = 0; step < n-1; step++){

        // time flows
        t += dt;

        // accruing on the money account
        money_account *= QL_EXP( r_*dt );

        // stock growth:
        // path contains the list of Gaussian variations
        // and path[n] is the n-th variation
        stockLogGrowth += path[step];
        stock = underlying_*QL_EXP(stockLogGrowth);

        // recalculate option value at the current stock value,
        // and the current time to maturity
        rDiscount = QL_EXP(-r_*(maturity_-t));
        qDiscount = QL_EXP(-stockDividendYield*(maturity_-t));
        forward = stock*qDiscount/rDiscount;
        variance = sigma_*sigma_*(maturity_-t);
        BlackFormula black(forward,rDiscount,variance,payoff);

        // recalculate delta
        delta = black.delta(stock);

        // re-hedging
        money_account -= (delta - stockAmount)*stock;
        stockAmount = delta;
    }

    /*************************/
    /*** option expiration ***/
    /*************************/
    // last accrual on my money account
    money_account *= QL_EXP( r_*dt );
    // last stock growth
    stockLogGrowth += path[n-1];
    stock = underlying_*QL_EXP(stockLogGrowth);

    // the hedger delivers the option payoff to the option holder
    Real optionPayoff = PlainVanillaPayoff(type_, strike_)(stock);
    money_account -= optionPayoff;

    // and unwinds the hedge selling his stock position
    money_account += stockAmount*stock;

    // final Profit&Loss
    return money_account;
}


// The computation over nSamples paths of the P&L distribution
void ReplicationError::compute(Size nTimeSteps, Size nSamples)
{
    QL_REQUIRE(nTimeSteps>0, "the number of steps must be > 0");

    // hedging interval
    // Time tau = maturity_ / nTimeSteps;

    /* Black-Scholes framework: the underlying stock price evolves
       lognormally with a fixed known volatility that stays constant
       throughout time.
    */
    Date today = Date::todaysDate();
    DayCounter dayCount = Actual365Fixed();
    Handle<Quote> stateVariable(
                          boost::shared_ptr<Quote>(new SimpleQuote(s0_)));
    Handle<YieldTermStructure> riskFreeRate(
                          boost::shared_ptr<YieldTermStructure>(
                                      new FlatForward(today, r_, dayCount)));
    Handle<YieldTermStructure> dividendYield(
                          boost::shared_ptr<YieldTermStructure>(
                                      new FlatForward(today, 0.0, dayCount)));
    Handle<BlackVolTermStructure> volatility(
                          boost::shared_ptr<BlackVolTermStructure>(
                               new BlackConstantVol(today, sigma_, dayCount)));
    boost::shared_ptr<StochasticProcess> diffusion(
                         new BlackScholesProcess(stateVariable, dividendYield,
                                                 riskFreeRate, volatility));

    // Black Scholes equation rules the path generator:
    // at each step the log of the stock
    // will have drift and sigma^2 variance
    PseudoRandom::rsg_type rsg =
        PseudoRandom::make_sequence_generator(nTimeSteps, 0);

    bool brownianBridge = false;

    typedef SingleAsset<PseudoRandom>::path_generator_type generator_type;
    boost::shared_ptr<generator_type> myPathGenerator(new
        generator_type(diffusion, maturity_, nTimeSteps,
                       rsg, brownianBridge));

    // The replication strategy's Profit&Loss is computed for each path
    // of the stock. The path pricer knows how to price a path using its
    // value() method
    boost::shared_ptr<PathPricer<Path> > myPathPricer(new
        ReplicationPathPricer(payoff_.optionType(), s0_,
            payoff_.strike(), r_, maturity_, sigma_));

    // a statistics accumulator for the path-dependant Profit&Loss values
    Statistics statisticsAccumulator;

    // The OneFactorMontecarloModel generates paths using myPathGenerator
    // each path is priced using myPathPricer
    // prices will be accumulated into statisticsAccumulator
    OneFactorMonteCarloOption MCSimulation(myPathGenerator,
                                           myPathPricer,
                                           statisticsAccumulator,
                                           false);

    // the model simulates nSamples paths
    MCSimulation.addSamples(nSamples);

    // the sampleAccumulator method of OneFactorMonteCarloOption_old
    // gives access to all the methods of statisticsAccumulator
    Real PLMean  = MCSimulation.sampleAccumulator().mean();
    Real PLStDev = MCSimulation.sampleAccumulator().standardDeviation();
    Real PLSkew  = MCSimulation.sampleAccumulator().skewness();
    Real PLKurt  = MCSimulation.sampleAccumulator().kurtosis();

    // Derman and Kamal's formula
    Real theorStD = QL_SQRT(M_PI/4/nTimeSteps)*vega_*sigma_;


    std::cout << nSamples << "\t| "
        << nTimeSteps << "\t | "
        << DecimalFormatter::toString(PLMean,   3) << " \t| "
        << DecimalFormatter::toString(PLStDev,  2) << " \t  | "
        << DecimalFormatter::toString(theorStD, 2) << " \t | "
        << DecimalFormatter::toString(PLSkew,   2) << " \t| "
        << DecimalFormatter::toString(PLKurt,   2) << std::endl;
}
