
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
    Revision 1.33  2001/03/21 10:48:08  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions

    Revision 1.32  2001/03/12 13:11:08  marmar
    Public method getPrices added

    Revision 1.31  2001/03/07 17:14:57  marmar
    Grid limits are fine tuned

    Revision 1.30  2001/03/02 13:50:01  marmar
    Purely virtual method initializeStepCondition()
    introduced in the design of StepConditionOption

    Revision 1.29  2001/03/02 08:36:45  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.

    Revision 1.28  2001/03/01 14:20:36  marmar
    Private-member syntax changed

    Revision 1.27  2001/02/19 12:19:29  marmar
    Added trailing _ to protected and private members

    Revision 1.26  2001/02/15 15:57:41  marmar
    Defined QL_MIN_VOLATILITY 0.0005 and
    QL_MAX_VOLATILITY 3.0

    Revision 1.25  2001/02/15 15:30:30  marmar
    dVolMultiplier_ and dRMultiplier_ defined
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
#include "valueatcenter.h"

namespace QuantLib {

    namespace Pricers {
    
        using FiniteDifferences::BoundaryCondition;
        using FiniteDifferences::BSMOperator;
        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;
        
        const double BSMNumericalOption::dVolMultiplier_ = 0.0001; 
        const double BSMNumericalOption::dRMultiplier_ = 0.0001; 
        
        BSMNumericalOption::BSMNumericalOption(BSMNumericalOption::Type type,
            double underlying, double strike, Rate dividendYield, 
            Rate riskFreeRate, Time residualTime, double volatility, 
            int gridPoints)
        : BSMOption(type, underlying, strike, dividendYield, riskFreeRate,
            residualTime, volatility), rhoComputed_(false),
            vegaComputed_(false),
            gridPoints_(safeGridPoints(gridPoints, residualTime)),
            grid_(gridPoints_), initialPrices_(gridPoints_){
                hasBeenCalculated_ = false;
        }
        
        double BSMNumericalOption::delta() const {
            if (!hasBeenCalculated_)  
                value();
            return delta_;
        }
        
        double BSMNumericalOption::gamma() const {
            if(!hasBeenCalculated_) 
                value();
            return gamma_;
        }
        
        double BSMNumericalOption::theta() const {
            if(!hasBeenCalculated_) 
                value();
            return theta_;
        }

        double BSMNumericalOption::vega() const {
        
            if(!vegaComputed_){
                Handle<BSMOption> brandNewFD = clone();
                double volMinus = volatility_ * (1.0 - dVolMultiplier_);
                brandNewFD -> setVolatility(volMinus);        
                vega_ = (value() - brandNewFD -> value()) / 
                    (volatility_ * dVolMultiplier_);          
                vegaComputed_ = true;
            }
            return vega_;
        }
        
        double BSMNumericalOption::rho() const {
        
            if(!rhoComputed_){
                Handle<BSMOption> brandNewFD = clone();
                Rate rMinus=riskFreeRate_ * (1.0 - dRMultiplier_);        
                brandNewFD -> setRiskFreeRate(rMinus);
                rho_=(value() - brandNewFD -> value()) / 
                    (riskFreeRate_ * dRMultiplier_);
                rhoComputed_  = true;
            }
            return rho_;
        }
         
        void BSMNumericalOption::setGridLimits() const {
            // correction for small volatilities
            double volSqrtTime = volatility_*QL_SQRT(residualTime_);
            // the prefactor fine tunes performance at small volatilities
            double prefactor = 1.0 + 0.02/volSqrtTime;
            double minMaxFactor = QL_EXP(4.0 * prefactor * volSqrtTime);
            
            sMin_ = underlying_/minMaxFactor;  // underlying grid min value
            sMax_ = underlying_*minMaxFactor;  // underlying grid max value
            // insure strike is included in the grid
            double safetyZoneFactor = 1.1;                    
            if(sMin_ > strike_/safetyZoneFactor){
                sMin_ = strike_/safetyZoneFactor;
                // enforce central placement of the underlying
                sMax_ = underlying_/(sMin_/underlying_);    
            }
            if(sMax_ < strike_*safetyZoneFactor){
                sMax_ = strike_*safetyZoneFactor;
                // enforce central placement of the underlying
                sMin_ = underlying_/(sMax_/underlying_);    
            }
        }

        void BSMNumericalOption::initializeGrid() const {            
            gridLogSpacing_ = (QL_LOG(sMax_)-QL_LOG(sMin_))/(gridPoints_-1);
            double edx = QL_EXP(gridLogSpacing_);
            grid_[0] = sMin_;
            int j;
            for (j=1; j<gridPoints_; j++)
                grid_[j] = grid_[j-1]*edx;
        }
        
        void BSMNumericalOption::initializeInitialCondition() const {
            int j;
            switch (type_) {
              case Call:
                for(j=0; j<gridPoints_; j++)
                    initialPrices_[j] = QL_MAX(grid_[j]-strike_,0.0);
                break;
              case Put:
                for(j=0; j<gridPoints_; j++)
                    initialPrices_[j] = QL_MAX(strike_-grid_[j],0.0);
                break;
              case Straddle:
                for(j=0; j<gridPoints_; j++)
                    initialPrices_[j] = QL_FABS(strike_-grid_[j]);
                break;
              default:
                throw IllegalArgumentError(
                    "BSMNumericalOption: invalid option type");  
            }
        }
        
        void BSMNumericalOption::initializeOperator() const {
            finiteDifferenceOperator_ = BSMOperator(gridPoints_, 
                gridLogSpacing_, riskFreeRate_, dividendYield_, volatility_);
                
            finiteDifferenceOperator_.setLowerBC(
                BoundaryCondition(BoundaryCondition::Neumann,
                    initialPrices_[1]-initialPrices_[0]));
                    
            finiteDifferenceOperator_.setHigherBC(
                BoundaryCondition(BoundaryCondition::Neumann,
                    initialPrices_[gridPoints_-1] - 
                        initialPrices_[gridPoints_-2]));
        }
                
    }

}


