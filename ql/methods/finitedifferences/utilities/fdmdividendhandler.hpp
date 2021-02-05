/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

/*! \file fdmdividendhandler.hpp
    \brief dividend handler for fdm method for one equity direction
*/

#ifndef quantlib_fdm_dividend_handler_hpp
#define quantlib_fdm_dividend_handler_hpp

#include <ql/instruments/dividendschedule.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>

namespace QuantLib {
    
    class DayCounter;
    
    class FdmDividendHandler : public StepCondition<Array> {
      public:
        FdmDividendHandler(const DividendSchedule& schedule,
                           const ext::shared_ptr<FdmMesher>& mesher,
                           const Date& referenceDate,
                           const DayCounter& dayCounter,
                           Size equityDirection);

        void applyTo(Array& a, Time t) const override;

        const std::vector<Time>& dividendTimes() const;
        const std::vector<Date>& dividendDates() const;
        const std::vector<Real>& dividends() const;
        
      private:
        Array x_; // grid-equity values in physical units

        std::vector<Time> dividendTimes_;
        std::vector<Date> dividendDates_;
        std::vector<Real> dividends_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const Size equityDirection_;
    };
}
#endif
