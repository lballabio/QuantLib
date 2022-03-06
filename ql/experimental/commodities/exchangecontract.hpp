/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

/*! \file exchangecontract.hpp
    \brief Exchange contract
*/

#ifndef quantlib_exchange_contract_hpp
#define quantlib_exchange_contract_hpp

#include <ql/time/date.hpp>
#include <map>
#include <utility>

namespace QuantLib {

    class ExchangeContract {
      public:
        ExchangeContract() = default;
        ExchangeContract(std::string code,
                         Date expirationDate,
                         Date underlyingStartDate,
                         Date underlyingEndDate);

        const std::string& code() const;
        const Date& expirationDate() const;
        const Date& underlyingStartDate() const;
        const Date& underlyingEndDate() const;
      protected:
        std::string code_;
        Date expirationDate_;
        Date underlyingStartDate_;
        Date underlyingEndDate_;
    };

    inline ExchangeContract::ExchangeContract(std::string code,
                                              Date expirationDate,
                                              Date underlyingStartDate,
                                              Date underlyingEndDate)
    : code_(std::move(code)), expirationDate_(expirationDate),
      underlyingStartDate_(underlyingStartDate), underlyingEndDate_(underlyingEndDate) {}

    inline const std::string& ExchangeContract::code() const {
        return code_;
    }

    inline const Date& ExchangeContract::expirationDate() const {
        return expirationDate_;
    }

    inline const Date& ExchangeContract::underlyingStartDate() const {
        return underlyingStartDate_;
    }

    inline const Date& ExchangeContract::underlyingEndDate() const {
        return underlyingEndDate_;
    }

    typedef std::map<Date, ExchangeContract> ExchangeContracts;

}

#endif


#ifndef id_2560bbab9275dee01322a3caa8277293
#define id_2560bbab9275dee01322a3caa8277293
inline bool test_2560bbab9275dee01322a3caa8277293(const int* i) {
    return i != nullptr;
}
#endif
