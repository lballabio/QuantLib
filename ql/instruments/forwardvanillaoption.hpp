/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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

/*! \file forwardvanillaoption.hpp
    \brief Forward version of a vanilla option
*/

#ifndef quantlib_forward_vanilla_option_hpp
#define quantlib_forward_vanilla_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    //! %Arguments for forward (strike-resetting) option calculation
    template <class ArgumentsType>
    class ForwardOptionArguments : public ArgumentsType {
      public:
        ForwardOptionArguments() : moneyness(Null<Real>()),
                                   resetDate(Null<Date>()) {}
        void validate() const override;
        Real moneyness;
        Date resetDate;
    };

    //! %Forward version of a vanilla option
    /*! \ingroup instruments */
    class ForwardVanillaOption : public OneAssetOption {
      public:
        typedef ForwardOptionArguments<OneAssetOption::arguments> arguments;
        typedef OneAssetOption::results results;
        ForwardVanillaOption(Real moneyness,
                             const Date& resetDate,
                             const ext::shared_ptr<StrikedTypePayoff>& payoff,
                             const ext::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;

      private:
        // arguments
        Real moneyness_;
        Date resetDate_;
    };


    // template definitions

    template <class ArgumentsType>
    inline void ForwardOptionArguments<ArgumentsType>::validate() const {
        ArgumentsType::validate();

        QL_REQUIRE(moneyness != Null<Real>(), "null moneyness given");
        QL_REQUIRE(moneyness > 0.0, "negative or zero moneyness given");

        QL_REQUIRE(resetDate != Null<Date>(), "null reset date given");
        QL_REQUIRE(resetDate >= Settings::instance().evaluationDate(),
                   "reset date in the past");
        QL_REQUIRE(this->exercise->lastDate() > resetDate,
                   "reset date later or equal to maturity");
    }


}


#endif



#ifndef id_1eb779ad4e11e47d943b6c816c74d69f
#define id_1eb779ad4e11e47d943b6c816c74d69f
inline bool test_1eb779ad4e11e47d943b6c816c74d69f(int* i) { return i != 0; }
#endif
