
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
    Revision 1.4  2001/01/16 11:30:08  nando
    restoring risktool.h

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
            RiskTool() {}
            //! \name Inspectors
            //@{
            //! returns the Value-At-Risk at a given percentile
            double valueAtRisk(double percentile) const;
            //! returns the Shortfall (observations below target)
            double shortfall(double target) const;
            //! returns the Average Shortfall (averaged shortfallness)
            double averageShortfall(double target) const;
            //@}
        };

        // inline definitions
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

        inline double RiskTool::shortfall(double target) const {
            Math::CumulativeNormalDistribution gI(mean(), standardDeviation());
            return gI(target);
        }

        double RiskTool::averageShortfall(double target) const {
            double m = mean();
            double s = standardDeviation();
            Math::CumulativeNormalDistribution gI(m, s);
            Math::NormalDistribution g(m, s);
            return ( (target - m) * gI(target) + s * s * g(target) );
        }


    }
}


#endif
