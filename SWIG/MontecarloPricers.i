
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*
    $Id$
    $Source$
    $Log$
    Revision 1.13  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_Montecarlo_Pricers_i
#define quantlib_Montecarlo_Pricers_i

%include QLArray.i
%include Vectors.i
%include Matrix.i

%{
using QuantLib::Pricers::AveragePriceAsian;
using QuantLib::Pricers::AverageStrikeAsian;
using QuantLib::Pricers::EverestOption;
using QuantLib::Pricers::GeometricAsianOption;
using QuantLib::Pricers::Himalaya;
using QuantLib::Pricers::McEuropeanPricer;
using QuantLib::Pricers::PlainBasketOption;
using QuantLib::Pricers::PagodaOption;
%}

class McEuropeanPricer {
  public:
	McEuropeanPricer(OptionType type, double underlying, double strike,
		 Rate dividendYield,   Rate riskFreeRate, double residualTime,
		 double volatility,	int timesteps, int confnumber, long seed);
    ~McEuropeanPricer();
	double value() const;
	double errorEstimate() const;
};

class GeometricAsianOption {
  public:
	GeometricAsianOption(OptionType type, double underlying, double strike,
		Rate dividendYield, Rate exerciseRate,
		double residualTime, double volatility);
    ~GeometricAsianOption();
	double value() const;
};


class AveragePriceAsian {
  public:
	AveragePriceAsian(OptionType type, double underlying, double strike,
		 Rate dividendYield,   Rate riskFreeRate, double residualTime,
		 double volatility,	int timesteps, int confnumber, long seed);
    ~AveragePriceAsian();
	double value() const;
	double errorEstimate() const;
};


class AverageStrikeAsian {
  public:
	AverageStrikeAsian(OptionType type, double underlying, double strike,
		 Rate dividendYield,   Rate riskFreeRate, double residualTime,
		 double volatility,	int timesteps, int confnumber, long seed);
    ~AverageStrikeAsian();
	double value() const;
	double errorEstimate() const;
};


class PlainBasketOption {
  public:
    PlainBasketOption(const Array &underlying,
        const Array &dividendYield, const Matrix &covariance,
        Rate riskFreeRate,  double residualTime,
        int timesteps, long samples, long seed = 0);
    ~PlainBasketOption();
	double value() const;
	double errorEstimate() const;
};


class PagodaOption {
  public:
    PagodaOption(const Array &portfolio,  double fraction,
        double roof, double residualTime, const Matrix &covariance,
        const Array &dividendYield, Rate riskFreeRate,
        int timesteps, long samples, long seed = 0);
    ~PagodaOption();
	double value() const;
	double errorEstimate() const;
};


class Himalaya {
  public:
    Himalaya(const Array& underlying, const Array& dividendYield,
        const Matrix &covariance, Rate riskFreeRate, double strike,
        const DoubleVector &timeDelays, long samples, long seed=0);
    ~Himalaya();
	double value() const;
	double errorEstimate() const;
};


class EverestOption {
  public:
    EverestOption(const Array& dividendYield, const Matrix &covariance,
                  Rate riskFreeRate, Time residualTime,
                  long samples, long seed=0);
    ~EverestOption();
	double value() const;
	double errorEstimate() const;
};


#endif
