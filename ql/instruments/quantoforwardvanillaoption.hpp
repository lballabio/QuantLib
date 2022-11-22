/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file quantoforwardvanillaoption.hpp
    \brief Quanto version of a forward vanilla option
*/

#ifndef quantlib_quanto_forward_vanilla_option_h
#define quantlib_quanto_forward_vanilla_option_h

#include <ql/instruments/quantovanillaoption.hpp>
#include <ql/instruments/forwardvanillaoption.hpp>

namespace QuantLib {

    //! Quanto version of a forward vanilla option
    /*! \ingroup instruments */
    class QuantoForwardVanillaOption : public ForwardVanillaOption {
      public:
        typedef ForwardVanillaOption::arguments arguments;
        typedef QuantoOptionResults<ForwardVanillaOption::results> results;
        QuantoForwardVanillaOption(Real moneyness,
                                   const Date& resetDate,
                                   const ext::shared_ptr<StrikedTypePayoff>&,
                                   const ext::shared_ptr<Exercise>&);
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

