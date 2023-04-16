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
        return data_[name].value();
    }

    void IndexManager::setHistory(const std::string& name, TimeSeries<Real> history) {
        data_[name] = std::move(history);
    }

    std::shared_ptr<Observable> IndexManager::notifier(const std::string& name) const {
        return data_[name];
    }

    std::vector<std::string> IndexManager::histories() const {
        std::vector<std::string> temp;
        temp.reserve(data_.size());
        for (const auto& i : data_)
            temp.push_back(i.first);
        return temp;
    }

    void IndexManager::clearHistory(const std::string& name) { data_.erase(name); }

    void IndexManager::clearHistories() { data_.clear(); }

    bool IndexManager::hasHistoricalFixing(const std::string& name, const Date& fixingDate) const {
        auto const& indexIter = data_.find(name);
        return (indexIter != data_.end()) &&
               ((*indexIter).second.value()[fixingDate] != Null<Real>());
    }

}
