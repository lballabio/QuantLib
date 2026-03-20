/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file indexmanager.hpp
    \brief global repository for past index fixings
*/

#ifndef quantlib_index_manager_hpp
#define quantlib_index_manager_hpp

#include <ql/patterns/singleton.hpp>
#include <ql/timeseries.hpp>
#include <ql/math/comparison.hpp>
#include <ql/utilities/observablevalue.hpp>
#include <algorithm>
#include <cctype>

namespace QuantLib {

    //! global repository for past index fixings
    /*! \note index names are case insensitive */
    class IndexManager : public Singleton<IndexManager> {
        friend class Singleton<IndexManager>;
        friend class Index;

      private:
        IndexManager() = default;

      public:
        //! returns all names of the indexes for which fixings were stored
        std::vector<std::string> histories() const;
        //! clears all stored fixings
        void clearHistories();

      private:
        struct CaseInsensitiveCompare {
          bool operator()(const std::string& s1, const std::string& s2) const {
            return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), [](const auto& c1, const auto& c2) {
              return std::toupper(static_cast<unsigned char>(c1)) < std::toupper(static_cast<unsigned char>(c2));
            });
          }
        };

        mutable std::map<std::string, TimeSeries<Real>, CaseInsensitiveCompare> data_;
        mutable std::map<std::string, ext::shared_ptr<Observable>> notifiers_;

        //! add a fixing
        void addFixing(const std::string& name,
                       const Date& fixingDate,
                       Real fixing,
                       bool forceOverwrite = false);
        //! add fixings
        template <class DateIterator, class ValueIterator>
        void addFixings(const std::string& name,
                        DateIterator dBegin,
                        DateIterator dEnd,
                        ValueIterator vBegin,
                        bool forceOverwrite = false,
                        const std::function<bool(const Date& d)>& isValidFixingDate = {}) {
            auto& h = data_[name];
            bool noInvalidFixing = true, noDuplicatedFixing = true;
            Date invalidDate, duplicatedDate;
            Real nullValue = Null<Real>();
            Real invalidValue = Null<Real>();
            Real duplicatedValue = Null<Real>();
            while (dBegin != dEnd) {
                bool validFixing = isValidFixingDate ? isValidFixingDate(*dBegin) : true;
                Real currentValue = h[*dBegin];
                bool missingFixing = forceOverwrite || currentValue == nullValue;
                if (validFixing) {
                    if (missingFixing)
                        h[*(dBegin++)] = *(vBegin++);
                    else if (close(currentValue, *(vBegin))) {
                        ++dBegin;
                        ++vBegin;
                    } else {
                        noDuplicatedFixing = false;
                        duplicatedDate = *(dBegin++);
                        duplicatedValue = *(vBegin++);
                    }
                } else {
                    noInvalidFixing = false;
                    invalidDate = *(dBegin++);
                    invalidValue = *(vBegin++);
                }
            }
            QL_DEPRECATED_DISABLE_WARNING
            notifier(name)->notifyObservers();
            QL_DEPRECATED_ENABLE_WARNING
            QL_REQUIRE(noInvalidFixing, "At least one invalid fixing provided: "
                                            << invalidDate.weekday() << " " << invalidDate << ", "
                                            << invalidValue);
            QL_REQUIRE(noDuplicatedFixing, "At least one duplicated fixing provided: "
                                               << duplicatedDate << ", " << duplicatedValue
                                               << " while " << h[duplicatedDate]
                                               << " value is already present");
        }

        bool hasHistory(const std::string& name) const;
        const TimeSeries<Real>& getHistory(const std::string& name) const;
        void clearHistory(const std::string& name);
        bool hasHistoricalFixing(const std::string& name, const Date& fixingDate) const;
        void setHistory(const std::string& name, TimeSeries<Real> history);
        ext::shared_ptr<Observable> notifier(const std::string& name) const;
    };

}


#endif
