/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Paul Farrington

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

/*! \file quantobarrieroption.hpp
    \brief Quanto version of a barrier option
*/

#ifndef quantlib_quanto_barrier_option_hpp
#define quantlib_quanto_barrier_option_hpp

#include <ql/instruments/quantovanillaoption.hpp>
#include <ql/instruments/barrieroption.hpp>

namespace QuantLib {

    //! Quanto version of a barrier option
    /*! \ingroup instruments */
    class QuantoBarrierOption : public BarrierOption {
      public:
        typedef BarrierOption::arguments arguments;
        typedef QuantoOptionResults<BarrierOption::results> results;
        QuantoBarrierOption(
                        Barrier::Type barrierType,
                        Real barrier,
                        Real rebate,
                        const boost::shared_ptr<StrikedTypePayoff>& payoff,
                        const boost::shared_ptr<Exercise>& exercise
            );
        //! \name greeks
        //@{
        Real qvega() const;
        Real qrho() const;
        Real qlambda() const;
        //@}
        void fetchResults(const PricingEngine::results*) const;
      private:
        void setupExpired() const;
        // results
        mutable Real qvega_, qrho_, qlambda_;
    };

}


#endif

