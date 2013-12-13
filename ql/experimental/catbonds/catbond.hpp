/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file catbond.hpp
    \brief cat bond class
*/

#ifndef quantlib_catbond_hpp
#define quantlib_catbond_hpp

#include <ql/instruments/bond.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/experimental/catbonds/catrisk.hpp>
#include <ql/experimental/catbonds/riskynotional.hpp>

namespace QuantLib {

	class CatBond : public Bond
	{
	public:
        class arguments;
        class results;
        class engine;

		CatBond(Natural settlementDays, 
			    const Calendar& calendar,
                const Date& issueDate,
                boost::shared_ptr<NotionalRisk> notionalRisk)       
                : Bond(settlementDays, calendar, issueDate), 
                  notionalRisk_(notionalRisk)
        {}
        virtual ~CatBond(void) {}

        virtual void setupArguments(PricingEngine::arguments*) const;
        virtual void fetchResults(const PricingEngine::results*) const;

        Real lossProbability() { return lossProbability_; }
        Real expectedLoss() { return expectedLoss_; }
        Real exhaustionProbability() { return exhaustionProbability_; }
    protected:
        boost::shared_ptr<NotionalRisk> notionalRisk_;

        mutable Real lossProbability_;
        mutable Real exhaustionProbability_;
        mutable Real expectedLoss_;
	};

    class CatBond::arguments : public Bond::arguments {
      public:
        Date startDate;
        boost::shared_ptr<NotionalRisk> notionalRisk;
        void validate() const;
    };

    //! results for a cat bond calculation
    class CatBond::results : public Bond::results {
      public:
        Real lossProbability;
        Real exhaustionProbability;
        Real expectedLoss;
    };

    //! base class for cat bond engine
    class CatBond::engine
        : public GenericEngine<CatBond::arguments,
                               CatBond::results> {};


    //! floating-rate cat bond (possibly capped and/or floored)
    /*! \ingroup instruments

        \test calculations are tested by checking results against
              cached values.
    */
    class FloatingCatBond : public CatBond {
      public:
        FloatingCatBond(Natural settlementDays,
                         Real faceAmount,
                         const Schedule& schedule,
                         const boost::shared_ptr<IborIndex>& iborIndex,
                         const DayCounter& accrualDayCounter,
                         boost::shared_ptr<NotionalRisk> notionalRisk,
                         BusinessDayConvention paymentConvention
                                             = Following,
                         Natural fixingDays = Null<Natural>(),
                         const std::vector<Real>& gearings
                                             = std::vector<Real>(1, 1.0),
                         const std::vector<Spread>& spreads
                                             = std::vector<Spread>(1, 0.0),
                         const std::vector<Rate>& caps
                                             = std::vector<Rate>(),
                         const std::vector<Rate>& floors
                                            = std::vector<Rate>(),
                         bool inArrears = false,
                         Real redemption = 100.0,
                         const Date& issueDate = Date());

        FloatingCatBond(Natural settlementDays,
                         Real faceAmount,
                         const Date& startDate,
                         const Date& maturityDate,
                         Frequency couponFrequency,
                         const Calendar& calendar,
                         const boost::shared_ptr<IborIndex>& iborIndex,
                         const DayCounter& accrualDayCounter,
                         boost::shared_ptr<NotionalRisk> notionalRisk,
                         BusinessDayConvention accrualConvention = Following,
                         BusinessDayConvention paymentConvention = Following,
                         Natural fixingDays = Null<Natural>(),
                         const std::vector<Real>& gearings
                                             = std::vector<Real>(1, 1.0),
                         const std::vector<Spread>& spreads
                                             = std::vector<Spread>(1, 0.0),
                         const std::vector<Rate>& caps = std::vector<Rate>(),
                         const std::vector<Rate>& floors = std::vector<Rate>(),
                         bool inArrears = false,
                         Real redemption = 100.0,
                         const Date& issueDate = Date(),
                         const Date& stubDate = Date(),
                         DateGeneration::Rule rule = DateGeneration::Backward,
                         bool endOfMonth = false);
    };
	

}

#endif
