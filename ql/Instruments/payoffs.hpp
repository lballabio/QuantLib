
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

#include <ql/payoff.hpp>
#include <ql/option.hpp>

namespace QuantLib {

    /*! Intermediate class for all those payoff that have
        a type
    */
    class TypePayoff : public Payoff {
      public:
        TypePayoff(Option::Type type)
        : type_(type) {}
        Option::Type optionType() const { return type_; };
      protected:
        Option::Type type_;
    };

    /*! Intermediate class for all those payoff that have
        a (fixed?) strike
    */
    class StrikedTypePayoff : public TypePayoff {
      public:
        StrikedTypePayoff(Option::Type type,
                          double strike)
        : TypePayoff(type), strike_(strike) {
            QL_REQUIRE(strike >= 0.0,
                       "StrikedTypePayoff: negative strike given");
        }
        double strike() const { return strike_; };
        void setStrike(double strike) { strike_ = strike; };
      protected:
        double strike_;
    };

    /*! The former PlainPayoff has been renamed PlainVanillaPayoff
        to stress that fact that now nobody elses derives from it */
    class PlainVanillaPayoff : public StrikedTypePayoff {
      public:
        PlainVanillaPayoff(Option::Type type,
                           double strike)
        : StrikedTypePayoff(type, strike) {}
        double operator()(double price) const;
    };

    inline double PlainVanillaPayoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return QL_MAX(price-strike_,0.0);
          case Option::Put:
            return QL_MAX(strike_-price,0.0);
          case Option::Straddle:
            return QL_FABS(strike_-price);
          default:
            QL_FAIL("Unknown/Illegal option type");
        }
    }


    class PercentageStrikePayoff : public StrikedTypePayoff {
      public:
        PercentageStrikePayoff(Option::Type type,
                               double moneyness)
        : StrikedTypePayoff(type, moneyness) {
            QL_REQUIRE(moneyness>=0.0,
            "PercentageStrikePayoff::PercentageStrikePayoff :"
            "negative moneyness not allowed");
        }
        double operator()(double price) const;
    };

    inline double PercentageStrikePayoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return price*QL_MAX(1.0-strike_,0.0);
          case Option::Put:
            return price*QL_MAX(strike_-1.0,0.0);
          case Option::Straddle:
            return price*QL_FABS(strike_-1.0);
          default:
            QL_FAIL("Unknown/Illegal option type");
        }
    }

    //! Binary Cash-Or-Nothing payoff
    class CashOrNothingPayoff : public StrikedTypePayoff {
      public:
        CashOrNothingPayoff(Option::Type type,
                            double strike,
                            double cashPayoff)
        : StrikedTypePayoff(type, strike), cashPayoff_(cashPayoff) {}
        double operator()(double price) const;
        double cashPayoff() const { return cashPayoff_;}
      private:
        double cashPayoff_;
    };

    inline double CashOrNothingPayoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? cashPayoff_ : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? cashPayoff_ : 0.0);
          case Option::Straddle:
            return cashPayoff_;
          default:
            QL_FAIL("Unknown/Illegal option type");
        }
    }


    //! Binary Asset-Or-Nothing payoff
    class AssetOrNothingPayoff : public StrikedTypePayoff {
    public:
        AssetOrNothingPayoff(Option::Type type,
                             double strike)
        : StrikedTypePayoff(type, strike) {}
        double operator()(double price) const;
    };

    inline double AssetOrNothingPayoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? price : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? price : 0.0);
          case Option::Straddle:
            return price;
          default:
            QL_FAIL("Unknown/Illegal option type");
        }
    }


    //! Binary Gap payoff
    class GapPayoff : public StrikedTypePayoff {
    public:
        GapPayoff(Option::Type type,
                  double strike,
                  double strikePayoff)
        : StrikedTypePayoff(type, strike), strikePayoff_(strikePayoff) {}
        double operator()(double price) const;
        double strikePayoff() const { return strikePayoff_;}
    private:
        double strikePayoff_;
    };

    inline double GapPayoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? price-strikePayoff_ : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? strikePayoff_-price : 0.0);
          case Option::Straddle:
            return 0.0;
          default:
            QL_FAIL("Unknown/Illegal option type");
        }
    }

    //! Binary supershare payoff
    class SuperSharePayoff : public StrikedTypePayoff {
    public:
        SuperSharePayoff(Option::Type type,
                         double strike,
                         double strikeIncrement)
        : StrikedTypePayoff(type, strike), strikeIncrement_(strikeIncrement) {}
        double operator()(double price) const;
        double strikeIncrement() const { return strikeIncrement_;}
    private:
        double strikeIncrement_;
    };

    inline double SuperSharePayoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return ((price-strike_                  > 0.0 ? 1.0 : 0.0)
                   -(price-strike_-strikeIncrement_ > 0.0 ? 1.0 : 0.0))
                / strikeIncrement_;
          case Option::Put:
            return ((strike_                 -price > 0.0 ? 1.0 : 0.0)
                   -(strike_+strikeIncrement_-price > 0.0 ? 1.0 : 0.0))
                / strikeIncrement_;
          case Option::Straddle:
            return -1.0;
          default:
            QL_FAIL("Unknown/Illegal option type");
        }
    }

}


#endif
