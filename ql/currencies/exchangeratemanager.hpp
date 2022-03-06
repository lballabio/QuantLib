/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl
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

/*! \file exchangeratemanager.hpp
    \brief exchange-rate repository
*/

#ifndef quantlib_exchange_rate_manager_hpp
#define quantlib_exchange_rate_manager_hpp

#include <ql/exchangerate.hpp>
#include <ql/patterns/singleton.hpp>
#include <ql/time/date.hpp>
#include <list>
#include <map>
#include <utility>

namespace QuantLib {

    //! exchange-rate repository
    /*! \test lookup of direct, triangulated, and derived exchange
              rates is tested.
    */
    class ExchangeRateManager : public Singleton<ExchangeRateManager> {
        friend class Singleton<ExchangeRateManager>;
      private:
        ExchangeRateManager();
      public:
        //! Add an exchange rate.
        /*! The given rate is valid between the given dates.

            \note If two rates are given between the same currencies
                  and with overlapping date ranges, the latest one
                  added takes precedence during lookup.
        */
        void add(const ExchangeRate&,
                 const Date& startDate = Date::minDate(),
                 const Date& endDate = Date::maxDate());
        /*! Lookup the exchange rate between two currencies at a given
            date.  If the given type is Direct, only direct exchange
            rates will be returned if available; if Derived, direct
            rates are still preferred but derived rates are allowed.

            \warning if two or more exchange-rate chains are possible
                     which allow to specify a requested rate, it is
                     unspecified which one is returned.
        */
        ExchangeRate lookup(const Currency& source,
                            const Currency& target,
                            Date date = Date(),
                            ExchangeRate::Type type =
                                                 ExchangeRate::Derived) const;
        //! remove the added exchange rates
        void clear();

        struct Entry {
            Entry() = default;
            Entry(ExchangeRate rate, const Date& start, const Date& end)
            : rate(std::move(rate)), startDate(start), endDate(end) {}
            ExchangeRate rate;
            Date startDate, endDate;
        };
      private:
        typedef BigInteger Key;
        mutable std::map<Key, std::list<Entry> > data_;
        Key hash(const Currency&, const Currency&) const;
        bool hashes(Key, const Currency&) const;
        void addKnownRates();
        ExchangeRate directLookup(const Currency& source,
                                  const Currency& target,
                                  const Date& date) const;
        ExchangeRate smartLookup(const Currency& source,
                                 const Currency& target,
                                 const Date& date,
                                 std::list<Integer> forbiddenCodes
                                        = std::list<Integer>()) const;
        const ExchangeRate* fetch(const Currency& source,
                                  const Currency& target,
                                  const Date& date) const;
    };

}


#endif


#ifndef id_e43b5717514981cd1f71857cfabe67db
#define id_e43b5717514981cd1f71857cfabe67db
inline bool test_e43b5717514981cd1f71857cfabe67db(const int* i) {
    return i != nullptr;
}
#endif
