/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl

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

#include <ql/Indexes/indexmanager.hpp>

namespace QuantLib {

    bool IndexManager::hasHistory(const std::string& name) const {
        return data_.find(name) != data_.end();
    }

    const History& IndexManager::getHistory(const std::string& name) const {
        return data_[name].value();
    }

    void IndexManager::setHistory(const std::string& name,
                                  const History& history) {
        data_[name] = history;
    }

    boost::shared_ptr<Observable> IndexManager::notifier(
                                              const std::string& name) const {
        return data_[name];
    }

    std::vector<std::string> IndexManager::histories() const {
        std::vector<std::string> temp;
        std::map<std::string, ObservableValue<History> >::const_iterator i;
        for (i = data_.begin(); i != data_.end(); i++)
            temp.push_back(i->first);
        return temp;
    }

    void IndexManager::clearHistories() {
        std::map<std::string, ObservableValue<History> >::iterator i;
        for (i = data_.begin(); i != data_.end(); i++)
            i->second = History();
    }

}

