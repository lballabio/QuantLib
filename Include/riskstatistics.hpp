
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
    Revision 1.3  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file riskstatistics.hpp
    \brief Normal distribution risk analysis tool: VAR, (Average) Shortfall
*/

#ifndef quantlib_riskstatistics_h
#define quantlib_riskstatistics_h

#include "qldefines.hpp"
#include "null.hpp"
#include "qlerrors.hpp"
#include "dataformatters.hpp"
#include "Math/statistics.hpp"
#include "Math/vartool.hpp"
#include <iostream>

namespace QuantLib {

    //! Risk analysis tool
    /*! It can accumulate a set of data and return risk quantities
        as Value-At-Risk, Expected Shortfall,
        Shortfall, Average Shortfall, plus statistic
        quantitities as mean, variance, std. deviation, skewness, kurtosis.
    */
    class RiskStatistics {
      public:
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
        double valueAtRisk(double percentile) const {
            return varTool_.valueAtRisk(percentile,
                                        statistics_.mean(),
                                        statistics_.standardDeviation());
        }
        //! returns the Expected Shortfall at a given percentile
        double expectedShortfall(double percentile) const {
            return varTool_.expectedShortfall(percentile,
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
        Math::Statistics statistics_;
        Math::VarTool varTool_;
    };

    // inline definitions

    /*! \pre weights must be positive or null */
    inline void RiskStatistics::add(double value, double weight) {
        statistics_.add( value , weight );
    }

    inline void RiskStatistics::reset() {
        statistics_.reset();
    }

}


#endif
