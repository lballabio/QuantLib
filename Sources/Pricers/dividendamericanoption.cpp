
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file dividendamericanoption.cpp
    \brief american option with discrete dividends
    
    $Source$
    $Name$
    $Log$
    Revision 1.10  2001/01/08 10:12:02  lballabio
    Fixed humongous bug where prices were not modified by dividends

    Revision 1.9  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros
    
    Revision 1.8  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks
    
*/

#include "dividendamericanoption.h"
#include "dividendeuropeanoption.h"
#include "finitedifferencemodel.h"
#include "cranknicolson.h"
#include "newcubicspline.h"

namespace QuantLib {

    namespace Pricers {
    
    using Math::Array;
    using Math::NewCubicSpline;
    using FiniteDifferences::FiniteDifferenceModel;
    using FiniteDifferences::CrankNicolson;
    using FiniteDifferences::StepCondition;
    using FiniteDifferences::TridiagonalOperator;

        DividendAmericanOption::DividendAmericanOption(
        DividendAmericanOption::Type type, double underlying, double strike,
            Rate underlyingGrowthRate, Rate riskFreeRate, Time residualTime, 
            double volatility, std::vector<double> dividends, 
            std::vector<Time> exdivdates, int timeSteps, int gridPoints)
        : theDividends(dividends), theExDivDates(exdivdates), 
          theNumberOfDivs(dividends.size()), timeStepPerDiv(timeSteps),
          BSMNumericalOption(type, underlying-addElements(dividends), 
          strike, underlyingGrowthRate, riskFreeRate, residualTime, volatility, 
          gridPoints) {

            QL_REQUIRE(theNumberOfDivs==theExDivDates.size(),
                "the number of dividends is different "
                "from the number of dates");
            QL_REQUIRE(theNumberOfDivs>=1,
                "the number of dividends must be at least one");
            QL_REQUIRE(underlying-addElements(dividends)>0,
                "Dividends cannot exceed underlying");
            QL_REQUIRE(theExDivDates[0]>0,
                "The dividend times must be positive");
            QL_REQUIRE(theExDivDates[theExDivDates.size()-1]<residualTime,
                "The dividend times must be within the residual time");
            QL_REQUIRE(theDividends[0]>=0,
                "Dividends cannot be negative");
            for (unsigned int j=1; j<theNumberOfDivs;j++) {
                QL_REQUIRE(theExDivDates[j-1]<theExDivDates[j],
                    "Dividend dates must be in increasing order");
                QL_REQUIRE(theDividends[j]>=0,
                    "Dividends cannot be negative");
            }
        }

        double DividendAmericanOption::value() const {

            if (!hasBeenCalculated)    {
                double dt = theResidualTime/timeStepPerDiv;              
            
                theOptionIsAmerican = true;
                setGridLimits();
                initializeGrid();
                initializeInitialCondition();
                initializeOperator();
                Array prices = theInitialPrices;
                Array controlPrices = theInitialPrices;

                Time beginDate = theResidualTime;
                int j=theNumberOfDivs-1;
                do {
                    Handle<StepCondition<Array> > americanCondition(
                        new BSMAmericanCondition(theInitialPrices));
                    FiniteDifferenceModel<CrankNicolson<TridiagonalOperator> >
                        model(theOperator);
                    Time endDate;
                    if (j >= 0)
                        endDate = theExDivDates[j];
                    else
                        endDate = dt;
                    if (theOptionIsAmerican)
                        model.rollback(prices, beginDate, endDate, 
                            timeStepPerDiv, americanCondition);
                    else
                        model.rollback(prices,beginDate,endDate,timeStepPerDiv);
                    model.rollback(controlPrices, beginDate, endDate, 
                        timeStepPerDiv);
                    beginDate = endDate;

                    if (j >= 0) {
                        Array oldGrid = theGrid;
                        double centre = valueAtCenter(theGrid);
                        double mltp = centre/theGrid[0];
                        double newMltp = mltp / (1 + (mltp - 1) * 
                        theDividends[j] / (centre+theDividends[j]));
                        QL_ENSURE(newMltp>1,"Dividends are to big");
                        sMin = (centre+theDividends[j])/newMltp;
                        sMax = (centre+theDividends[j])*newMltp;
                        initializeGrid();
                        initializeInitialCondition();
                        initializeOperator();
                        movePricesBeforeExDiv(theDividends[j], theGrid, 
                            prices, oldGrid);
                        movePricesBeforeExDiv(theDividends[j], theGrid, 
                            controlPrices, oldGrid);
                    } else {
                        //Last iteration: option price and greeks are computed
                        double theValuePlus = valueAtCenter(prices) - 
                            valueAtCenter(controlPrices);
                        if(theOptionIsAmerican)
                            model.rollback(prices,dt,0,1,americanCondition);                  
                        else
                            model.rollback(prices,dt,0,1);            
                        model.rollback(controlPrices,dt,0,1);
                      
                        DividendEuropeanOption analitic(theType, 
                            theUnderlying+addElements(theDividends), theStrike, 
                            theUnderlyingGrowthRate, theRiskFreeRate, 
                            theResidualTime, theVolatility, theDividends, 
                            theExDivDates);
                        theValue = valueAtCenter(prices) - 
                            valueAtCenter(controlPrices) + analitic.value();
                        theDelta = firstDerivativeAtCenter(prices,theGrid) - 
                            firstDerivativeAtCenter(controlPrices,theGrid) + 
                            analitic.delta();
                        theGamma = secondDerivativeAtCenter(prices,theGrid) - 
                            secondDerivativeAtCenter(controlPrices,theGrid) + 
                            analitic.gamma();
    
                        if (theOptionIsAmerican)
                            model.rollback(prices,0,-dt,1,americanCondition);
                        else
                            model.rollback(prices,0,-dt,1);            
                        model.rollback(controlPrices,0,-dt,1);
                    
                        double theValueMinus = valueAtCenter(prices) - 
                            valueAtCenter(controlPrices);
                        theTheta = (theValuePlus - theValueMinus) / 
                            (2*dt)+analitic.theta();                                            
                    }
                } while (--j>=-1);                            
                hasBeenCalculated = true;
            }
            return theValue;
        }

        void DividendAmericanOption::movePricesBeforeExDiv(double Div, 
            const Array& newGrid, Array& prices, const Array& oldGrid) const {

            int j;
            Array vOldGrid(oldGrid+Div);
            NewCubicSpline priceSpline(vOldGrid, prices);
            if (theOptionIsAmerican) {
                for (j=0; j<prices.size(); j++)
                    prices[j] = QL_MAX(priceSpline.value(newGrid[j], j) , 
                        theInitialPrices[j]);
            } else {
                for (j=0; j<prices.size(); j++)
                    prices[j] = priceSpline.value(newGrid[j], j);
            }
        }
        
    }
    
}
