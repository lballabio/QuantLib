
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

/*! \file risktool.h
    \brief Normal distribution risk analysis tool: VAR, (Average) Shortfall
    
    $Source$
    $Log$
    Revision 1.2  2001/01/15 12:41:14  aleppo
    bug fixed: shortfall(s) were not normalized

    Revision 1.1  2001/01/12 17:30:29  nando
    added RiskTool.
    It offres VAR, shortfall, average shortfall methods.
    It still needs test

*/

#ifndef quantlib_risktool_h
#define quantlib_risktool_h

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"
#include "dataformatters.h"
#include "statistics.h"
#include "normaldistribution.h"
#include <iostream>

namespace QuantLib {

    namespace RiskTool {
        //! Risk analysis tool
        /*! It can accumulate a set of data and return risk quantities
            as Value-At-Risk, Shortfall, Average Shortfall, plus statistic
            quantitities as mean, variance, std. deviation, skewness, kurtosis.
        */
        class RiskTool : public Math::Statistics {
          public:
            RiskTool(double target = Null<double>());
            //! \name Inspectors
            //@{
            //! returns the Value-At-Risk at a given percentile
            double valueAtRisk(double percentile) const;
            //! returns the Shortfall (observations below target)
            double shortfall() const {
                return shortfallCounter_/weightSum();
                }
            //! returns the Average Shortfall (averaged shortfallness)
            double averageShortfall() const {
                return averageShortfall_/weightSum();
                }
            //@}
            
            //! \name Modifiers
            //@{
            //! adds a datum to the set, possibly with a weight
            void add(double value, double weight = 1.0);
            //! adds a sequence of data to the set
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
            //! resets the data to a null set
            void reset(double target);
            //! resets the data to a null set
            void reset();
            //@}
          private:
            double target_, shortfallCounter_, averageShortfall_;
        };
    
        // inline definitions
        
        /*! \pre weights must be positive or null */
        inline void RiskTool::add(double value, double weight) {
            Math::Statistics::add(value, weight);
    
            if (!IsNull(target_) && value<target_) {
                shortfallCounter_ += weight;
                averageShortfall_ += weight * (target_-value);
            }
            
        }
    
        /*! \pre percentile must be in range 90%-100% */
        inline double RiskTool::valueAtRisk(double percentile) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "RiskTool::valueAtRisk : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");
    
            Math::InvCumulativeNormalDistribution dist(mean(),
                                             standardDeviation());
            // VAR must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(dist(1.0-percentile), 0.0);
        }
    }
}


#endif
