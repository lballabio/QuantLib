/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file payoff.hpp
    \brief Option payoff classes

    \fullpath
    ql/%payoff.hpp
*/

// $Id$

#ifndef quantlib_payoff_h
#define quantlib_payoff_h

#include <ql/option.hpp>
#include <functional>

namespace QuantLib {

    //! Vanilla option payoff
    class Payoff : std::unary_function<double,double> {
    public:
        virtual ~Payoff() {}
        Payoff() : type_(Option::Type(-1)), strike_(Null<double>()) {}
        Payoff(Option::Type type,
               double strike)
        : type_(type), strike_(strike) {}
        virtual double operator()(double price) const;
        double strike() const { return strike_; };
        Option::Type optionType() const { return type_; };
    protected:
        Option::Type type_;
        double strike_;
    };

    inline double Payoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return QL_MAX(price-strike_,0.0);
          case Option::Put:
            return QL_MAX(strike_-price,0.0);
          case Option::Straddle:
            return QL_FABS(strike_-price);
          default:
            throw Error("Unknown/Illegal option type");
        }
    }


    //! Binary Cash-Or-Nothing option payoff
    class CashOrNothingPayoff : public Payoff {
    public:
        CashOrNothingPayoff(Option::Type type,
                            double strike,
                            double cashPayoff)
                            : Payoff(type, strike), cashPayoff_(cashPayoff) {}
        double operator()(double price) const;
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
            throw Error("Unknown/Illegal option type");
        }
    }

    //! Binary Asset-Or-Nothing option payoff
    class AssetOrNothingPayoff : public Payoff {
    public:
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
              throw Error("Unknown/Illegal option type");
        }
    }

}

#endif
