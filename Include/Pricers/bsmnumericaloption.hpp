
/*
 * Copyright (C) 2000, 2001
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file bsmnumericaloption.hpp
    \brief common code for numerical option evaluation

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.16  2001/03/21 11:33:02  marmar
    Main loop transfered from method value to method calculate.
    Methods vega and rho moved from BSMNumericalOption to BSMOption

    Revision 1.15  2001/03/21 10:49:27  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions

    Revision 1.14  2001/03/12 12:59:53  marmar
    Public method getGrid added

    Revision 1.13  2001/03/01 14:20:51  marmar
    Private-member syntax changed

    Revision 1.12  2001/02/15 15:29:10  marmar
    dVolMultiplier_ and dRMultiplier_ defined
    constant

    Revision 1.11  2001/02/14 10:11:05  marmar
    BSMNumericalOption has  a cleaner constructor

    Revision 1.10  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.9  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.8  2001/01/08 11:44:17  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.7  2001/01/08 10:28:16  lballabio
    Moved Array to Math namespace

    Revision 1.6  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef BSM_numerical_option_pricer_h
#define BSM_numerical_option_pricer_h

#include "qldefines.hpp"
#include "bsmoption.hpp"
#include "FiniteDifferences/bsmoperator.hpp"
#include "array.hpp"
#include "handle.hpp"

namespace QuantLib {

    namespace Pricers {

        class BSMNumericalOption : public BSMOption {
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
            virtual void setGridLimits() const;
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
            mutable double sMin_, sMax_;
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
              residualTime>1.0 ? (int)(QL_NUM_OPT_MIN_GRID_POINTS + 
              (residualTime-1.0)*QL_NUM_OPT_GRID_POINTS_PER_YEAR) :
              QL_NUM_OPT_MIN_GRID_POINTS);
        }
        
    }

}


#endif
