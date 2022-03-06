/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file holderextensibleoption.hpp
    \brief Holder-extensible option
*/

#ifndef quantlib_holder_extensible_option_hpp
#define quantlib_holder_extensible_option_hpp

#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/oneassetoption.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    class HolderExtensibleOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        HolderExtensibleOption(
                       Option::Type type,
                       Real premium,
                       Date secondExpiryDate,
                       Real secondStrike,
                       const ext::shared_ptr<StrikedTypePayoff>& payoff,
                       const ext::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        Real premium_;
        Date secondExpiryDate_;
        Real secondStrike_;
    };

    class HolderExtensibleOption::arguments : public OneAssetOption::arguments {
      public:
        Real premium;
        Date secondExpiryDate;
        Real secondStrike;

        void validate() const override;
    };

    class HolderExtensibleOption::engine
        : public GenericEngine<HolderExtensibleOption::arguments,
                               HolderExtensibleOption::results> {};

}


#endif


#ifndef id_8a64f35d8c15e4a442ac509ccca7f0a0
#define id_8a64f35d8c15e4a442ac509ccca7f0a0
inline bool test_8a64f35d8c15e4a442ac509ccca7f0a0(int* i) { return i != 0; }
#endif
