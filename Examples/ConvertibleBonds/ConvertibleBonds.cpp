
#include <ql/quantlib.hpp>
#include <iostream>

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;

int main(int argc, char* argv[])
{

  try {

		QL_IO_INIT

		std::cout << "Using " << QL_VERSION << std::endl << std::endl;

		Option::Type type(Option::Put);
        Real underlying = 36.0;
        Real strike = 40.0;
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
        Date exerciseDate = calendar.advance(today, length, Years);

		Date issueDate = today;

		BusinessDayConvention convention = ModifiedFollowing;

		Frequency frequencies[] = { Semiannual, Annual };

		Schedule schedule(calendar,issueDate,exerciseDate,
                          frequencies[1], convention,Date(), true);

        DividendSchedule dividends;
        CallabilitySchedule callability;

		std::vector<Real> coupons(1, 0.05);

		DayCounter bondDayCount = Thirty360();

        Integer callLength[] = { 2, 4 };  // Call dates, years 2, 4.
        Integer putLength[] = { 3 }; // Put dates year 3

        Real callPrices[] = { 101.5, 100.85 };
        Real putPrices[]= { 105.0 };

        // Load call schedules
        for (Size i=0; i<LENGTH(callLength); i++) {
            callability.push_back(
                   Callability(Price(callPrices[i], Price::Clean),
                               Callability::Call,
                               calendar.advance(today,callLength[i], Years)));
        }

		for (Size j=0; j<LENGTH(putLength); j++) {
            callability.push_back(
	               Callability(Price(putPrices[j], Price::Clean),
                               Callability::Put,
                               calendar.advance(today,putLength[j], Years)));
        }

        // Assume dividends are paid every 6 months.
        /* for (Date d = today + 6*Months;
             d < exerciseDate;
             d += 6*Months) {
            dividends.push_back(
                      boost::shared_ptr<CashFlow>(new FixedDividend(1.0, d)));
        }*/ 

        DayCounter dayCounter = Actual365Fixed();
        Time maturity = dayCounter.yearFraction(settlementDate,
                                                exerciseDate);

        std::cout << "option type = "  << type << std::endl;
        std::cout << "Time to maturity = "        << maturity
                  << std::endl;
        std::cout << "Underlying price = "        << underlying
                  << std::endl;
        std::cout << "Strike = "                  << strike
                  << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                  << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;

        std::string method = "Tsiveriotis-Fernandes";

        Real value;

        std::cout << std::endl ;

        // write column headings
        Size widths[] = { 35, 14 };
        std::cout << std::setw(widths[0]) << std::left << "Method"
                  << std::setw(widths[1]) << std::left << "Value"
                  << std::endl;

        boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));
        boost::shared_ptr<Exercise> amExercise(
                                          new AmericanExercise(settlementDate,
                                                               exerciseDate));

        Handle<Quote> underlyingH(
            boost::shared_ptr<Quote>(new SimpleQuote(underlying)));

		Handle<YieldTermStructure> flatTermStructure(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, riskFreeRate, dayCounter)));

        Handle<YieldTermStructure> flatDividendTS(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, dividendYield, dayCounter)));

        Handle<BlackVolTermStructure> flatVolTS(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, volatility, dayCounter)));


		boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

		boost::shared_ptr<BlackScholesProcess> stochasticProcess(
                                    new BlackScholesProcess(underlyingH,
                                                            flatDividendTS,
                                                            flatTermStructure,
                                                            flatVolTS));

		Size timeSteps = 801;

		Handle<Quote> creditSpread(
                       boost::shared_ptr<Quote>(new SimpleQuote(spreadRate)));

		boost::shared_ptr<SimpleQuote> rate(new SimpleQuote(riskFreeRate));

		Handle<YieldTermStructure> discountCurve(
                boost::shared_ptr<YieldTermStructure>(
                    new FlatForward(today, Handle<Quote>(rate), dayCounter)));

		boost::shared_ptr<PricingEngine> engine(
                 new BinomialConvertibleEngine<CoxRossRubinstein>(timeSteps));

		ConvertibleBond eurocvbond(stochasticProcess, payoff, exercise,
                                   engine, conversionRatio, dividends,
                                   callability, creditSpread, issueDate,
                                   settlementDays, coupons, bondDayCount,
                                   schedule, redemption, discountCurve);

	    value = eurocvbond.NPV();


		std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::endl;


    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }

}

