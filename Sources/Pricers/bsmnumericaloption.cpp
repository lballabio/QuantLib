
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
    Revision 1.27  2001/02/19 12:19:29  marmar
    Added trailing _ to protected and private members

    Revision 1.26  2001/02/15 15:57:41  marmar
    Defined QL_MIN_VOLATILITY 0.0005 and
    QL_MAX_VOLATILITY 3.0

    Revision 1.25  2001/02/15 15:30:30  marmar
    dVolMultiplier and dRMultiplier defined
    constant

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
        
        const double BSMNumericalOption::dVolMultiplier = 0.0001; 
        const double BSMNumericalOption::dRMultiplier = 0.0001; 
        
        BSMNumericalOption::BSMNumericalOption(BSMNumericalOption::Type type,
            double underlying, double strike, Rate dividendYield, 
            Rate riskFreeRate, Time residualTime, double volatility, 
            int gridPoints)
        : BSMOption(type, underlying, strike, dividendYield, riskFreeRate,
            residualTime, volatility), rhoComputed(false), vegaComputed(false),
            theGridPoints(safeGridPoints(gridPoints, residualTime)),
            theGrid(theGridPoints), theInitialPrices(theGridPoints){
                hasBeenCalculated_ = false;
        }
        
        double BSMNumericalOption::delta() const {
            if (!hasBeenCalculated_)  
                value();
            return theDelta;
        }
        
        double BSMNumericalOption::gamma() const {
            if(!hasBeenCalculated_) 
                value();
            return theGamma;
        }
        
        double BSMNumericalOption::theta() const {
            if(!hasBeenCalculated_) 
                value();
            return theTheta;
        }

        double BSMNumericalOption::vega() const {
        
            if(!vegaComputed){
                if(!hasBeenCalculated_) 
                    value();
                Handle<BSMOption> brandNewFD = clone();
                double volMinus = volatility_ * (1.0 - dVolMultiplier);
                brandNewFD -> setVolatility(volMinus);        
                theVega = (value() - brandNewFD -> value()) / 
                    (volatility_ * dVolMultiplier);          
                vegaComputed = true;
            }
            return theVega;
        }
        
        double BSMNumericalOption::rho() const {
        
            if(!rhoComputed){
                if(!hasBeenCalculated_) 
                    value();
                Handle<BSMOption> brandNewFD = clone();
                Rate rMinus=riskFreeRate_ * (1.0 - dRMultiplier);        
                brandNewFD -> setRiskFreeRate(rMinus);
                theRho=(value() - brandNewFD -> value()) / 
                    (riskFreeRate_ * dRMultiplier);
                rhoComputed  = true;
            }
            return theRho;
        }
        
        void BSMNumericalOption::setGridLimits() const {
            // correction for small volatilities
            double prefactor = 1.0 + 0.05/volatility_;
            double minMaxFactor = 
                QL_EXP(4.0*prefactor*volatility_*QL_SQRT(residualTime_));
            sMin = underlying_/minMaxFactor;  // underlying grid min value
            sMax = underlying_*minMaxFactor;  // underlying grid max value
            // insure strike is included in the grid
            double safetyZoneFactor = 1.1;                    
            if(sMin > strike_/safetyZoneFactor){
                sMin = strike_/safetyZoneFactor;
                // enforce central placement of the underlying
                sMax = underlying_/(sMin/underlying_);    
            }
            if(sMax < strike_*safetyZoneFactor){
                sMax = strike_*safetyZoneFactor;
                // enforce central placement of the underlying
                sMin = underlying_/(sMax/underlying_);    
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
            switch (type_) {
              case Call:
                for(j=0; j<theGridPoints; j++)
                    theInitialPrices[j] = QL_MAX(theGrid[j]-strike_,0.0);
                break;
              case Put:
                for(j=0; j<theGridPoints; j++)
                    theInitialPrices[j] = QL_MAX(strike_-theGrid[j],0.0);
                break;
              case Straddle:
                for(j=0; j<theGridPoints; j++)
                    theInitialPrices[j] = QL_FABS(strike_-theGrid[j]);
                break;
              default:
                throw IllegalArgumentError(
                    "BSMNumericalOption: invalid option type");  
            }
        }
        
        void BSMNumericalOption::initializeOperator() const {
            theOperator = BSMOperator(theGridPoints, theGridLogSpacing, 
                riskFreeRate_, dividendYield_, volatility_);
                
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
