
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

/*! \file riskstatistics.h
    \brief Normal distribution risk analysis tool: VAR, (Average) Shortfall

    $Source$
    $Log$
    Revision 1.2  2001/02/20 13:58:59  nando
    added class VarTool.
    RiskStatistics was derived from Statistics:
    now RiskStatistics includes VarTool and Statistics.
    VarTool is not based on Statistics, but requires
    mean and standardDeviation as input.

    Revision 1.1  2001/01/19 09:33:57  nando
    RiskTool is now RiskStatistics everywhere

    Revision 1.4  2001/01/16 11:30:08  nando
    restoring riskstatistics.h

    Revision 1.2  2001/01/15 12:41:14  aleppo
    bug fixed: shortfall(s) were not normalized

    Revision 1.1  2001/01/12 17:30:29  nando
    added RiskStatistics.
    It offres VAR, shortfall, average shortfall methods.
    It still needs test

*/

#ifndef quantlib_riskstatistics_h
#define quantlib_riskstatistics_h

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"
#include "dataformatters.h"
#include "statistics.h"
#include "vartool.h"
#include <iostream>

namespace QuantLib {

    //! Risk analysis tool
    /*! It can accumulate a set of data and return risk quantities
        as Value-At-Risk, Shortfall, Average Shortfall, plus statistic
        quantitities as mean, variance, std. deviation, skewness, kurtosis.
    */
    class RiskStatistics {
      public:
        RiskStatistics() ;
        //! \name Inspectors
        //@{
        // Statistics proxy methods
        double samples() const {return statistics_.samples(); }
        double weightSum() const {return statistics_.weightSum(); }
        double mean() const {return statistics_.mean(); }
        double variance() const {return statistics_.variance(); }
        double standardDeviation() const {
            return statistics_.standardDeviation(); }
        double errorEstimate() const {
            return statistics_.errorEstimate(); }
        double skewness() const {return statistics_.skewness(); }
        double kurtosis() const {return statistics_.kurtosis(); }
        double min() const {return statistics_.min(); }
        double max() const {return statistics_.max(); }

        // VarTool proxy methods
        //! returns the Value-At-Risk at a given percentile
        double valueAtRisk(double percentile)  {
            return varTool_.valueAtRisk(percentile,
                                        statistics_.mean(),
                                        statistics_.standardDeviation());
        }
        //! returns the Shortfall (observations below target)
        double shortfall( double target ) const {
            return varTool_.shortfall(target,
                                      statistics_.mean(),
                                      statistics_.standardDeviation());
        }
        //! returns the Average Shortfall (averaged shortfallness)
        double averageShortfall( double target ) const  {
            return varTool_.averageShortfall(
                                         target,
                                         statistics_.mean(),
                                         statistics_.standardDeviation());
        }
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
        QuantLib::Math::Statistics statistics_;
        QuantLib::Math::VarTool varTool_;
    };

    // inline definitions
    inline RiskStatistics::RiskStatistics() {
    statistics_  = QuantLib::Math::Statistics();
    varTool_     = QuantLib::Math::VarTool();
    }

    /*! \pre weights must be positive or null */
    inline void RiskStatistics::add(double value, double weight) {
        statistics_.add( value , weight );
    }

    inline void RiskStatistics::reset() {
        statistics_.reset();
    }

}


#endif
