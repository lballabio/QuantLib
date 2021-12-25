/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 StatPro Italia srl
 Copyright (C) 2009 Jose Aparicio

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

#include <ql/experimental/credit/issuer.hpp>
#include <utility>

namespace QuantLib {

    namespace {
        bool between(const ext::shared_ptr<DefaultEvent>& e,
                     const Date& start,
                     const Date& end,
                     bool includeRefDate = false) {
            return !e->hasOccurred(start, includeRefDate) &&
                e->hasOccurred(end, includeRefDate);
        }
    }

    Issuer::Issuer(std::vector<std::pair<DefaultProbKey, Handle<DefaultProbabilityTermStructure> > >
                       probabilities,
                   DefaultEventSet events)
    : probabilities_(std::move(probabilities)), events_(std::move(events)) {}

    Issuer::Issuer(const std::vector<std::vector<ext::shared_ptr<DefaultType> > >& eventTypes,
                   const std::vector<Currency>& currencies,
                   const std::vector<Seniority>& seniorities,
                   const std::vector<Handle<DefaultProbabilityTermStructure> >& curves,
                   DefaultEventSet events)
    : events_(std::move(events)) {
        QL_REQUIRE((eventTypes.size() == curves.size()) &&
            (curves.size()== currencies.size()) &&
            (currencies.size() == seniorities.size()),
            "Incompatible size of Issuer parameters.");

        for(Size i=0; i <eventTypes.size(); i++) {
            DefaultProbKey keytmp(eventTypes[i], currencies[i],
                seniorities[i]);
            probabilities_.emplace_back(keytmp, curves[i]);
        }
    }

    const Handle<DefaultProbabilityTermStructure>&
        Issuer::defaultProbability(const DefaultProbKey& key) const {
        for (const auto& probabilitie : probabilities_)
            if (key == probabilitie.first)
                return probabilitie.second;
        QL_FAIL("Probability curve not available.");
    }

    ext::shared_ptr<DefaultEvent>
    Issuer::defaultedBetween(const Date& start,
                             const Date& end,
                             const DefaultProbKey& contractKey,
                             bool includeRefDate
                             ) const
    {
        // to do: the set is ordered, see how to use it to speed this up
        for (const auto& event : events_) {
            if (event->matchesDefaultKey(contractKey) && between(event, start, end, includeRefDate))
                return event;
        }
        return ext::shared_ptr<DefaultEvent>();
    }


    std::vector<ext::shared_ptr<DefaultEvent> >
    Issuer::defaultsBetween(const Date& start,
                            const Date& end,
                            const DefaultProbKey& contractKey,
                            bool includeRefDate
                            ) const
    {
        std::vector<ext::shared_ptr<DefaultEvent> > defaults;
        // to do: the set is ordered, see how to use it to speed this up
        for (const auto& event : events_) {
            if (event->matchesDefaultKey(contractKey) && between(event, start, end, includeRefDate))
                defaults.push_back(event);
        }
        return defaults;
    }

}
