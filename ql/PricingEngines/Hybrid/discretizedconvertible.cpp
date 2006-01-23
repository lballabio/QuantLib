/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Theo Boafo

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/bond.hpp>
#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Instruments/dividendschedule.hpp>
#include <ql/Instruments/callabilityschedule.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/PricingEngines/Hybrid/discretizedconvertible.hpp>

#include <vector>

namespace QuantLib {

    void DiscretizedConvertible::reset(Size size) {

		Size i = arguments_.cashFlows.size()-1;
		
		values_ = Array(size, arguments_.cashFlows[i]->amount());  // Set to bond redemption values + accrued interest if any.

        //values_ = Array(size, arguments_.redemption);  // Set to bond redemption values.
		conversionProbability_ = Array(size, 0.0);	
		spreadAdjustRate_ = Array(size, 0.0);

        // initialise Put, Call and dividend provisions.
        putValues_ = Array(size, 0.0);
        callValues_ = Array(size, 0.0);
        dividendValues_ = Array(size, 0.0);

        DayCounter dayCounter = Actual365Fixed();

		boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                                arguments_.stochasticProcess);

        QL_REQUIRE(process, "Black-Scholes process required");

        Date settlementDate = process->riskFreeRate()->referenceDate();

        DayCounter rfdc  = process->riskFreeRate()->dayCounter();

        // Load Call/Put values that coincide with time steps.
        for (Size i=0; i<arguments_.callability.size(); i++) 
        {

			Time period = dayCounter.yearFraction(settlementDate,
                              arguments_.callability[i].date());
                                                                               
            //! Find time step which coincides with call, put provisions.
            Size j = method()->timeGrid().findIndex(period);
            
			// Add accrued interest to call and put values if any.   
            if (arguments_.callability[i].type() == Callability::Call )

				callValues_[j] = arguments_.callability[i].price().amount() + 
				                 cvbond_->accruedAmount(arguments_.callability[i].date());

            else if (arguments_.callability[i].type() == Callability::Put )

                putValues_[j] = arguments_.callability[i].price().amount() +
				                cvbond_->accruedAmount(arguments_.callability[i].date());
 	 
        }


        adjustValues();

        Real creditSpread = arguments_.creditSpread->value();
        
        Date exercise = arguments_.exercise->lastDate();

        Rate riskFreeRate = process->riskFreeRate()->zeroRate(exercise, rfdc,
                                              Continuous, NoFrequency); 

		Array grid = method()->grid(time());

		
		//boost::shared_ptr<BlackScholesLattice> lattice =
        //    boost::dynamic_pointer_cast<BlackScholesLattice>(method());

        //QL_REQUIRE(lattice, "non-Black-Scholes lattice given");

        //boost::shared_ptr<Tree> tree(lattice->tree());

		// Load Conversion probabilites depending on whether conversion is likely at terminal
        // nodes
        //Size i = size;

        for (Size j=0; j<values_.size(); j++) 
        {

           if ( values_[j] == arguments_.conversionRatio*grid[j] )
           {

               conversionProbability_[j] = 1.0;
        
           }

           //! Calculate blended discount rate to be used on roll back.
           spreadAdjustRate_[j] =  conversionProbability_[j] * riskFreeRate + 
                                    (1-conversionProbability_[j])*(riskFreeRate + creditSpread);
    

        }

        // Load present Value of each cash dividend that coincide with time steps.
        for (Size i=0; i<arguments_.dividends.size(); i++) 
        {

			Time period = dayCounter.yearFraction(settlementDate,arguments_.dividends[i]->date());
                                                                               
            //! Find time step which coincides with cash dividend.
            Size j = method()->timeGrid().findIndex(period);

            if (arguments_.dividends[i]->date() >= settlementDate)
            {
				dividendValues_[j] = arguments_.dividends[i]->amount() *
                           process->riskFreeRate()->discount(arguments_.dividends[i]->date());
                   
            }
 	 
         }

         
    }

    void DiscretizedConvertible::postAdjustValuesImpl() {

        Time now = time();
        Size i;
        switch (arguments_.exercise->type()) {
          case Exercise::American:
            if (now <= arguments_.stoppingTimes[1] &&
                now >= arguments_.stoppingTimes[0])
                applySpecificCondition();
            break;
          case Exercise::European:
            if (isOnTime(arguments_.stoppingTimes[0]))
                applySpecificCondition();
            break;
          case Exercise::Bermudan:
            for (i = 0; i<arguments_.stoppingTimes.size(); i++) {
                if (isOnTime(arguments_.stoppingTimes[i]))
                    applySpecificCondition();
            }
            break;
          default:
            QL_FAIL("invalid option type");
        }
    }

    void DiscretizedConvertible::applySpecificCondition() {

		Array grid = method()->grid(time());

 //       boost::shared_ptr<BlackScholesLattice> lattice =
 //           boost::dynamic_pointer_cast<BlackScholesLattice>(method());

 //       QL_REQUIRE(lattice, "non-Black-Scholes lattice given");

 //       boost::shared_ptr<Tree> tree(lattice->tree());

        Size i = method()->timeGrid().findIndex(time());

        //        Real tempvalue = 0.0;

        boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                                arguments_.stochasticProcess);

        QL_REQUIRE(process, "Black-Scholes process required");

		Date settlementDate = process->riskFreeRate()->referenceDate();

		DayCounter dayCounter = Actual365Fixed();

        Time maturity = dayCounter.yearFraction(settlementDate,arguments_.exercise->lastDate());
                                                         
        if ( time() == maturity )

           // At maturity apply condition for terminal nodes
           for (Size j=0; j<values_.size(); j++) {

              // Add present value of Dividend if any to underlying price value at node
              values_[j] =
				  std::max(values_[j],
                         (arguments_.conversionRatio*(grid[j] + dividendValues_[i])));
          }


        else
                   
            // check for whether bonds are called or puttable and test whether conversion is
           // optimal.
           for (Size j=0; j<values_.size(); j++) {

              Real tempValue = std::max(putValues_[i],std::min(values_[j],callValues_[i]));

              // Add present value of Dividend if any to underlying price value at node
              values_[j] =
                std::max(tempValue,
                         (arguments_.conversionRatio*(grid[j] + dividendValues_[i])));

          }
    }

}


