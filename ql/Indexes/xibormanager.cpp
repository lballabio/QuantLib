
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/Indexes/xibormanager.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    #if defined(QL_PATCH_SOLARIS)
    bool XiborManager::initialized_ = false;
    XiborManager::HistoryMap* XiborManager::historyMap_ = 0;
    void XiborManager::checkHistoryMap() {
        if (!initialized_) {
            historyMap_ = new HistoryMap;
            initialized_ = true;
        }
    }
    #else
    XiborManager::HistoryMap XiborManager::historyMap_;
    #endif

    void XiborManager::setHistory(const std::string& name,
                                  const History& history) {
        #if defined(QL_PATCH_SOLARIS)
        checkHistoryMap();
        (*historyMap_)[name] = history;
        #else
        historyMap_[name] = history;
        #endif
    }

    #if defined(QL_PATCH_SOLARIS)
    History XiborManager::getHistory(const std::string& name) {
        checkHistoryMap();
        return (*historyMap_)[name];
    }
    #else
    const History& XiborManager::getHistory(const std::string& name) {
        XiborManager::HistoryMap::const_iterator i =
            historyMap_.find(name);
        QL_REQUIRE(i != historyMap_.end(),
                   name + " history not loaded");
        return i->second;
    }
    #endif

    bool XiborManager::hasHistory(const std::string& name) {
        #if defined(QL_PATCH_SOLARIS)
        checkHistoryMap();
        return (historyMap_->find(name) != historyMap_->end());
        #else
        return (historyMap_.find(name) != historyMap_.end());
        #endif
    }

    std::vector<std::string> XiborManager::histories() {
        std::vector<std::string> temp;
        XiborManager::HistoryMap::const_iterator i;
        #if defined(QL_PATCH_SOLARIS)
        checkHistoryMap();
        for (i = historyMap_->begin(); i != historyMap_->end(); i++)
            temp.push_back(i->first);
        #else
        for (i = historyMap_.begin(); i != historyMap_.end(); i++)
            temp.push_back(i->first);
        #endif
        return temp;
    }

    #endif

}

