
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file riskstatistics.hpp
    \brief Normal distribution risk analysis tool: VAR, (average) shortfall

    \fullpath
    ql/%riskstatistics.hpp
*/

// $Id$

#ifndef quantlib_risk_statistics_h
#define quantlib_risk_statistics_h

#include <ql/Math/statistic.hpp>
#include <ql/Math/riskmeasures.hpp>

namespace QuantLib {

    //! Risk analysis tool
    /*! It can accumulate a set of data and return risk quantities
        as Value-At-Risk, Expected Shortfall,
        Shortfall, Average Shortfall, plus statistic
        quantitities as mean, variance, std. deviation, skewness, kurtosis.

        \deprecated use Statistic instead (or even take a look at
                    HStatistics)
    */
    class RiskStatistics {
      public:
        //! \name Inspectors
        //@{
        // Statistic proxy methods
        Size samples() const {return statistic_.samples(); }
        double weightSum() const {return statistic_.weightSum(); }
        double mean() const {return statistic_.mean(); }
        double variance() const {return statistic_.variance(); }
        double standardDeviation() const {
            return statistic_.standardDeviation(); }
        double errorEstimate() const {
            return statistic_.errorEstimate(); }
        double skewness() const {return statistic_.skewness(); }
        double kurtosis() const {return statistic_.kurtosis(); }
        double min() const {return statistic_.min(); }
        double max() const {return statistic_.max(); }

        // RiskMeasures proxy methods
        //! returns the Potential-Upside at a given percentile
        double potentialUpside(double percentile) const {
            return riskMeasures_.potentialUpside(percentile,
                statistic_.mean(), statistic_.standardDeviation());
        }
        //! returns the Value-At-Risk at a given percentile
        double valueAtRisk(double percentile) const {
            return riskMeasures_.valueAtRisk(percentile,
                statistic_.mean(), statistic_.standardDeviation());
        }
        //! returns the Expected Shortfall at a given percentile
        double expectedShortfall(double percentile) const {
            return riskMeasures_.expectedShortfall(percentile,
                statistic_.mean(), statistic_.standardDeviation());
        }
        //! returns the Shortfall (observations below target)
        double shortfall(double target) const {
            return riskMeasures_.shortfall(target,
                statistic_.mean(), statistic_.standardDeviation());
        }
        //! returns the Average Shortfall (averaged shortfallness)
        double averageShortfall(double target) const  {
            return riskMeasures_.averageShortfall(target,
                statistic_.mean(), statistic_.standardDeviation());
        }
        //@}
        //! \name Modifiers
        //@{
        void add(double value, double weight = 1.0);
        template <class DataIterator>
        void addSequence(DataIterator begin, DataIterator end) {
          for (;begin!=end;++begin)
            add(*begin);
        }
        //! adds a sequence of data to the set, each with its weight
        template <class DataIterator, class WeightIterator>
        void addSequence(DataIterator begin, DataIterator end,
          WeightIterator wbegin) {
            for (;begin!=end;++begin,++wbegin)
                add(*begin, *wbegin);
        }
        void reset();

        //@}
      private:
        Math::Statistic statistic_;
        Math::RiskMeasures riskMeasures_;
    };

    // inline definitions

    /*! \pre weights must be positive or null */
    inline void RiskStatistics::add(double value, double weight) {
        statistic_.add( value , weight );
    }

    inline void RiskStatistics::reset() {
        statistic_.reset();
    }

}


#endif
