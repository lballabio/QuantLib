/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl

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

#include <ql/indexes/indexmanager.hpp>

namespace QuantLib {

    bool IndexManager::hasHistory(const std::string& name) const {
        return data_.find(name) != data_.end();
    }

    const TimeSeries<Real>& IndexManager::getHistory(const std::string& name) const {
        return data_[name];
    }

    void IndexManager::setHistory(const std::string& name, TimeSeries<Real> history) {
        QL_DEPRECATED_DISABLE_WARNING
        notifier(name)->notifyObservers();
        QL_DEPRECATED_ENABLE_WARNING
        data_[name] = std::move(history);
    }

    void IndexManager::addFixing(const std::string& name,
                                 const Date& fixingDate,
                                 Real fixing,
                                 bool forceOverwrite) {
        addFixings(name, &fixingDate, (&fixingDate) + 1, &fixing, forceOverwrite);
    }

    ext::shared_ptr<Observable> IndexManager::notifier(const std::string& name) const {
        auto n = notifiers_.find(name);
        if(n != notifiers_.end())
            return n->second;
        auto o = ext::make_shared<Observable>();
        notifiers_[name] = o;
        return o;
    }

    std::vector<std::string> IndexManager::histories() const {
        std::vector<std::string> temp;
        temp.reserve(data_.size());
        for (const auto& i : data_)
            temp.push_back(i.first);
        return temp;
    }

    void IndexManager::clearHistory(const std::string& name) {
        QL_DEPRECATED_DISABLE_WARNING
        notifier(name)->notifyObservers();
        QL_DEPRECATED_ENABLE_WARNING
        data_.erase(name);
    }

    void IndexManager::clearHistories() {
        QL_DEPRECATED_DISABLE_WARNING
        for (auto const& d : data_)
            notifier(d.first)->notifyObservers();
        QL_DEPRECATED_ENABLE_WARNING
        data_.clear();
    }

    bool IndexManager::hasHistoricalFixing(const std::string& name, const Date& fixingDate) const {
        auto const& indexIter = data_.find(name);
        return (indexIter != data_.end()) &&
               ((*indexIter).second[fixingDate] != Null<Real>());
    }

}
