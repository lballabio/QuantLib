
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

/*! \file bsmnumericaloption.hpp
    \fullpath Include/ql/Pricers/%bsmnumericaloption.hpp
    \brief common code for numerical option evaluation

    $Id$
*/

// $Source$
// $Log$
// Revision 1.13  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.12  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.11  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.10  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.9  2001/07/09 16:29:27  lballabio
// Some documentation and market element
//
// Revision 1.8  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.7  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef BSM_numerical_option_pricer_h
#define BSM_numerical_option_pricer_h

#include "ql/Pricers/singleassetoption.hpp"
#include "ql/FiniteDifferences/bsmoperator.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Black-Scholes-Merton option priced numerically
        class BSMNumericalOption : public SingleAssetOption {
          public:
            BSMNumericalOption(Type type, double underlying, double strike,
                Rate dividendYield, Rate riskFreeRate, Time residualTime,
                double volatility, int gridPoints);
            // accessors
            virtual void calculate() const = 0;
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            Array getGrid() const{return grid_;}

          protected:
            // methods
            virtual void setGridLimits(double center, double timeDelay) const;
            virtual void initializeGrid() const;
            virtual void initializeInitialCondition() const;
            virtual void initializeOperator() const;
            // input data
            int gridPoints_;
            // results
            mutable double delta_, gamma_, theta_;

            mutable Array grid_;
            mutable FiniteDifferences::BSMOperator finiteDifferenceOperator_;
            mutable Array initialPrices_;
            // temporaries
            mutable double sMin_, center_, sMax_;
          private:
            // temporaries
            mutable double gridLogSpacing_;
            int safeGridPoints(int gridPoints, Time residualTime);
        };

        //! This is a safety check to be sure we have enough grid points.
        #define QL_NUM_OPT_MIN_GRID_POINTS            100
        //! This is a safety check to be sure we have enough grid points.
        #define QL_NUM_OPT_GRID_POINTS_PER_YEAR        50

            // The following is a safety check to be sure we have enough grid
            // points.
        inline int BSMNumericalOption::safeGridPoints(int gridPoints,
                                                        Time residualTime){
            return QL_MAX(gridPoints,
              residualTime>1.0 ? int(QL_NUM_OPT_MIN_GRID_POINTS +
              (residualTime-1.0)*QL_NUM_OPT_GRID_POINTS_PER_YEAR) :
              QL_NUM_OPT_MIN_GRID_POINTS);
        }

    }

}


#endif
