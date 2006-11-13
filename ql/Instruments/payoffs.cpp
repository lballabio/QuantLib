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

#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    std::string TypePayoff::description() const {
        std::ostringstream result;
        result << type() << " " << optionType();
        return result.str();
    }

    Real FloatingTypePayoff::operator()(Real) const {
        QL_FAIL("floating payoff not handled");
    }

    void FloatingTypePayoff::accept(AcyclicVisitor& v) {
        Visitor<FloatingTypePayoff>* v1 =
            dynamic_cast<Visitor<FloatingTypePayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    std::string StrikedTypePayoff::description() const {
        std::ostringstream result;
        result << TypePayoff::description() << ", " << strike() << " strike";
        return result.str();
    }

    Real PlainVanillaPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return std::max<Real>(price-strike_,0.0);
          case Option::Put:
            return std::max<Real>(strike_-price,0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    void PlainVanillaPayoff::accept(AcyclicVisitor& v) {
        Visitor<PlainVanillaPayoff>* v1 =
            dynamic_cast<Visitor<PlainVanillaPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    Real PercentageStrikePayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return price*std::max<Real>(Real(1.0)-strike_,0.0);
          case Option::Put:
            return price*std::max<Real>(strike_-Real(1.0),0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    void PercentageStrikePayoff::accept(AcyclicVisitor& v) {
        Visitor<PercentageStrikePayoff>* v1 =
            dynamic_cast<Visitor<PercentageStrikePayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    Real AssetOrNothingPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? price : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? price : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    void AssetOrNothingPayoff::accept(AcyclicVisitor& v) {
        Visitor<AssetOrNothingPayoff>* v1 =
            dynamic_cast<Visitor<AssetOrNothingPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    std::string CashOrNothingPayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << cashPayoff() << " cash payoff";
        return result.str();
    }

    Real CashOrNothingPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? cashPayoff_ : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? cashPayoff_ : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    void CashOrNothingPayoff::accept(AcyclicVisitor& v) {
        Visitor<CashOrNothingPayoff>* v1 =
            dynamic_cast<Visitor<CashOrNothingPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    std::string GapPayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << strikePayoff() << " strike payoff";
        return result.str();
    }

    Real GapPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? price-strikePayoff_ : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? strikePayoff_-price : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    void GapPayoff::accept(AcyclicVisitor& v) {
        Visitor<GapPayoff>* v1 =
            dynamic_cast<Visitor<GapPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    std::string SuperSharePayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << strikeIncrement() << " strike increment";
        return result.str();
    }

    Real SuperSharePayoff::operator()(Real price) const {
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

    void SuperSharePayoff::accept(AcyclicVisitor& v) {
        Visitor<SuperSharePayoff>* v1 =
            dynamic_cast<Visitor<SuperSharePayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

}
