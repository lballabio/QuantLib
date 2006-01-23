// ConvertibleBonds.cpp : Defines the entry point for the console application.

//#include "utilities.hpp"
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
		Real conversionRatio = 2.0;
        Real strike = 40.0;
        Real spreadRate = 0.5;

        Spread dividendYield = 0.00;
        Rate riskFreeRate = 0.06;
        Volatility volatility = 0.20;

        Integer settlementDays = 3;
        Integer length = 5;
        Real redemption = 100.0;

        //! Set up Dates/Schedules
        Calendar calendar = TARGET();
        Date today = calendar.adjust(Date::todaysDate());
//        Settings::instance().setEvaluationDate(today);

		Settings::instance().evaluationDate() = today;
        Date settlementDate = calendar.advance(today, settlementDays, Days);
        Date exerciseDate = calendar.advance(today, length, Years);

		Date issueDate=today;

		BusinessDayConvention convention = ModifiedFollowing;

		Frequency frequencies[] = { Semiannual, Annual };

		Schedule schedule(calendar,issueDate,exerciseDate,
                  frequencies[1], convention,Date(), true);

        DividendSchedule dividends;
        CallabilitySchedule callability;

		std::vector<Real> coupons(1, 0.05);

		DayCounter bondDayCount = Thirty360();

//		std::vector<Date> callDates(3);
        Integer callLength[] = {2,4};  // Call dates, years 2, 4.
        Integer putLength[] = {3}; // Put dates year 3

        Real callPrices[] = {101.5,100.85};
        Real putPrices[]= {100.95};
 
        // Load Call schedules
        for (Size i=0; i<LENGTH(callLength); i++) 
        {
	  callability.push_back( 
	    Callability(Price(callPrices[i],
			      Price::Clean),
			Callability::Call,
			calendar.advance(today,callLength[i], Years)));
        }

        // Load Put schedules
        Size i = LENGTH(callLength);

		for (Size j=0; j<LENGTH(putLength); j++) 
        {

	  callability.push_back(
	      Callability(Price(putPrices[j],
				Price::Clean),
			  Callability::Put,
			  calendar.advance(today,putLength[j], Years)));	 
        }
  
        // Assume Dividends are paid every 6 months.
        Size k=0;  
        for (Date d = today + 6*Months;
             d < exerciseDate;
             d += 6*Months) {
	  dividends.push_back(boost::shared_ptr<CashFlow>(new FixedDividend(5.0, d)));
        }


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

        std::string method;

        Real value, discrepancy, rightValue, relativeDiscrepancy;
        rightValue = 0.5;

        std::cout << std::endl ;

        // write column headings
        Size widths[] = { 35, 14, 14, 14 };
        std::cout << std::setw(widths[0]) << std::left << "Method"
                  << std::setw(widths[1]) << std::left << "Value"
                  << std::setw(widths[2]) << std::left << "Discrepancy"
                  << std::setw(widths[3]) << std::left << "Rel. Discr."
                  << std::endl;

        boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));
        boost::shared_ptr<Exercise> amExercise(
                                          new AmericanExercise(settlementDate,
                                                               exerciseDate));

 //       boost::shared_ptr<Exercise> berExercise(new BermudanExercise(exDates));


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

		
		boost::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

		boost::shared_ptr<BlackScholesProcess> stochasticProcess(new
            BlackScholesProcess(
                underlyingH,
                flatDividendTS,
                flatTermStructure,
                flatVolTS));

		Size timeSteps = 801;

		Handle<Quote> CreditSpread(
            boost::shared_ptr<Quote>(new SimpleQuote(spreadRate)));

		boost::shared_ptr<SimpleQuote> rate(new SimpleQuote(0.0));
            
//		Handle<YieldTermStructure> discountCurve(flatRate(today,rate,dayCounter));

            
		Handle<YieldTermStructure> discountCurve(boost::shared_ptr<YieldTermStructure>(
                          new FlatForward(today, Handle<Quote>(rate), dayCounter)));

		boost::shared_ptr<PricingEngine> engine(new BinomialConvertibleEngine<CoxRossRubinstein>(timeSteps));

		//boost::shared_ptr<PricingEngine> engine(new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps));

		ConvertibleBond eurocvbond(stochasticProcess, payoff, exercise,engine,conversionRatio,
                                   dividends,callability,CreditSpread,issueDate,settlementDays,
                                   coupons,bondDayCount,schedule,redemption,discountCurve);

	    value = eurocvbond.NPV();
        
// American exercise convertible
//ConvertibleBond americancvbond(stochasticProcess, payoff, amExercise,
//            boost::shared_ptr<PricingEngine>(
//            new BinomialConvertibleEngine<CoxRossRubinstein>(timeSteps)),conversionRatio,
//            dividends,callability,creditSpread,issueDate,settlementDays,
//            coupons,schedule,redemption,discountCurve);


		discrepancy = std::fabs(value-rightValue);
		relativeDiscrepancy = discrepancy/rightValue;
		std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[3]) << std::left << relativeDiscrepancy
                  << std::endl;


    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }

}






