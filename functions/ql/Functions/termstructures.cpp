/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Katiuscia Manzoni

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

#include <ql/Functions/termstructures.hpp>
#include <ql/TermStructures/ratehelpers.hpp>
#include <ql/TermStructures/bondhelpers.hpp>

using namespace boost;

namespace QuantLib {

    // helper class
    namespace detail {

        class RateHelperPrioritySorter {
          public:
            bool operator()(const boost::shared_ptr<RateHelper>& h1,
                            const boost::shared_ptr<RateHelper>& h2) const {

                if (h1->latestDate() > h2->latestDate())
                    return false;

                if (h1->latestDate() == h2->latestDate()) {
                    if (dynamic_pointer_cast<FuturesRateHelper>(h1)) {
                        return false;
                    } else if (!dynamic_pointer_cast<FuturesRateHelper>(h2)) {
                        if (dynamic_pointer_cast<SwapRateHelper>(h1)) {
                            return false;
                        } else if (!dynamic_pointer_cast<SwapRateHelper>(h2)) {
                            if (dynamic_pointer_cast<FixedCouponBondHelper>(h1)) {
                                return false;
                            }
                        }
                    }
                }

                return true;

            }
        };

    }

    std::vector<shared_ptr<RateHelper> > rateHelperSelection(
        const std::vector<shared_ptr<RateHelper> >& instruments,
        const std::vector<bool>& includeFlag,
        unsigned int nFutures) {

        QL_REQUIRE(!instruments.empty(), "no instrument given");

        Size nInstruments = instruments.size();
        QL_REQUIRE(includeFlag.size()==nInstruments,
            "includeFlag / instruments mismatch");

        // purge input rate helpers according to their includeFlag,
        // their expiration, and maximum number of allowed futures
        std::vector<shared_ptr<RateHelper> > rhs;
        Size i;
        unsigned int futuresCounter = 0;
        for (i=0; i<nInstruments; i++) {
            if (includeFlag[i] && (instruments[i]->earliestDate() <
                                   Settings::instance().evaluationDate())) {
                if (!dynamic_pointer_cast<FuturesRateHelper>(instruments[i])) {
                    rhs.push_back(instruments[i]);
                } else if (futuresCounter<nFutures) {
                    futuresCounter++;
                    rhs.push_back(instruments[i]);
                }
            }
        }

        // zero or one rate helper left
        if (rhs.size()<2) return rhs;

        // sort rate helpers
        std::sort(rhs.begin(),rhs.end(), detail::RateHelperPrioritySorter());

        std::vector<shared_ptr<RateHelper> > result;
        for (i=0; i<rhs.size()-1; i++) {
            if (rhs[i]->latestDate() < rhs[i+1]->latestDate()) 
                result.push_back(rhs[i]);
        }
        result.push_back(rhs[i]);

        return result;
    }


}
