/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file dividendbarrieroption.hpp
    \brief Barrier option on a single asset with discrete dividends
*/

#ifndef quantlib_dividend_barrier_option_hpp
#define quantlib_dividend_barrier_option_hpp

#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/dividendschedule.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Single-asset barrier option with discrete dividends
    /*! \ingroup instruments */
    class DividendBarrierOption : public BarrierOption {
      public:
        class arguments;
        class engine;
        DividendBarrierOption(
                        Barrier::Type barrierType,
                        Real barrier,
                        Real rebate,
                        const ext::shared_ptr<StrikedTypePayoff>& payoff,
                        const ext::shared_ptr<Exercise>& exercise,
                        const std::vector<Date>& dividendDates,
                        const std::vector<Real>& dividends);
      protected:
        void setupArguments(PricingEngine::arguments*) const override;

      private:
        DividendSchedule cashFlow_;
    };


    //! %Arguments for dividend barrier option calculation
    class DividendBarrierOption::arguments : public BarrierOption::arguments {
      public:
        DividendSchedule cashFlow;
        arguments() = default;
        void validate() const override;
    };

    //! %Dividend-barrier-option %engine base class
    class DividendBarrierOption::engine
        : public GenericEngine<DividendBarrierOption::arguments,
                               DividendBarrierOption::results> {};

}

#endif
