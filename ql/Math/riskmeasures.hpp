
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

/*! \file riskmeasures.hpp

    \fullpath
    Include/ql/Math/%riskmeasures.hpp
    \brief Risk functions

*/

// $Id$
//  $Log$
//  Revision 1.1  2001/09/03 13:57:12  nando
//  source (*.hpp and *.cpp) moved under topdir/ql
//
//  Revision 1.15  2001/08/31 15:23:45  sigmud
//  refining fullpath entries for doxygen documentation
//
//  Revision 1.14  2001/08/09 14:59:46  sigmud
//  header modification
//
//  Revision 1.13  2001/08/08 11:07:48  sigmud
//  inserting \fullpath for doxygen
//
//  Revision 1.12  2001/08/07 11:25:54  sigmud
//  copyright header maintenance
//
//  Revision 1.11  2001/07/25 15:47:28  sigmud
//  Change from quantlib.sourceforge.net to quantlib.org
//
//  Revision 1.10  2001/07/02 12:36:18  sigmud
//  pruned redundant header inclusions
//
//  Revision 1.9  2001/06/22 16:38:15  lballabio
//  Improved documentation
//

#ifndef quantlib_risk_measures_h
#define quantlib_risk_measures_h

#include "ql/null.hpp"
#include "ql/Math/normaldistribution.hpp"
#include <iostream>
#include <vector>


namespace QuantLib {

    namespace Math {

        //! Interface for risk functions
        class RiskMeasures {
          public:
            RiskMeasures() {}
            double potentialUpside(double percentile,
                                    double mean,
                                    double std) const ;
            double valueAtRisk(double percentile,
                               double mean,
                               double std) const ;
            double expectedShortfall(double percentile,
                                     double mean,
                                     double std) const ;
            double shortfall(double target,
                             double mean,
                             double std) const ;
            double averageShortfall(double target,
                                    double mean,
                                    double std) const ;
       };

        // inline definitions
        /*! \pre percentile must be in range 90%-100% */
        inline double RiskMeasures::potentialUpside(double percentile,
                                                     double mean,
                                                     double std) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "RiskMeasures::potentialUpside : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            Math::InvCumulativeNormalDistribution gInverse(mean, std);
            // PotenzialUpSide must be a gain
            // this means that it has to be MAX(dist(percentile), 0.0)
            return QL_MAX(gInverse(percentile), 0.0);
        }

        /*! \pre percentile must be in range 90%-100% */
        inline double RiskMeasures::valueAtRisk(double percentile,
                                           double mean,
                                           double std) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "RiskMeasures::valueAtRisk : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            Math::InvCumulativeNormalDistribution gInverse(mean, std);
            // VAR must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(gInverse(1.0-percentile), 0.0);
        }

        /*! \pre percentile must be in range 90%-100% */
        inline double RiskMeasures::expectedShortfall(double percentile,
                                                 double mean,
                                                 double std) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "RiskMeasures::expectedShortfall : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            Math::InvCumulativeNormalDistribution gInverse(mean, std);
            double var = gInverse(1.0-percentile);
            Math::NormalDistribution g(mean, std);
            double result = mean - std*std*g(var)/(1.0-percentile);
            // expectedShortfall must be a loss
            // this means that it has to be MIN(result, 0.0)
            // expectedShortfall must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }

        inline double RiskMeasures::shortfall(double target,
                                         double mean,
                                         double std) const {
            Math::CumulativeNormalDistribution gIntegral(mean, std);
            return gIntegral(target);
        }

        inline double RiskMeasures::averageShortfall(double target,
                                                double mean,
                                                double std) const {
            Math::CumulativeNormalDistribution gIntegral(mean, std);
            Math::NormalDistribution g(mean, std);
            return ( (target-mean)*gIntegral(target) + std*std*g(target) );
        }

    }

}


#endif
