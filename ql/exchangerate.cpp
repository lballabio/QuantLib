
/*
 Copyright (C) 2004 StatPro Italia srl
 Copyright (C) 2004 Decillion Pty(Ltd)

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

#include <ql/exchangerate.hpp>

namespace QuantLib {

    Money ExchangeRate::exchange(const Money& amount) const {
        switch (type_) {
          case Direct:
            if (amount.currency() == source_)
                return Money(amount.value()*rate_, target_);
            else if (amount.currency() == target_)
                return Money(amount.value()/rate_, source_);
            else
                QL_FAIL("exchange rate not applicable");
            break;
          case Derived:
            if (amount.currency() == rateChain_.front().source() ||
                amount.currency() == rateChain_.front().target()) {
                return rateChain_.back().exchange(
                                         rateChain_.front().exchange(amount));
            } else if (amount.currency() == rateChain_.back().source() ||
                       amount.currency() == rateChain_.back().target()) {
                return rateChain_.front().exchange(
                                          rateChain_.back().exchange(amount));
            } else {
                QL_FAIL("exchange rate not applicable");
            }
            break;
          default:
            QL_FAIL("unknown exchange-rate type");
        }
    }

    ExchangeRate ExchangeRate::chain(const ExchangeRate& r1,
                                     const ExchangeRate& r2) {
        ExchangeRate result;
        result.type_ = Derived;
        result.rateChain_.push_back(r1);
        result.rateChain_.push_back(r2);
        if (r1.source_ == r2.source_) {
            result.source_ = r1.target_;
            result.target_ = r2.target_;
        } else if (r1.source_ == r2.target_) {
            result.source_ = r1.target_;
            result.target_ = r2.source_;
        } else if (r1.target_ == r2.source_) {
            result.source_ = r1.source_;
            result.target_ = r2.target_;
        } else if (r1.target_ == r2.target_) {
            result.source_ = r1.source_;
            result.target_ = r2.source_;
        } else {
            QL_FAIL("exchange rates not chainable");
        }
        return result;
    }

}
