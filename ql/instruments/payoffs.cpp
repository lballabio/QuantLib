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

#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    std::string NullPayoff::name() const {
        return "Null";
    }

    std::string NullPayoff::description() const {
        return name();
    }

    Real NullPayoff::operator()(Real) const {
        QL_FAIL("dummy payoff given");
    }

    void NullPayoff::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<NullPayoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }



    std::string TypePayoff::description() const {
        std::ostringstream result;
        result << name() << " " << optionType();
        return result.str();
    }

    //std::string StrikedPayoff::description() const {
    //    std::ostringstream result;
    //    result << ", " << strike() << " strike";
    //    return result.str();
    //}

    Real FloatingTypePayoff::operator()(Real) const {
        QL_FAIL("floating payoff not handled");
    }

    Real FloatingTypePayoff::operator()(Real price, Real strike) const {
        switch (type_) {
            case Option::Call:
                return std::max<Real>(price - strike,0.0);
            case Option::Put:
                return std::max<Real>(strike - price,0.0);
            default:
                QL_FAIL("unknown/illegal option type");
        }
    }

    std::string StrikedTypePayoff::description() const {
        std::ostringstream result;
        result << TypePayoff::description() << ", " <<
                  strike() << " strike";
        return result.str();
    }

    void FloatingTypePayoff::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<FloatingTypePayoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Payoff::accept(v);
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
        auto* v1 = dynamic_cast<Visitor<PlainVanillaPayoff>*>(&v);
        if (v1 != nullptr)
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
        auto* v1 = dynamic_cast<Visitor<PercentageStrikePayoff>*>(&v);
        if (v1 != nullptr)
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
        auto* v1 = dynamic_cast<Visitor<AssetOrNothingPayoff>*>(&v);
        if (v1 != nullptr)
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
        auto* v1 = dynamic_cast<Visitor<CashOrNothingPayoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Payoff::accept(v);}

    std::string GapPayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << secondStrike() << " strike payoff";
        return result.str();
    }

    Real GapPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ >= 0.0 ? price-secondStrike_ : 0.0);
          case Option::Put:
            return (strike_-price >= 0.0 ? secondStrike_-price : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    void GapPayoff::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<GapPayoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    Real SuperFundPayoff::operator()(Real price) const {
        return (price>=strike_ && price<secondStrike_) ? price/strike_ : 0.0;
    }

    void SuperFundPayoff::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<SuperFundPayoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }
    std::string SuperSharePayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << secondStrike() << " second strike"<< ", " << cashPayoff() << " amount";
        return result.str();
    }

    Real SuperSharePayoff::operator()(Real price) const {
        return (price>=strike_ && price<secondStrike_) ? cashPayoff_ : 0.0;
    }

    void SuperSharePayoff::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<SuperSharePayoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

}
