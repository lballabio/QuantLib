
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

namespace QuantLib {

    //! %Arguments for forward (strike-resetting) option calculation
    template <class ArgumentsType> 
    class ForwardOptionArguments : public ArgumentsType {
      public:
        ForwardOptionArguments() : moneyness(Null<double>()),
                                   resetDate(Null<Date>()) {}
        void validate() const;
        double moneyness;
        Date resetDate;
    };

    //! Forward version of a vanilla option
    /*! \ingroup instruments */
    class ForwardVanillaOption : public VanillaOption {
      public:
        typedef ForwardOptionArguments<VanillaOption::arguments> arguments;
        typedef VanillaOption::results results;
        ForwardVanillaOption(
            double moneyness,
            Date resetDate,
            const boost::shared_ptr<BlackScholesProcess>& stochProc,
            const boost::shared_ptr<StrikedTypePayoff>& payoff,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine);
        void setupArguments(Arguments*) const;
      protected:
        void performCalculations() const;
      private:
        // arguments
        double moneyness_;
        Date resetDate_;
    };



    template <class ArgumentsType> 
    void ForwardOptionArguments<ArgumentsType>::validate() const {
        ArgumentsType::validate();
        QL_REQUIRE(moneyness != Null<double>(),
                   "null moneyness given");
        QL_REQUIRE(moneyness > 0.0,
                   "negative or zero moneyness given");
        QL_REQUIRE(resetDate != Null<Date>(),
                   "null reset date given");

        Time resetTime =
            blackScholesProcess->riskFreeRate()->dayCounter().yearFraction(
                blackScholesProcess->riskFreeRate()->referenceDate(), resetDate);

        QL_REQUIRE(resetTime >=0,
                   "negative reset time given");
        QL_REQUIRE(exercise->lastDate() >= resetDate,
                   "reset time greater than maturity");
    }

}


#endif

