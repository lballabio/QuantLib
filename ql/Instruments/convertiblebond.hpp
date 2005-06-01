/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2005 QuantLib

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

/*! \file convertiblebond.hpp
    \brief This is a convertible class under active development.  It may change radically.  Please subscribe to the quantlib list.
*/


#ifndef quantlib_convertible_bond_hpp
#define quantlib_convertible_bond_hpp

#include <ql/Instruments/bond.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/Instruments/dividendschedule.hpp>
#include <ql/Instruments/callabilityschedule.hpp>
namespace QuantLib {
    class DayCounter;
    class Schedule;
    class Quote;
    class Date;

class ConvertibleBond: public Bond {
public:
    ConvertibleBond(
                    const boost::shared_ptr<GenericStochasticProcess>& stochProc,
                    Real  conversionRatio,  
                    const DividendSchedule&  dividends,
                    const CallabilitySchedule& callability,
                    const Handle<Quote>& creditSpread,
                    // Bond Parameters
                    const Date& issueDate,
                    Integer settlementDays,
                    Rate coupon,
                    const DayCounter& dayCounter,
                    const Schedule& schedule,
                    Real redemption = 100
                    );  // constructor
    virtual ~ConvertibleBond();
    Real conversionRatio() const;
    const DividendSchedule& dividends() const;
    const CallabilitySchedule& callability() const;
    const Handle<Quote>& creditSpread() const;
private:
    Real conversionRatio_;
    Spread creditSpreadRates_;
    CallabilitySchedule callability_;
    DividendSchedule dividends_;
    Handle<Quote> creditSpread_;
};  
}
#endif
