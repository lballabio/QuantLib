
/*
 * Copyright (C) 2000, 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 *   http://quantlib.sourceforge.net/LICENSE.TXT
*/

/* $Source$
   $Log$
   Revision 1.27  2001/04/04 09:51:18  marmar
   introducing cliquet option

   Revision 1.26  2001/03/22 16:37:15  marmar
   Barrier option greeks included

   Revision 1.25  2001/03/21 09:54:56  marmar
   BermudanOption added, DividendOption removed

   Revision 1.24  2001/03/20 15:13:59  marmar
   MultiPeriodOption is a generalization of DividendAmericanOption

   Revision 1.23  2001/03/12 12:59:01  marmar
   __str__ now represents the object while __repr__ is unchanged

   Revision 1.22  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_pricers_i
#define quantlib_pricers_i

%include Date.i
%include Options.i
%include Financial.i
%include Vectors.i

%{
using QuantLib::Pricers::AmericanOption;
using QuantLib::Pricers::BinaryOption;
using QuantLib::Pricers::BermudanOption;
using QuantLib::Pricers::BSMEuropeanOption;
using QuantLib::Pricers::CliquetOption;
using QuantLib::Pricers::FiniteDifferenceEuropean;
using QuantLib::Pricers::DividendAmericanOption;
using QuantLib::Pricers::DividendEuropeanOption;
using QuantLib::Pricers::DividendShoutOption;
using QuantLib::Pricers::ShoutOption;
%}

class BSMEuropeanOption {
  public:
	BSMEuropeanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime, 
	  double volatility);
	~BSMEuropeanOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4, 
	  int maxEvaluations = 100) const ;
};


class CliquetOption {
  public:
	CliquetOption(OptionType type, 
                  double underlying, 
                  Rate dividendYield, 
                  Rate riskFreeRate, 
                  const DoubleVector &dates, 
                  double volatility);
	~CliquetOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, 
	                         double accuracy = 1e-4, 
	                         int maxEvaluations = 100) const ;
};

class FiniteDifferenceEuropean {
  public:
	FiniteDifferenceEuropean(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime, 
	  double volatility, int timeSteps = 200, int gridPoints = 800);
	~FiniteDifferenceEuropean();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	Array getGrid() const;
	Array getPrices() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4, 
	  int maxEvaluations = 100) const ;
};

class BinaryOption {
  public:
	BinaryOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime, 
	  double volatility, double cashPayoff = 1);
	~BinaryOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4, 
	  int maxEvaluations = 100) const ;
};

class AmericanOption {
  public:
	AmericanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, int timeSteps = 100, int gridPoints = 100);
    ~AmericanOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4,
                             int maxEvaluations = 100) const ;
};
class ShoutOption {
  public:
	ShoutOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, int timeSteps = 100, int gridPoints = 100);
    ~ShoutOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4,
                             int maxEvaluations = 100) const ;
};


class BermudanOption{
  public:
	BermudanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, const DoubleVector &dates,
	  int timeSteps = 100, int gridPoints = 100);
	~BermudanOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;	
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4,
	  int maxEvaluations = 100) const ;
};

class DividendShoutOption{
  public:
	DividendShoutOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, 
	  const DoubleVector &dividends, 
	  const DoubleVector &exdivdates,
	  int timeSteps = 100, int gridPoints = 100);
	~DividendShoutOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;	
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4,
	  int maxEvaluations = 100) const ;
};

class DividendAmericanOption {
  public:
	DividendAmericanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, 
	  const DoubleVector &dividends, 
	  const DoubleVector &exdivdates,
	  int timeSteps = 100, int gridPoints = 100);
	~DividendAmericanOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;	
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4,
	  int maxEvaluations = 100) const ;
};

class DividendEuropeanOption {
  public:
	DividendEuropeanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, 
	  const DoubleVector &dividends, 
	  const DoubleVector &exdivdates);
	~DividendEuropeanOption();
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4,
	  int maxEvaluations = 100) const ;
};

%include Barrier.i

%{
using QuantLib::Pricers::BarrierOption;
%}


class BarrierOption {
  public:
    BarrierOption(BarrierType barrType, OptionType type, double underlying, 
        double strike, Rate dividendYield, Rate riskFreeRate,
        Time residualTime, double volatility, double barrier, 
        double rebate = 0.0);
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
    ~BarrierOption();
    double value() const;
};


#endif
