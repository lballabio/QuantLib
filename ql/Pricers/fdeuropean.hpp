
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

/*! \file finitedifferenceeuropean.hpp
    \brief Example of European option calculated using finite differences

    \fullpath
    ql/Pricers/%finitedifferenceeuropean.hpp
*/

// $Id$

#ifndef quantlib_pricers_finite_difference_european_option_h
#define quantlib_pricers_finite_difference_european_option_h

#include <ql/Pricers/fdbsmoption.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Example of European option calculated using finite differences
        class FiniteDifferenceEuropean : public FdBsmOption {
          public:
            FiniteDifferenceEuropean(Option::Type type,
                                     double underlying,
                                     double strike,
                                     Spread dividendYield,
                                     Rate riskFreeRate,
                                     Time residualTime,
                                     double volatility,
                                     size_t timeSteps = 200,
                                     size_t gridPoints = 800);
                   Array getPrices() const;
            Handle<SingleAssetOption> clone() const{
                return Handle<SingleAssetOption>(
                    new FiniteDifferenceEuropean(*this));
            }
          protected:
            void calculate() const;
          private:
            size_t timeSteps_;
            mutable Array euroPrices_;
        };

        // inline definitions

        inline Array  FiniteDifferenceEuropean::getPrices() const{
            value();
            return euroPrices_;
        }

    }

}

#endif
