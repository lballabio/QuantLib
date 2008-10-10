/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file basket.hpp
    \brief basket of issuers and related notionals
*/

#ifndef quantlib_basket_hpp
#define quantlib_basket_hpp

#include <ql/default.hpp>
#include <ql/issuer.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/experimental/credit/loss.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    /*! Credit Basket
      A basket is a collection of credit names, represented by a unique
      identifier (a text string), associated notional amounts, a pool and
      tranche information. The pool is a map of "names" to issuers.
      The Basket structure is motivated by CDO squared instruments containing
      various underlying inner CDOs which can be represented by respective
      baskets including their tranche structure.
      The role of the Pool is providing a unique list of relevant issuers
      while names may appear multiple times across different baskets (overlap).

      \ingroup credit
     */
    class Basket {
      public:
        Basket() {}
        Basket(const std::vector<std::string>& names,
               const std::vector<Real>& notionals,
               const boost::shared_ptr<Pool> pool,
               Real attachmentRatio = 0.0,
               Real detachmentRatio = 1.0);

        Size size() const;

        const std::vector<std::string>& names() const;

        const std::vector<Real>& notionals() const;

        boost::shared_ptr<Pool> pool() const;
        /*!
          Loss Given Default for all issuers/notionals based on expected
          recovery rates for the respective issuers.
         */
        const std::vector<Real>& LGDs() const;
        /*!
          Attachment point expressed as a fraction of the total pool notional.
         */
        Real attachmentRatio() const;
        /*!
          Detachment point expressed as a fraction of the total pool notional.
         */
        Real detachmentRatio() const;
        /*!
          Original basket notional ignoring any losses.
        */
        Real basketNotional() const;
        /*!
          Original tranche notional ignoring any losses.
        */
        Real trancheNotional() const;
        /*!
          Attachment amount = attachmentRatio() * basketNotional()
         */
        Real attachmentAmount() const;
        /*!
          Detachment amount = detachmentRatio() * basketNotional()
         */
        Real detachmentAmount() const;
        /*!
          Vector of cumulative default probability to date d for al issuers in
          the basket.
         */
        std::vector<Real> probabilities(const Date& d) const;
        /*!
          Actual basket losses between start and end date, taking the actual
          recovery rates of loss events into account.
        */
        Real cumulatedLoss(const Date& startDate,
                           const Date& endDate) const;
        /*!
          Remaining basket notional after losses between start and end date.
          The full notional for defaulted names is subracted, recovery ignored.
         */
        Real remainingNotional(const Date& startDate,
                               const Date& endDate) const;
        /*!
          Vector of surviving notionals after losses between start and end date,
          recovery ignored.
         */
        std::vector<Real> remainingNotionals(const Date& startDate,
                                             const Date& endDate) const;
        /*!
          Vector of surviving issuers after defaults between start and end date.
         */
        std::vector<std::string> remainingNames(const Date& startDate,
                                                const Date& endDate) const;
        /*!
          The remaining attachment amount is
          RAA = max (0, attachmentAmount - cumulatedLoss())

          The remaining attachment ratio is then
          RAR = RAA / remainingNotional()
         */
        Real remainingAttachmentRatio(const Date& startDate,
                                      const Date& endDate) const;
        Real remainingAttachmentAmount(const Date& startDate,
                                       const Date& endDate) const;
        /*!
          The remaining detachment amount is
          RDA = max (0, detachmentAmount - cumulatedLoss())

          The remaining detachment ratio is then
          RDR = RDA / remainingNotional()
         */
        Real remainingDetachmentRatio(const Date& startDate,
                                      const Date& endDate) const;
        Real remainingDetachmentAmount(const Date& startDate,
                                       const Date& endDate) const;

        /*!
          Based on the default times stored in the Pool for each name, return
          the related tranche loss between start and end date for this basket.
          Names with actual default events between effective and start date are
          ignored.
         */
        Real scenarioTrancheLoss(Date endDate) const;
        void updateScenarioLoss(Date startDate, Date endDate);
        std::vector<Loss> scenarioBasketLosses() const;
        std::vector<Loss> scenarioIncrementalTrancheLosses(Date startDate,
                                                           Date endDate) const;
      private:
        std::vector<std::string> names_;
        std::vector<Real> notionals_;
        boost::shared_ptr<Pool> pool_;
        Real attachmentRatio_;
        Real detachmentRatio_;
        Real basketNotional_;
        Real trancheNotional_;
        Real attachmentAmount_;
        Real detachmentAmount_;
        std::vector<Real> LGDs_;
        std::vector<Loss> scenarioLoss_;
    };

}


#endif
