
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

#ifndef quantlib_pricers_i
#define quantlib_pricers_i

%module Pricers

%{
#include "quantlib.h"
/* #include "binaryoption.h" */
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: Pricers is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%include Date.i
%include Options.i
%include Financial.i
%include Vectors.i

%{
using QuantLib::Pricers::BSMEuropeanOption;
using QuantLib::Pricers::AmericanOption;
using QuantLib::Pricers::ShoutOption;
using QuantLib::Pricers::DividendAmericanOption;
using QuantLib::Pricers::DividendEuropeanOption;
using QuantLib::Pricers::BinaryOption;
%}

class BSMEuropeanOption {
  public:
	BSMEuropeanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime, 
	  double volatility);
	~BSMEuropeanOption();
	void setVolatility(double newVolatility) ;
	void setRiskFreeRate(Rate newRate) ;
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4, 
	  int maxEvaluations = 100) const ;
};

class BinaryOption {
  public:
	BinaryOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime, 
	  double volatility, double cashPayoff = 1);
	~BinaryOption();
	void setVolatility(double newVolatility) ;
	void setRiskFreeRate(Rate newRate) ;
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
	void setVolatility(double newVolatility) ;
	void setRiskFreeRate(Rate newRate) ;
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
	void setVolatility(double newVolatility) ;
	void setRiskFreeRate(Rate newRate) ;
	double value() const;
	double delta() const;
	double gamma() const;
	double theta() const;
	double vega() const;
	double rho() const;
	double impliedVolatility(double targetValue, double accuracy = 1e-4,
                             int maxEvaluations = 100) const ;
};

class DividendAmericanOption{
  public:
	DividendAmericanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, DoubleVector dividends, DoubleVector exdivdates,
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

class DividendEuropeanOption{
  public:
	DividendEuropeanOption(OptionType type, double underlying, double strike, 
	  Rate dividendYield, Rate riskFreeRate, Time residualTime,
	  double volatility, DoubleVector dividends, DoubleVector exdivdates);
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


class BarrierOption{
    public:
        BarrierOption(BarrierType barrType, OptionType type, double underlying, 
            double strike, Rate dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility, double barrier, 
            double rebate = 0.0);
        ~BarrierOption();
        double value() const;
};


#endif
