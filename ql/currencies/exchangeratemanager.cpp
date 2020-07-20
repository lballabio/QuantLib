/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
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

#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/america.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    namespace {

        struct valid_at {
            Date d;
            explicit valid_at(const Date& d) : d(d) {}
            bool operator()(const ExchangeRateManager::Entry& e) const {
                return d >= e.startDate && d <= e.endDate;
            }
        };

    }

    ExchangeRateManager::ExchangeRateManager() {
        addKnownRates();
    }

    void ExchangeRateManager::add(const ExchangeRate& rate,
                                  const Date& startDate,
                                  const Date& endDate) {
        Key k = hash(rate.source(), rate.target());
        data_[k].push_front(Entry(rate,startDate,endDate));
    }

    ExchangeRate ExchangeRateManager::lookup(const Currency& source,
                                             const Currency& target,
                                             Date date,
                                             ExchangeRate::Type type) const {

        if (source == target)
            return ExchangeRate(source,target,1.0);

        if (date == Date())
            date = Settings::instance().evaluationDate();

        if (type == ExchangeRate::Direct) {
            return directLookup(source,target,date);
        } else if (!source.triangulationCurrency().empty()) {
            const Currency& link = source.triangulationCurrency();
            if (link == target)
                return directLookup(source,link,date);
            else
                return ExchangeRate::chain(directLookup(source,link,date),
                                           lookup(link,target,date));
        } else if (!target.triangulationCurrency().empty()) {
            const Currency& link = target.triangulationCurrency();
            if (source == link)
                return directLookup(link,target,date);
            else
                return ExchangeRate::chain(lookup(source,link,date),
                                           directLookup(link,target,date));
        } else {
            return smartLookup(source,target,date);
        }
    }

    void ExchangeRateManager::clear() {
        data_.clear();
        addKnownRates();
    }

    ExchangeRateManager::Key ExchangeRateManager::hash(
                               const Currency& c1, const Currency& c2) const {
        return Key(std::min(c1.numericCode(),c2.numericCode()))*1000
             + Key(std::max(c1.numericCode(),c2.numericCode()));
    }

    bool ExchangeRateManager::hashes(ExchangeRateManager::Key k,
                                     const Currency& c) const {
        return c.numericCode() == k % 1000 || c.numericCode() == k/1000;
    }

    void ExchangeRateManager::addKnownRates() {
        // currencies obsoleted by Euro
        add(ExchangeRate(EURCurrency(), ATSCurrency(), 13.7603),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), BEFCurrency(), 40.3399),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), DEMCurrency(), 1.95583),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), ESPCurrency(), 166.386),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), FIMCurrency(), 5.94573),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), FRFCurrency(), 6.55957),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), GRDCurrency(), 340.750),
            Date(1,January,2001), Date::maxDate());
        add(ExchangeRate(EURCurrency(), IEPCurrency(), 0.787564),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), ITLCurrency(), 1936.27),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), LUFCurrency(), 40.3399),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), NLGCurrency(), 2.20371),
            Date(1,January,1999), Date::maxDate());
        add(ExchangeRate(EURCurrency(), PTECurrency(), 200.482),
            Date(1,January,1999), Date::maxDate());
        // other obsoleted currencies
        add(ExchangeRate(TRYCurrency(), TRLCurrency(), 1000000.0),
            Date(1,January,2005), Date::maxDate());
        add(ExchangeRate(RONCurrency(), ROLCurrency(), 10000.0),
            Date(1,July,2005), Date::maxDate());
        add(ExchangeRate(PENCurrency(), PEICurrency(), 1000000.0),
            Date(1,July,1991), Date::maxDate());
        add(ExchangeRate(PEICurrency(), PEHCurrency(), 1000.0),
            Date(1,February,1985), Date::maxDate());
    }

    ExchangeRate ExchangeRateManager::directLookup(const Currency& source,
                                                   const Currency& target,
                                                   const Date& date) const {
        if (const ExchangeRate* rate = fetch(source,target,date))
            return *rate;
        else
            QL_FAIL("no direct conversion available from "
                    << source.code() << " to " << target.code()
                    << " for " << date);
    }

    ExchangeRate ExchangeRateManager::smartLookup(
                                         const Currency& source,
                                         const Currency& target,
                                         const Date& date,
                                         std::list<Integer> forbidden) const {
        // direct exchange rates are preferred.
        if (const ExchangeRate* direct = fetch(source,target,date))
            return *direct;

        // if none is found, turn to smart lookup. The source currency
        // is forbidden to subsequent lookups in order to avoid cycles.
        forbidden.push_back(source.numericCode());
        std::map<Key, std::list<Entry> >::const_iterator i;
        for (i = data_.begin(); i != data_.end(); ++i) {
            // we look for exchange-rate data which involve our source
            // currency...
            if (hashes(i->first, source) && !(i->second.empty())) {
                // ...whose other currency is not forbidden...
                const Entry& e = i->second.front();
                const Currency& other =
                    source == e.rate.source() ?
                        e.rate.target() : e.rate.source();
                if (std::find(forbidden.begin(),forbidden.end(),
                              other.numericCode()) == forbidden.end()) {
                    // ...and which carries information for the requested date.
                    if (const ExchangeRate* head = fetch(source,other,date)) {
                        // if we can get to the target from here...
                        try {
                            ExchangeRate tail = smartLookup(other,target,date,
                                                            forbidden);
                            // ..we're done.
                            return ExchangeRate::chain(*head,tail);
                        } catch (Error&) {
                            // otherwise, we just discard this rate.
                            ;
                        }
                    }
                }
            }
        }
        // if the loop completed, we have no way to return the requested rate.
        QL_FAIL("no conversion available from "
                << source.code() << " to " << target.code()
                << " for " << date);
    }

    const ExchangeRate* ExchangeRateManager::fetch(const Currency& source,
                                                   const Currency& target,
                                                   const Date& date) const {
        const std::list<Entry>& rates = data_[hash(source,target)];
        std::list<Entry>::const_iterator i =
            std::find_if(rates.begin(), rates.end(), valid_at(date));
        return i == rates.end() ?
            (const ExchangeRate*) 0 :
            &(i->rate);
    }

}

