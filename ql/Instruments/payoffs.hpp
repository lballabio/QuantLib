
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

/*! \file payoffs.hpp
    \brief Payoffs for various options
*/

#ifndef quantlib_payoffs_hpp
#define quantlib_payoffs_hpp

#include <ql/option.hpp>

namespace QuantLib {

    //! Intermediate class for call/put/straddle payoffs
    class TypePayoff : public Payoff {
      public:
        TypePayoff(Option::Type type)
        : type_(type) {}
        Option::Type optionType() const { return type_; };
      protected:
        Option::Type type_;
    };

    //! Intermediate class for payoffs based on a fixed strike
    class StrikedTypePayoff : public TypePayoff {
      public:
        StrikedTypePayoff(Option::Type type,
                          Real strike)
        : TypePayoff(type), strike_(strike) {
            QL_REQUIRE(strike >= 0.0,
                       "negative strike given");
        }
        Real strike() const { return strike_; };
      protected:
        Real strike_;
    };


    //! Plain-vanilla payoff
    class PlainVanillaPayoff : public StrikedTypePayoff {
      public:
        PlainVanillaPayoff(Option::Type type,
                           Real strike)
        : StrikedTypePayoff(type, strike) {}
        Real operator()(Real price) const;
    };

    inline Real PlainVanillaPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return QL_MAX<Real>(price-strike_,0.0);
          case Option::Put:
            return QL_MAX<Real>(strike_-price,0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }


    //! %Payoff with strike expressed as percentage
    class PercentageStrikePayoff : public StrikedTypePayoff {
      public:
        PercentageStrikePayoff(Option::Type type,
                               Real moneyness)
        : StrikedTypePayoff(type, moneyness) {
            QL_REQUIRE(moneyness>=0.0,
                       "negative moneyness not allowed");
        }
        Real operator()(Real price) const;
    };

    inline Real PercentageStrikePayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return price*QL_MAX<Real>(Real(1.0)-strike_,0.0);
          case Option::Put:
            return price*QL_MAX<Real>(strike_-Real(1.0),0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }


    //! Binary cash-or-nothing payoff
    class CashOrNothingPayoff : public StrikedTypePayoff {
      public:
        CashOrNothingPayoff(Option::Type type,
                            Real strike,
                            Real cashPayoff)
        : StrikedTypePayoff(type, strike), cashPayoff_(cashPayoff) {}
        Real operator()(Real price) const;
        Real cashPayoff() const { return cashPayoff_;}
      private:
        Real cashPayoff_;
    };

    inline Real CashOrNothingPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? cashPayoff_ : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? cashPayoff_ : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }


    //! Binary asset-or-nothing payoff
    class AssetOrNothingPayoff : public StrikedTypePayoff {
    public:
        AssetOrNothingPayoff(Option::Type type,
                             Real strike)
        : StrikedTypePayoff(type, strike) {}
        Real operator()(Real price) const;
    };

    inline Real AssetOrNothingPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? price : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? price : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }


    //! Binary gap payoff
    class GapPayoff : public StrikedTypePayoff {
    public:
        GapPayoff(Option::Type type,
                  Real strike,
                  Real strikePayoff)
        : StrikedTypePayoff(type, strike), strikePayoff_(strikePayoff) {}
        Real operator()(Real price) const;
        Real strikePayoff() const { return strikePayoff_;}
    private:
        Real strikePayoff_;
    };

    inline Real GapPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? price-strikePayoff_ : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? strikePayoff_-price : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    //! Binary supershare payoff
    class SuperSharePayoff : public StrikedTypePayoff {
    public:
        SuperSharePayoff(Option::Type type,
                         Real strike,
                         Real strikeIncrement)
        : StrikedTypePayoff(type, strike), strikeIncrement_(strikeIncrement) {}
        Real operator()(Real price) const;
        Real strikeIncrement() const { return strikeIncrement_;}
    private:
        Real strikeIncrement_;
    };

    inline Real SuperSharePayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return ((price-strike_                  > 0.0 ? 1.0 : 0.0)
                   -(price-strike_-strikeIncrement_ > 0.0 ? 1.0 : 0.0))
                / strikeIncrement_;
          case Option::Put:
            return ((strike_                 -price > 0.0 ? 1.0 : 0.0)
                   -(strike_+strikeIncrement_-price > 0.0 ? 1.0 : 0.0))
                / strikeIncrement_;
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

}


#endif
