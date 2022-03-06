/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl
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

/*! \file defaultprobabilitykey.hpp
    \brief Classes for default-event description.
*/

#ifndef quantlib_default_probability_key_hpp
#define quantlib_default_probability_key_hpp

#include <ql/experimental/credit/defaulttype.hpp>
#include <ql/currency.hpp>
#include <vector>

namespace QuantLib {

    /*! Used to index market implied credit curve probabilities. It is
        a proxy to the defaultable bond or class of bonds which
        determines the credit contract conditions.  It aggregates the
        atomic default types in a group defining the contract
        conditions and which serves to index the probability curves
        calibrated to the market.
    */
    class DefaultProbKey {
      protected:
        //! aggregation of event types for which the contract is sensitive.
        std::vector<ext::shared_ptr<DefaultType> > eventTypes_;
        //! Currency of the bond and protection leg payment.
        Currency obligationCurrency_;
        //! Reference bonds seniority.
        Seniority seniority_ = NoSeniority;

      public:
        DefaultProbKey();

        DefaultProbKey(std::vector<ext::shared_ptr<DefaultType> > eventTypes,
                       Currency cur,
                       Seniority sen);

        const Currency& currency() const {return obligationCurrency_;}
        Seniority seniority() const {return seniority_;}
        const std::vector<ext::shared_ptr<DefaultType> >&
            eventTypes() const {
                return eventTypes_;
        }
        Size size() const {return eventTypes_.size();}
    };

    bool operator==(const DefaultProbKey& lhs, const DefaultProbKey& rhs);


    //! ISDA standard default contractual key for corporate US debt.
    //    Restructuring here can be set to NoRestructuring.
    class NorthAmericaCorpDefaultKey : public DefaultProbKey {
      public:
        // with only one restructuring type
        NorthAmericaCorpDefaultKey(const Currency& currency,
                                   Seniority sen,
                                   Period graceFailureToPay =
                                       Period(30, Days),
                                   Real amountFailure = 1.e6,
                                   Restructuring::Type resType =
                                                           Restructuring::CR);
    };

}

#endif


#ifndef id_d6a435d5eb703c671c1484a3a69857a3
#define id_d6a435d5eb703c671c1484a3a69857a3
inline bool test_d6a435d5eb703c671c1484a3a69857a3(int* i) { return i != 0; }
#endif
