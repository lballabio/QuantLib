
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

/*! \file discretegeometricaso.hpp
    \brief Discrete Geometric Average Strike Option

    \fullpath
    ql/Pricers/%discretegeometricaso.hpp
*/

// $Id$

#ifndef quantlib_discrete_geometric_average_strike_option_h
#define quantlib_discrete_geometric_average_strike_option_h

#include "ql/Pricers/singleassetoption.hpp"
#include "ql/Math/normaldistribution.hpp"
#include <vector>


namespace QuantLib {

    namespace Pricers {

        //! Discrete Geometric Average Strike Asian Option (European style)
        /*! This class implements a discrete geometric average strike asian
            option, with european exercise.
            The formula is from "Asian Option", E. Levy (1997)
            in "Exotic Options: The State of the Art",
            edited by L. Clewlow, C. Strickland, pag65-97

            \todo analytical greeks for Discrete Geometric Average Strike
            Asian Option
        */
        class DiscreteGeometricASO : public SingleAssetOption    {
           public:
            DiscreteGeometricASO(Option::Type type,
                                 double underlying,
                                 Spread dividendYield,
                                 Rate riskFreeRate,
                                 const std::vector<Time>& times,
                                 double volatility);
            double value() const;
            double delta() const {return 0.0;}
            double gamma() const {return 0.0;}
            double theta() const {return 0.0;}
            Handle<SingleAssetOption> clone() const;
           private:
            static const Math::CumulativeNormalDistribution f_;
            std::vector<Time> times_;
        };


        // inline definitions
        inline Handle<SingleAssetOption> DiscreteGeometricASO::clone()
          const {
            return Handle<SingleAssetOption>(
                new DiscreteGeometricASO(*this));
        }

    }

}


#endif
