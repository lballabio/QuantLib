
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

/*! \file bsmnumericaloption.h
    \brief common code for numerical option evaluation

    $Source$
    $Name$
    $Log$
    Revision 1.12  2001/02/15 15:29:10  marmar
    dVolMultiplier and dRMultiplier defined
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

#include "qldefines.h"
#include "bsmoption.h"
#include "bsmoperator.h"
#include "array.h"
#include "handle.h"

namespace QuantLib {

    namespace Pricers {

        class BSMNumericalOption : public BSMOption {
          public:
                BSMNumericalOption(Type type, double underlying, double strike, 
                    Rate dividendYield, Rate riskFreeRate, Time residualTime, 
                    double volatility, int gridPoints);
                // accessors
                double delta() const;
                double gamma() const;
                double theta() const;
                double rho()   const;
                double vega()  const;

          protected:
            // methods
            double valueAtCenter(const Array& a) const;
            double firstDerivativeAtCenter(const Array& a, 
                    const Array& g) const;
            double secondDerivativeAtCenter(const Array& a, 
                    const Array& g) const;
            void setGridLimits() const;
            void initializeGrid() const;
            void initializeInitialCondition() const;
            void initializeOperator() const;
            // input data
            int theGridPoints;
            // results
            mutable bool rhoComputed, vegaComputed;
            mutable double theDelta, theGamma, theTheta;
            mutable double  theRho, theVega;

            mutable Array theGrid;
            mutable FiniteDifferences::BSMOperator theOperator;
            mutable Array theInitialPrices;
            // temporaries
            mutable double sMin, sMax;
          private:
            // temporaries
            mutable double theGridLogSpacing;
            const static double dVolMultiplier, dRMultiplier;
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
