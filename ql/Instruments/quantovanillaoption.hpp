/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file quantovanillaoption.hpp
    \brief Quanto version of a vanilla option
*/

#ifndef quantlib_quanto_vanilla_option_h
#define quantlib_quanto_vanilla_option_h

#include <ql/Instruments/vanillaoption.hpp>
#include <ql/PricingEngines/Quanto/quantoengine.hpp>

namespace QuantLib {

    //! quanto version of a vanilla option
    /*! \ingroup instruments */
    class QuantoVanillaOption : public VanillaOption {
      public:
        typedef QuantoOptionArguments<VanillaOption::arguments> arguments;
        typedef QuantoOptionResults<VanillaOption::results> results;
        typedef QuantoEngine<VanillaOption::arguments,
                             VanillaOption::results> engine;
        QuantoVanillaOption(
                      const Handle<YieldTermStructure>& foreignRiskFreeTS,
                      const Handle<BlackVolTermStructure>& exchRateVolTS,
                      const Handle<Quote>& correlation,
                      const boost::shared_ptr<StochasticProcess>&,
                      const boost::shared_ptr<StrikedTypePayoff>&,
                      const boost::shared_ptr<Exercise>&,
                      const boost::shared_ptr<PricingEngine>&);
        //! \name greeks
        //@{
        Real qvega() const;
        Real qrho() const;
        Real qlambda() const;
        //@}
        void setupArguments(Arguments*) const;
      protected:
        void setupExpired() const;
        void performCalculations() const;
        // arguments
        Handle<YieldTermStructure> foreignRiskFreeTS_;
        Handle<BlackVolTermStructure> exchRateVolTS_;
        Handle<Quote> correlation_;
        // results
        mutable Real qvega_, qrho_, qlambda_;
    };

}


#endif
