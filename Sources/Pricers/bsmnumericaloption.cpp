
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
 * QuantLib license is also available at 
 *  http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file bsmnumericaloption.cpp
    \brief common code for numerical option evaluation
    
    $Source$
    $Name$
    $Log$
    Revision 1.24  2001/02/14 10:11:25  marmar
    BSMNumericalOption has  a cleaner constructor

    Revision 1.23  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.22  2001/01/19 08:52:07  marmar
    Small bug fixed. Initialization of grid vectors is now after
     the determination of their size

    Revision 1.21  2001/01/08 11:44:18  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.20  2001/01/08 10:28:17  lballabio
    Moved Array to Math namespace

    Revision 1.19  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros
    
    Revision 1.18  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks
    
*/

#include "bsmnumericaloption.h"

namespace QuantLib {

    namespace Pricers {
    
        using FiniteDifferences::BoundaryCondition;
        using FiniteDifferences::BSMOperator;
        
        double BSMNumericalOption::dVolMultiplier = 0.0001; 
        double BSMNumericalOption::dRMultiplier = 0.0001; 
        
        BSMNumericalOption::BSMNumericalOption(BSMNumericalOption::Type type,
            double underlying, double strike, Rate dividendYield, 
            Rate riskFreeRate, Time residualTime, double volatility, 
            int gridPoints)
        : BSMOption(type, underlying, strike, dividendYield, riskFreeRate,
            residualTime, volatility), rhoComputed(false), vegaComputed(false),
            theGridPoints(safeGridPoints(gridPoints, residualTime)),
            theGrid(theGridPoints), theInitialPrices(theGridPoints){
                hasBeenCalculated = false;
        }
        
        double BSMNumericalOption::delta() const {
            if (!hasBeenCalculated)  
                value();
            return theDelta;
        }
        
        double BSMNumericalOption::gamma() const {
            if(!hasBeenCalculated) 
                value();
            return theGamma;
        }
        
        double BSMNumericalOption::theta() const {
            if(!hasBeenCalculated) 
                value();
            return theTheta;
        }

        double BSMNumericalOption::vega() const {
        
            if(!vegaComputed){
                if(!hasBeenCalculated) 
                    value();
                Handle<BSMOption> brandNewFD = clone();
                double volMinus=theVolatility*(1.0-dVolMultiplier);
                brandNewFD->setVolatility(volMinus);        
                theVega=(value() - brandNewFD->value()) / 
                    (theVolatility*dVolMultiplier);          
                vegaComputed = true;
            }
            return theVega;
        }
        
        double BSMNumericalOption::rho() const {
        
            if(!rhoComputed){
                if(!hasBeenCalculated) 
                    value();
                Handle<BSMOption> brandNewFD = clone();
                Rate rMinus=theRiskFreeRate*(1.0-dRMultiplier);        
                brandNewFD->setRiskFreeRate(rMinus);
                theRho=(value() - brandNewFD->value()) / 
                    (theRiskFreeRate*dRMultiplier);
                rhoComputed  = true;
            }
            return theRho;
        }
        
        void BSMNumericalOption::setGridLimits() const {
            // correction for small volatilities
            double prefactor = 1.0+0.05/theVolatility;
            double minMaxFactor = 
                QL_EXP(4.0*prefactor*theVolatility*QL_SQRT(theResidualTime));
            sMin = theUnderlying/minMaxFactor;  // underlying grid min value
            sMax = theUnderlying*minMaxFactor;  // underlying grid max value
            // insure strike is included in the grid
            double safetyZoneFactor = 1.1;                    
            if(sMin > theStrike/safetyZoneFactor){
                sMin = theStrike/safetyZoneFactor;
                // enforce central placement of the underlying
                sMax = theUnderlying/(sMin/theUnderlying);    
            }
            if(sMax < theStrike*safetyZoneFactor){
                sMax = theStrike*safetyZoneFactor;
                // enforce central placement of the underlying
                sMin = theUnderlying/(sMax/theUnderlying);    
            }
        }

