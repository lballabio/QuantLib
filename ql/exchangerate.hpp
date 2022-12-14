/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl
 Copyright (C) 2004 Decillion Pty(Ltd)

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

/*! \file exchangerate.hpp
    \brief exchange rate between two currencies
*/

#ifndef quantlib_exchange_rate_hpp
#define quantlib_exchange_rate_hpp

#include <ql/money.hpp>
#include <ql/utilities/null.hpp>
#include <utility>

namespace QuantLib {

    //! exchange rate between two currencies
    /*! \test application of direct and derived exchange rate is
              tested against calculations.
    */
    class ExchangeRate {
      public:
        enum Type { Direct,  /*!< given directly by the user */
                    Derived  /*!< derived from exchange rates between
                                  other currencies */
        };
        //! \name Constructors
        //@{
        ExchangeRate();
        /*! the rate \f$ r \f$ is given with the convention that a
            unit of the source is worth \f$ r \f$ units of the target.
        */
        ExchangeRate(Currency source, Currency target, Decimal rate);
        //@}

        //! \name Inspectors
        //@{
        //! the source currency.
        const Currency& source() const;
        //! the target currency.
        const Currency& target() const;
        //! the type
        Type type() const;
        //! the exchange rate (when available)
        Decimal rate() const;
        //@}

        //! \name Utility methods
        //@{
        //! apply the exchange rate to a cash amount
        Money exchange(const Money& amount) const;
        //! chain two exchange rates
        static ExchangeRate chain(const ExchangeRate& r1,
                                  const ExchangeRate& r2);
        //@}
      private:
        Currency source_, target_;
        Decimal rate_;
        Type type_;
        std::pair<ext::shared_ptr<ExchangeRate>,
                  ext::shared_ptr<ExchangeRate> > rateChain_;
    };


    // inline definitions

    inline ExchangeRate::ExchangeRate() : rate_(Null<Decimal>()), type_(Direct) {}

    inline ExchangeRate::ExchangeRate(Currency source, Currency target, Decimal rate)
    : source_(std::move(source)), target_(std::move(target)), rate_(rate), type_(Direct) {}

    inline const Currency& ExchangeRate::source() const {
        return source_;
    }

    inline const Currency& ExchangeRate::target() const {
        return target_;
    }

    inline ExchangeRate::Type ExchangeRate::type() const {
        return type_;
    }

    inline Decimal ExchangeRate::rate() const {
        return rate_;
    }

}


#endif
