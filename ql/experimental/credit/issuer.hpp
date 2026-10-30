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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file issuer.hpp
    \brief Classes for credit-name handling.
*/

#ifndef quantlib_issuer_hpp
#define quantlib_issuer_hpp

#include <ql/experimental/credit/defaultevent.hpp>
#include <ql/experimental/credit/defaultprobabilitykey.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <set>
#include <vector>

namespace QuantLib {

    typedef std::set<ext::shared_ptr<DefaultEvent>,
                earlier_than<ext::shared_ptr<DefaultEvent> > >
            DefaultEventSet;

    class Issuer {
      public:
        typedef std::pair<DefaultProbKey,
                          Handle<DefaultProbabilityTermStructure> >
                                                               key_curve_pair;
        /*! The first argument represents the probability of an issuer
            of having any of its bonds with the given seniority,
            currency incurring in that particular event.  The second
            argument represents the history of past events.  Theres no
            check on whether the event list makes sense, events can
            occur several times and several of them can take place on
            the same date.

            To do: add settlement event access
        */
        Issuer(std::vector<key_curve_pair> probabilities = std::vector<key_curve_pair>(),
               DefaultEventSet events = DefaultEventSet());

        Issuer(const std::vector<std::vector<ext::shared_ptr<DefaultType> > >& eventTypes,
               const std::vector<Currency>& currencies,
               const std::vector<Seniority>& seniorities,
               const std::vector<Handle<DefaultProbabilityTermStructure> >& curves,
               DefaultEventSet events = DefaultEventSet());

        //! \name Inspectors
        //@{
        const Handle<DefaultProbabilityTermStructure>&
            defaultProbability(const DefaultProbKey& key) const;

        //@}

        //! \name Utilities
        //@{
        //! If a default event with the required seniority and
        //    restructuring type is found, it is returned for
        //    inspection; otherwise, the method returns an empty pointer.
        ext::shared_ptr<DefaultEvent>
        defaultedBetween(const Date& start,
                         const Date& end,
                         const DefaultProbKey& key,
                         bool includeRefDate = false
                         ) const;

        //@}
        std::vector<ext::shared_ptr<DefaultEvent> >
        defaultsBetween(const Date& start,
                        const Date& end,
                        const DefaultProbKey& contractKey,
                        bool includeRefDate
                        ) const ;
      private:
        //! probabilities of events for each bond collection
        // vector of pairs preferred over maps for performance
        std::vector<std::pair<DefaultProbKey,
            Handle<DefaultProbabilityTermStructure> > > probabilities_;
        //! History of past events affecting this issuer. Notice it is possible
        //    for the same event to occur on the same bond several times along
        //    time.
        DefaultEventSet events_;
    };

}

#endif
