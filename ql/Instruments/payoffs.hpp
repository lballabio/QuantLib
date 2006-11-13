/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file payoffs.hpp
    \brief Payoffs for various options
*/

#ifndef quantlib_payoffs_hpp
#define quantlib_payoffs_hpp

#include <ql/option.hpp>

namespace QuantLib {

    //! Intermediate class for put/call payoffs
    class TypePayoff : public Payoff {
      public:
        TypePayoff(Option::Type type) : type_(type) {}
        Option::Type optionType() const { return type_; };
        //! \name Payoff interface
        //@{
        std::string description() const;
        //@}
      protected:
        Option::Type type_;
    };

    //! Payoff based on a floating strike
    class FloatingTypePayoff : public TypePayoff {
      public:
        FloatingTypePayoff(Option::Type type) : TypePayoff(type) {}
        //! \name Payoff interface
        //@{
        std::string type() const { return "FloatingType";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! Intermediate class for payoffs based on a fixed strike
    class StrikedTypePayoff : public TypePayoff {
      public:
        StrikedTypePayoff(Option::Type type,
                          Real strike)
        : TypePayoff(type), strike_(strike) {}
        Real strike() const { return strike_; };
        //! \name Payoff interface
        //@{
        std::string description() const;
        //@}
      protected:
        Real strike_;
    };

    //! Plain-vanilla payoff
    class PlainVanillaPayoff : public StrikedTypePayoff {
      public:
        PlainVanillaPayoff(Option::Type type,
                           Real strike)
        : StrikedTypePayoff(type, strike) {}
        //! \name Payoff interface
        //@{
        std::string type() const { return "Vanilla";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! %Payoff with strike expressed as percentage
    class PercentageStrikePayoff : public StrikedTypePayoff {
      public:
        PercentageStrikePayoff(Option::Type type,
                               Real moneyness)
        : StrikedTypePayoff(type, moneyness) {}
        //! \name Payoff interface
        //@{
        std::string type() const { return "PercentageStrike";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! Binary asset-or-nothing payoff
    class AssetOrNothingPayoff : public StrikedTypePayoff {
      public:
        AssetOrNothingPayoff(Option::Type type,
                             Real strike)
        : StrikedTypePayoff(type, strike) {}
        //! \name Payoff interface
        //@{
        std::string type() const { return "AssetOrNothing";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! Binary cash-or-nothing payoff
    class CashOrNothingPayoff : public StrikedTypePayoff {
      public:
        CashOrNothingPayoff(Option::Type type,
                            Real strike,
                            Real cashPayoff)
        : StrikedTypePayoff(type, strike), cashPayoff_(cashPayoff) {}
        //! \name Payoff interface
        //@{
        std::string type() const { return "CashOrNothing";}
        std::string description() const;
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
        Real cashPayoff() const { return cashPayoff_;}
      private:
        Real cashPayoff_;
    };

    //! Binary gap payoff
    class GapPayoff : public StrikedTypePayoff {
      public:
        GapPayoff(Option::Type type,
                  Real strike,
                  Real strikePayoff)
        : StrikedTypePayoff(type, strike), strikePayoff_(strikePayoff) {}
        //! \name Payoff interface
        //@{
        std::string type() const { return "Gap";}
        std::string description() const;
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
        Real strikePayoff() const { return strikePayoff_;}
      private:
        Real strikePayoff_;
    };

    //! Binary supershare payoff
    class SuperSharePayoff : public StrikedTypePayoff {
      public:
        SuperSharePayoff(Option::Type type,
                         Real strike,
                         Real strikeIncrement)
        : StrikedTypePayoff(type, strike), strikeIncrement_(strikeIncrement) {}
        //! \name Payoff interface
        //@{
        std::string type() const { return "SuperShare";}
        std::string description() const;
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
        Real strikeIncrement() const { return strikeIncrement_;}
      private:
        Real strikeIncrement_;
    };
}

#endif
