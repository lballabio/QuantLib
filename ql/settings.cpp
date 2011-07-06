/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2011 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2004, 2005, 2007, 2009 StatPro Italia srl

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

#include <ql/settings.hpp>

namespace QuantLib {

    Settings::DateProxy::DateProxy()
    : ObservableValue<Date>(
                            #ifdef QL_TODAY_AS_STICKY_EVALDATE
                            Date::todaysDate())
                            #else
                            Date())
                            #endif
    {}

    std::ostream& operator<<(std::ostream& out,
                             const Settings::DateProxy& p) {
        return out << Date(p);
    }

    Settings::Settings()
    : includeReferenceDateCashFlows_(false),
      enforcesTodaysHistoricFixings_(false) {}

    SavedSettings::SavedSettings()
    : evaluationDate_(Settings::instance().evaluationDate()),
      includeReferenceDateCashFlows_(
                        Settings::instance().includeReferenceDateCashFlows()),
      includeTodaysCashFlows_(Settings::instance().includeTodaysCashFlows()),
      enforcesTodaysHistoricFixings_(
                        Settings::instance().enforcesTodaysHistoricFixings()) {}

    SavedSettings::~SavedSettings() {
        try {
            if (Settings::instance().evaluationDate() != evaluationDate_)
                Settings::instance().evaluationDate() = evaluationDate_;
            Settings::instance().includeReferenceDateCashFlows() =
                includeReferenceDateCashFlows_;
            Settings::instance().includeTodaysCashFlows() =
                includeTodaysCashFlows_;
            Settings::instance().enforcesTodaysHistoricFixings() =
                enforcesTodaysHistoricFixings_;
        } catch (...) {
            // nothing we can do except bailing out.
        }
    }

}
