/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quantodoublebarrieroption.hpp
    \brief Quanto version of a double barrier option
*/

#ifndef quantlib_quanto_double_barrier_option_hpp
#define quantlib_quanto_double_barrier_option_hpp

#include <ql/instruments/quantovanillaoption.hpp>
#include <ql/instruments/doublebarrieroption.hpp>

namespace QuantLib {

    //! Quanto version of a double barrier option
    /*! \ingroup instruments */
    class QuantoDoubleBarrierOption : public DoubleBarrierOption {
      public:
        typedef DoubleBarrierOption::arguments arguments;
        typedef QuantoOptionResults<DoubleBarrierOption::results> results;
        QuantoDoubleBarrierOption(
                        DoubleBarrier::Type barrierType,
                        Real barrier_lo,
                        Real barrier_hi,
                        Real rebate,
                        const ext::shared_ptr<StrikedTypePayoff>& payoff,
                        const ext::shared_ptr<Exercise>& exercise
            );
        //! \name greeks
        //@{
        Real qvega() const;
        Real qrho() const;
        Real qlambda() const;
        //@}
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void setupExpired() const override;
        // results
        mutable Real qvega_, qrho_, qlambda_;
    };

}


#endif

