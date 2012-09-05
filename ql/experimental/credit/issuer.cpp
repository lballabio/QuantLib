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

namespace QuantLib {

    namespace {
        bool between(const boost::shared_ptr<DefaultEvent>& e,
                     const Date& start,
                     const Date& end,
                     bool includeRefDate = false) {
            return !e->hasOccurred(start, includeRefDate) &&
                e->hasOccurred(end, includeRefDate);
        }
    }

    Issuer::Issuer(const std::vector<std::pair<DefaultProbKey,
                    Handle<DefaultProbabilityTermStructure> > >&
                    probabilities,
                   const DefaultEventSet& events
        )
    : probabilities_(probabilities), events_(events) { }

    Issuer::Issuer(const std::vector<std::vector<
                       boost::shared_ptr<DefaultType> > >& eventTypes,
                   const std::vector<Currency>& currencies,
                   const std::vector<Seniority>& seniorities,
                   const std::vector<Handle<
                       DefaultProbabilityTermStructure> >& curves,
                   const DefaultEventSet& events)
    : events_(events) {
        QL_REQUIRE((eventTypes.size() == curves.size()) &&
            (curves.size()== currencies.size()) &&
            (currencies.size() == seniorities.size()),
            "Incompatible size of Issuer parameters.");

        for(Size i=0; i <eventTypes.size(); i++) {
            DefaultProbKey keytmp(eventTypes[i], currencies[i],
                seniorities[i]);
            probabilities_.push_back(std::make_pair(keytmp, curves[i]));
        }
    }

    const Handle<DefaultProbabilityTermStructure>&
        Issuer::defaultProbability(const DefaultProbKey& key) const {
        for(Size i=0; i<probabilities_.size(); i++)
            if(key == probabilities_[i].first)
                return probabilities_[i].second;
        QL_FAIL("Probability curve not available.");
    }

    boost::shared_ptr<DefaultEvent>
    Issuer::defaultedBetween(const Date& start,
                             const Date& end,
                             const DefaultProbKey& contractKey,
                             bool includeRefDate
                             ) const
    {
        // to do: the set is ordered, see how to use it to speed this up
        for(DefaultEventSet::const_iterator itev = events_.begin();
            // am i really speeding things up with the date comp?
            itev != events_.end(); // && (*itev)->date() > start;
            itev++) {
            if((*itev)->matchesDefaultKey(contractKey) &&
                between(*itev, start, end, includeRefDate))
                return *itev;
        }
        return boost::shared_ptr<DefaultEvent>();
    }


    std::vector<boost::shared_ptr<DefaultEvent> >
    Issuer::defaultsBetween(const Date& start,
                            const Date& end,
                            const DefaultProbKey& contractKey,
                            bool includeRefDate
                            ) const
    {
        std::vector<boost::shared_ptr<DefaultEvent> > defaults;
        // to do: the set is ordered, see how to use it to speed this up
        for(DefaultEventSet::const_iterator itev = events_.begin();
            // am i really speeding things up with the date comp?
            itev != events_.end(); // && (*itev)->date() > start;
            itev++) {
            if((*itev)->matchesDefaultKey(contractKey) &&
                between(*itev, start, end, includeRefDate))
                defaults.push_back(*itev);
        }
        return defaults;
    }

}