        void BSMNumericalOption::initializeGrid() const {            
            theGridLogSpacing = (QL_LOG(sMax)-QL_LOG(sMin))/(theGridPoints-1);
            double edx = QL_EXP(theGridLogSpacing);
            theGrid[0] = sMin;
            int j;
            for (j=1; j<theGridPoints; j++)
                theGrid[j] = theGrid[j-1]*edx;
        }
        
        void BSMNumericalOption::initializeInitialCondition() const {
            int j;
            switch (theType) {
              case Call:
                for(j=0; j<theGridPoints; j++)
                    theInitialPrices[j] = QL_MAX(theGrid[j]-theStrike,0.0);
                break;
              case Put:
                for(j=0; j<theGridPoints; j++)
                    theInitialPrices[j] = QL_MAX(theStrike-theGrid[j],0.0);
                break;
              case Straddle:
                for(j=0; j<theGridPoints; j++)
                    theInitialPrices[j] = QL_FABS(theStrike-theGrid[j]);
                break;
              default:
                throw IllegalArgumentError(
                    "BSMNumericalOption: invalid option type");  
            }
        }
        
        void BSMNumericalOption::initializeOperator() const {
            theOperator = BSMOperator(theGridPoints, theGridLogSpacing, 
                theRiskFreeRate, dividendYield_, theVolatility);
            theOperator.setLowerBC(
                BoundaryCondition(BoundaryCondition::Neumann,
                    theInitialPrices[1]-theInitialPrices[0]));
            theOperator.setHigherBC(
                BoundaryCondition(BoundaryCondition::Neumann,
                    theInitialPrices[theGridPoints-1] - 
                        theInitialPrices[theGridPoints-2]));
        }
        
        // Useful functions
        
        double BSMNumericalOption::valueAtCenter(const Array& a) const {
            int jmid = a.size()/2;
            if (a.size() % 2 == 1)
                return a[jmid];
            else 
                return (a[jmid]+a[jmid-1])/2.0;
        }
            
        double BSMNumericalOption::firstDerivativeAtCenter(const Array& a, 
            const Array& g) const {
            QL_REQUIRE(a.size()==g.size(),
                "BSMNumericalOption::firstDerivativeAtCenter: "
                "a and g must be of the same size");
            QL_REQUIRE(a.size()>=3,
                "BSMNumericalOption::firstDerivativeAtCenter: "
                "the size of the two vectors must be at least 3");
            int jmid = a.size()/2;
            if(a.size() % 2 == 1)
                return (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
            else
                return (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
        }
        
        double BSMNumericalOption::secondDerivativeAtCenter(
            const Array& a, const Array& g) const {
            QL_REQUIRE(a.size()==g.size(),
                "BSMNumericalOption::secondDerivativeAtCenter: "
                "a and g must be of the same size");
            QL_REQUIRE(a.size()>=4,
                "BSMNumericalOption::secondDerivativeAtCenter: "
                "the size of the two vectors must be at least 4");
            int jmid = a.size()/2;
            if(a.size() % 2 == 1){
                double deltaPlus = (a[jmid+1]-a[jmid])/(g[jmid+1]-g[jmid]);
                double deltaMinus = (a[jmid]-a[jmid-1])/(g[jmid]-g[jmid-1]);
                double dS = (g[jmid+1]-g[jmid-1])/2.0;
                return (deltaPlus-deltaMinus)/dS;    
            }          
            else{
                double deltaPlus = (a[jmid+1]-a[jmid-1])/(g[jmid+1]-g[jmid-1]);
                double deltaMinus = (a[jmid]-a[jmid-2])/(g[jmid]-g[jmid-2]);
                return (deltaPlus-deltaMinus)/(g[jmid]-g[jmid-1]);    
            }
        }

    }

}
