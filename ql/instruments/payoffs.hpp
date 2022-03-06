/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2006, 2008 StatPro Italia srl
 Copyright (C) 2006 Chiara Fornarola

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

/*! \file payoffs.hpp
    \brief Payoffs for various options
*/

#ifndef quantlib_payoffs_hpp
#define quantlib_payoffs_hpp

#include <ql/option.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    //! Dummy %payoff class
    class NullPayoff : public Payoff {
      public:
        //! \name Payoff interface
        //@{
        std::string name() const override;
        std::string description() const override;
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
        //@}
    };


    //! Intermediate class for put/call payoffs
    class TypePayoff : public Payoff {
      public:
        Option::Type optionType() const { return type_; };
        //! \name Payoff interface
        //@{
        std::string description() const override;
        //@}
      protected:
        explicit TypePayoff(Option::Type type) : type_(type) {}
        Option::Type type_;
    };

    ////! Intermediate class for payoffs based on a fixed strike
    //class StrikedPayoff : public Payoff {
    //  public:
    //    StrikedPayoff(Real strike) : strike_(strike) {}
    //    Real strike() const { return strike_; };
    //    //! \name Payoff interface
    //    //@{
    //    std::string description() const;
    //    //@}
    //  protected:
    //    Real strike_;
    //};

    //! %Payoff based on a floating strike
    class FloatingTypePayoff : public TypePayoff {
      public:
        FloatingTypePayoff(Option::Type type) : TypePayoff(type) {}
        //! \name Payoff interface
        //@{
        std::string name() const override { return "FloatingType"; }
        Real operator()(Real price, Real strike) const;
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
        //@}
    };

    //! Intermediate class for payoffs based on a fixed strike
    class StrikedTypePayoff : public TypePayoff
                              //, public StrikedPayoff
    {
      public:
        //! \name Payoff interface
        //@{
        std::string description() const override;
        //@}
        Real strike() const { return strike_; };
      protected:
        StrikedTypePayoff(Option::Type type,
                          Real strike)
        : TypePayoff(type), strike_(strike) {}
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
        std::string name() const override { return "Vanilla"; }
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
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
        std::string name() const override { return "PercentageStrike"; }
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
        //@}
    };

    /*! Definitions of Binary path-independent payoffs used below,
        can be found in M. Rubinstein, E. Reiner:"Unscrambling The Binary Code", Risk, Vol.4 no.9,1991.
        (see: http://www.in-the-money.com/artandpap/Binary%20Options.doc)
    */

    //! Binary asset-or-nothing payoff
    class AssetOrNothingPayoff : public StrikedTypePayoff {
      public:
        AssetOrNothingPayoff(Option::Type type,
                             Real strike)
        : StrikedTypePayoff(type, strike) {}
        //! \name Payoff interface
        //@{
        std::string name() const override { return "AssetOrNothing"; }
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
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
        std::string name() const override { return "CashOrNothing"; }
        std::string description() const override;
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
        //@}
        Real cashPayoff() const { return cashPayoff_;}
      protected:
        Real cashPayoff_;
    };

    //! Binary gap payoff
    /*! This payoff is equivalent to being a) long a PlainVanillaPayoff at
        the first strike (same Call/Put type) and b) short a
        CashOrNothingPayoff at the first strike (same Call/Put type) with
        cash payoff equal to the difference between the second and the first
        strike.
        \warning this payoff can be negative depending on the strikes
    */
    class GapPayoff : public StrikedTypePayoff {
      public:
        GapPayoff(Option::Type type,
                  Real strike,
                  Real secondStrike) // a.k.a. payoff strike
        : StrikedTypePayoff(type, strike), secondStrike_(secondStrike) {}
        //! \name Payoff interface
        //@{
        std::string name() const override { return "Gap"; }
        std::string description() const override;
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
        //@}
        Real secondStrike() const { return secondStrike_;}
      protected:
        Real secondStrike_;
    };

    //! Binary supershare and superfund payoffs

    //! Binary superfund payoff
    /*! Superfund sometimes also called "supershare", which can lead to ambiguity; within QuantLib
        the terms supershare and superfund are used consistently according to the definitions in
        Bloomberg OVX function's help pages.
    */
    /*! This payoff is equivalent to being (1/lowerstrike) a) long (short) an AssetOrNothing
        Call (Put) at the lower strike and b) short (long) an AssetOrNothing
        Call (Put) at the higher strike
    */
    class SuperFundPayoff : public StrikedTypePayoff {
      public:
        SuperFundPayoff(Real strike,
                        Real secondStrike)
        : StrikedTypePayoff(Option::Call, strike),
          secondStrike_(secondStrike) {
            QL_REQUIRE(strike>0.0,
                       "strike (" <<  strike << ") must be "
                       "positive");
            QL_REQUIRE(secondStrike>strike,
                       "second strike (" <<  secondStrike << ") must be "
                       "higher than first strike (" << strike << ")");
        }
        //! \name Payoff interface
        //@{
        std::string name() const override { return "SuperFund"; }
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
        //@}
        Real secondStrike() const { return secondStrike_;}
      protected:
        Real secondStrike_;
    };

    //! Binary supershare payoff
    class SuperSharePayoff : public StrikedTypePayoff {
      public:
        SuperSharePayoff(Real strike,
                         Real secondStrike,
                         Real cashPayoff)
        : StrikedTypePayoff(Option::Call, strike),
          secondStrike_(secondStrike),
          cashPayoff_(cashPayoff){
              QL_REQUIRE(secondStrike>strike,
              "second strike (" <<  secondStrike << ") must be "
              "higher than first strike (" << strike << ")");}

        //! \name Payoff interface
        //@{
        std::string name() const override { return "SuperShare"; }
        std::string description() const override;
        Real operator()(Real price) const override;
        void accept(AcyclicVisitor&) override;
        //@}
        Real secondStrike() const { return secondStrike_;}
        Real cashPayoff() const { return cashPayoff_;}
      protected:
        Real secondStrike_;
        Real cashPayoff_;
    };
}

#endif


#ifndef id_830965d7f43f6c50d04286f9c75d45e5
#define id_830965d7f43f6c50d04286f9c75d45e5
inline bool test_830965d7f43f6c50d04286f9c75d45e5(const int* i) {
    return i != nullptr;
}
#endif
