
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quantoforwardvanillaoption.hpp
    \brief Quanto version of a forward vanilla option
*/

#ifndef quantlib_quanto_forward_vanilla_option_h
#define quantlib_quanto_forward_vanilla_option_h

#include <ql/Instruments/quantovanillaoption.hpp>
#include <ql/Instruments/forwardvanillaoption.hpp>

namespace QuantLib {

    //! Quanto version of a forward vanilla option
    /*! \ingroup instruments */ 
    class QuantoForwardVanillaOption : public QuantoVanillaOption {
      public:
        typedef QuantoOptionArguments<ForwardVanillaOption::arguments>
                                                                 arguments;
        typedef QuantoOptionResults<ForwardVanillaOption::results>
                                                                 results;
        typedef QuantoEngine<ForwardVanillaOption::arguments,
                             ForwardVanillaOption::results> engine;
        QuantoForwardVanillaOption(
                           const Handle<TermStructure>& foreignRiskFreeTS,
                           const Handle<BlackVolTermStructure>& exchRateVolTS,
                           const Handle<Quote>& correlation,
                           Real moneyness,
                           Date resetDate,
                           const boost::shared_ptr<BlackScholesProcess>&,
                           const boost::shared_ptr<StrikedTypePayoff>& payoff,
                           const boost::shared_ptr<Exercise>& exercise,
                           const boost::shared_ptr<PricingEngine>& engine);
        void setupArguments(Arguments*) const;
      private:
        void performCalculations() const;
        // arguments
        Real moneyness_;
        Date resetDate_;
    };

}


#endif

