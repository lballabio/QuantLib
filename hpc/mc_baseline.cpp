#include <ql/quantlib.hpp>
#include <iostream>
#include <chrono>
#include <cmath>

using namespace QuantLib;

int main() {
    Calendar calendar = TARGET();
    Date today(3, October, 2026);
    Settings::instance().evaluationDate() = today;

    DayCounter dc = Actual365Fixed();
    Date maturity = today + Period(6, Months);

    std::cout << "Today: " << today << std::endl;
    std::cout << "Maturity: " << maturity << std::endl;

    Real spot = 100.0;
    Real strike = 100.0;
    Rate r = 0.05;
    Rate q = 0.00;
    Volatility sigma = 0.2;

    Handle<Quote> spotHandle(ext::make_shared<SimpleQuote>(spot));
    Handle<YieldTermStructure> riskFreeRate(ext::make_shared<FlatForward>(today, r, dc));
    Handle<YieldTermStructure> dividendYield(ext::make_shared<FlatForward>(today, q, dc));
    Handle<BlackVolTermStructure> volatility(ext::make_shared<BlackConstantVol>(today, calendar, sigma, dc));

    //auto gives a shared pointer here in QuantLib, which is a smart pointer that manages the lifetime of the object it points to. This is useful for managing resources and ensuring that the object is properly deleted when it is no longer needed.
    auto process = ext::make_shared<BlackScholesMertonProcess>(spotHandle, dividendYield, riskFreeRate, volatility);

    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Call, strike);
    auto exercise = ext::make_shared<EuropeanExercise>(maturity);
    VanillaOption option(payoff, exercise);

    //analytical reference
    option.setPricingEngine(ext::make_shared<AnalyticEuropeanEngine>(process));
    Real analytic = option.NPV();

    std::cout << "Analytical NPV: " << analytic << std::endl;
    std::cout << "paths MC price stderr |diff|/stderr seconds" << std::endl;

    for (Size n : {1000, 10000, 100000, 1000000}) {
        option.setPricingEngine(MakeMCEuropeanEngine<PseudoRandom>(process).withSteps(1).withSamples(n).withSeed(1000)); //seed kinda fixed things
        auto start = std::chrono::high_resolution_clock::now();
        Real mc = option.NPV();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        Real stderr = option.errorEstimate();
        Real diff = std::fabs(mc - analytic);
        std::cout << n << " " << mc << " " << stderr << " " << diff / stderr << " " << elapsed.count() << std::endl;
    }

    std::cout << std::endl;

    //american exercise: no closed form, use a binomial tree as the reference price instead
    auto americanExercise = ext::make_shared<AmericanExercise>(today, maturity);
    VanillaOption americanOption(payoff, americanExercise);

    americanOption.setPricingEngine(ext::make_shared<BinomialVanillaEngine<CoxRossRubinstein>>(process, 801));
    Real binomial = americanOption.NPV();

    std::cout << "Binomial (CRR, 801 steps) American NPV: " << binomial << std::endl;
    std::cout << "paths MC price stderr |diff|/stderr seconds" << std::endl;

    for (Size n : {1000, 10000, 100000, 1000000}) {
        americanOption.setPricingEngine(MakeMCAmericanEngine<PseudoRandom>(process)
                                             .withSteps(50)
                                             .withPolynomialOrder(2)
                                             .withSamples(n)
                                             .withSeed(1000));
        auto start = std::chrono::high_resolution_clock::now();
        Real mc = americanOption.NPV();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        Real stderrAmerican = americanOption.errorEstimate();
        Real diff = std::fabs(mc - binomial);
        std::cout << n << " " << mc << " " << stderrAmerican << " " << diff / stderrAmerican << " "
                   << elapsed.count() << std::endl;
    }
}