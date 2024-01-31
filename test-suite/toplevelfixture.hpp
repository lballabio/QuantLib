/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2004, 2005, 2006, 2007, 2008 StatPro Italia srl

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

#ifndef quantlib_top_level_fixture_hpp
#define quantlib_top_level_fixture_hpp

#include <boost/test/unit_test.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    using QuantLib::SavedSettings;
    using QuantLib::IndexManager;

    class TopLevelFixture {  // NOLINT(cppcoreguidelines-special-member-functions)
      public:
        // Restore settings after each test.
        SavedSettings restore;

        TopLevelFixture() = default;

        ~TopLevelFixture() {
            IndexManager::instance().clearHistories();
            BOOST_CHECK(true);
        }

#if BOOST_VERSION <= 105300
        // defined to avoid unused-variable warnings. It doesn't
        // work after Boost 1.53 because the functions were
        // overloaded and the address can't be resolved.
        void _use_check(const void* = &boost::test_tools::check_is_close,
                        const void* = &boost::test_tools::check_is_small) const {}
#endif
    };
}

#endif
