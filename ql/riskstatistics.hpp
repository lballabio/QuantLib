
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file riskstatistics.hpp

    \fullpath
    Include/ql/%riskstatistics.hpp
    \brief Normal distribution risk analysis tool: VAR, (average) shortfall

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:54:20  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.14  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.13  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.12  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.11  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.10  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.9  2001/07/04 12:00:37  uid40428
// Array of random numbers built with an array of dates
//
// Revision 1.8  2001/07/02 12:36:17  sigmud
// pruned redundant header inclusions
//
// Revision 1.7  2001/06/11 16:00:23  nando
// potentialUpFront is now potentialUpSide.
// Few typos fixed
//
// Revision 1.6  2001/06/11 13:51:16  aleppo
// Potential  Up Front added
//
// Revision 1.5  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.4  2001/05/28 13:09:55  nando
// R019-branch-merge3 merged into trunk
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_riskstatistics_h
#define quantlib_riskstatistics_h

#include "ql/Math/statistics.hpp"
#include "ql/Math/riskmeasures.hpp"

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

        // RiskMeasures proxy methods
        //! returns the Potential-Up-Front at a given percentile
        double potentialUpside(double percentile) const {
            return riskMeasures_.potentialUpside(percentile,
                                               statistics_.mean(),
                                               statistics_.standardDeviation());
        }
        //! returns the Value-At-Risk at a given percentile
        double valueAtRisk(double percentile) const {
            return riskMeasures_.valueAtRisk(percentile,
                                        statistics_.mean(),
                                        statistics_.standardDeviation());
        }
        //! returns the Expected Shortfall at a given percentile
        double expectedShortfall(double percentile) const {
            return riskMeasures_.expectedShortfall(percentile,
                                              statistics_.mean(),
                                              statistics_.standardDeviation());
        }
        //! returns the Shortfall (observations below target)
        double shortfall( double target ) const {
            return riskMeasures_.shortfall(target,
                                      statistics_.mean(),
                                      statistics_.standardDeviation());
        }
        //! returns the Average Shortfall (averaged shortfallness)
        double averageShortfall( double target ) const  {
            return riskMeasures_.averageShortfall(
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
        Math::RiskMeasures riskMeasures_;
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
