
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file forwardvanillaoption.hpp
    \brief Forward version of a vanilla option
*/

#ifndef quantlib_forward_vanilla_option_h
#define quantlib_forward_vanilla_option_h

#include <ql/Instruments/vanillaoption.hpp>
#include <ql/PricingEngines/Forward/forwardengine.hpp>

namespace QuantLib {

    //! Forward version of a vanilla option
    /*! \ingroup instruments */
    class ForwardVanillaOption : public VanillaOption {
      public:
        typedef ForwardOptionArguments<VanillaOption::arguments> arguments;
        typedef VanillaOption::results results;
        typedef ForwardEngine<VanillaOption::arguments,
                              VanillaOption::results> engine;
        ForwardVanillaOption(
            Real moneyness,
            Date resetDate,
            const boost::shared_ptr<StochasticProcess>& stochProc,
            const boost::shared_ptr<StrikedTypePayoff>& payoff,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine);
        void setupArguments(Arguments*) const;
      protected:
        void performCalculations() const;
      private:
        // arguments
        Real moneyness_;
        Date resetDate_;
    };

}


#endif

