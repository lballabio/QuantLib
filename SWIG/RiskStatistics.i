
/*
 * Copyright (C) 2000
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
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/* $Source$
   $Log$
   Revision 1.7  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_risk_statistics_i
#define quantlib_risk_statistics_i

%include Vectors.i

%{
using QuantLib::RiskStatistics;
using QuantLib::Math::VarTool;
%}


class VarTool {
  public:
    VarTool();
    ~VarTool();
    double valueAtRisk(double percentile,
                       double mean,
                       double std) const ;
    double shortfall(double target,
                     double mean,
                     double std) const ;
    double averageShortfall(double target,
                            double mean,
                            double std) const ;
};



class RiskStatistics {
  public:
    RiskStatistics();
    virtual ~RiskStatistics();
    // Accessors
    int samples() const;
    double weightSum() const;
    double mean() const;
    double variance() const;
    double standardDeviation() const;
    double errorEstimate() const;
    double skewness() const;
    double kurtosis() const;
    double min() const;
    double max() const;
    double valueAtRisk(double percentile) const;
    double expectedShortfall(double percentile) const;
    double shortfall(double target) const;
    double averageShortfall(double target) const;
    // Modifiers
    void add(double value, double weight = 1.0);
    void reset();
};

%addmethods RiskStatistics {
	void addSequence(DoubleVector values) {
	  self->addSequence(values.begin(), values.end());
	}
	void addWeightedSequence(DoubleVector values, DoubleVector weights) {
	  self->addSequence(values.begin(), values.end(), weights.begin());
	}
}

#endif
