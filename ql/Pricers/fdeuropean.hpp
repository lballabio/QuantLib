
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
        class FdEuropean : public FdBsmOption {
          public:
            FdEuropean(Option::Type type,
                                     double underlying,
                                     double strike,
                                     Spread dividendYield,
                                     Rate riskFreeRate,
                                     Time residualTime,
                                     double volatility,
                                     Size timeSteps = 200,
                                     Size gridPoints = 800);
                   Array getPrices() const;
            Handle<SingleAssetOption> clone() const{
                return Handle<SingleAssetOption>(
                    new FdEuropean(*this));
            }
          protected:
            void calculate() const;
          private:
            Size timeSteps_;
            mutable Array euroPrices_;
        };

        // inline definitions

        inline Array  FdEuropean::getPrices() const{
            value();
            return euroPrices_;
        }

    }

}

#endif
