/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
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

/*! \file basket.hpp
    \brief basket of issuers and related notionals
*/

#ifndef quantlib_basket_hpp
#define quantlib_basket_hpp

#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/experimental/credit/defaultprobabilitykey.hpp>
#include <ql/experimental/credit/issuer.hpp>
#include <ql/experimental/credit/recoveryratemodel.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/experimental/credit/loss.hpp>

namespace QuantLib {

    /*! Credit Basket.

        A basket is a collection of credit names, represented by a
        unique identifier (a text string), associated notional
        amounts, a pool and tranche information. The pool is a map of
        "names" to issuers.  The Basket structure is motivated by CDO
        squared instruments containing various underlying inner CDOs
        which can be represented by respective baskets including their
        tranche structure.  The role of the Pool is providing a unique
        list of relevant issuers while names may appear multiple times
        across different baskets (overlap).
     */
    class Basket : public LazyObject {
      public:
        Basket() {}
        Basket(const std::vector<std::string>& names,
               const std::vector<Real>& notionals,
               const boost::shared_ptr<Pool> pool,
               const std::vector<DefaultProbKey>& defaultKeys,
               const std::vector<boost::shared_ptr<RecoveryRateModel> >& rrModels,
               Real attachmentRatio = 0.0,
               Real detachmentRatio = 1.0);

        Size size() const;

        const std::vector<std::string>& names() const;

        const std::vector<Real>& notionals() const;
        Real notional();

        boost::shared_ptr<Pool> pool() const;

        const std::vector<DefaultProbKey>& defaultKeys() const;
        const std::vector<boost::shared_ptr<RecoveryRateModel> >&
            recoveryModels() const;

        /*! Loss Given Default for all issuers/notionals based on
            expected recovery rates for the respective issuers.
        */
        const std::vector<Real>& LGDs() const;
        Real lgd();

        //! Attachment point expressed as a fraction of the total pool notional.
        Real attachmentRatio() const;
        //! Detachment point expressed as a fraction of the total pool notional.
        Real detachmentRatio() const;
        //! Original basket notional ignoring any losses.
        Real basketNotional() const;
        //! Original expected basket LGD.
        Real basketLGD() const;
        //! Original tranche notional ignoring any losses.
        Real trancheNotional() const;
        //! Attachment amount = attachmentRatio() * basketNotional()
        Real attachmentAmount() const;
        //! Detachment amount = detachmentRatio() * basketNotional()
        Real detachmentAmount() const;
        /*! Vector of cumulative default probability to date d for al
            issuers in the basket.
        */
        std::vector<Real> probabilities(const Date& d) const;
        /*! Actual basket losses between start and end date, taking
            the actual recovery rates of loss events into account.
        */
        Real cumulatedLoss(const Date& startDate,
                           const Date& endDate) const;
        /*! Remaining basket notional after losses between start and
            end date.  The full notional for defaulted names is
            subracted, recovery ignored.
        */
        Real remainingNotional(const Date& startDate,
                               const Date& endDate) const;
        /*! Vector of surviving notionals after losses between start
            and end date, recovery ignored.
        */
        std::vector<Real> remainingNotionals(const Date& startDate,
                                             const Date& endDate) const;
        /*! Vector of surviving issuers after defaults between start
            and end date.
        */
        std::vector<std::string> remainingNames(const Date& startDate,
                                                const Date& endDate) const;


        std::vector<DefaultProbKey>
            remainingDefaultKeys(const Date& startDate,
                                          const Date& endDate) const;
        std::vector<boost::shared_ptr<RecoveryRateModel> >
            remainingRecModels(const Date& startDate,
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
          Based on the default times stored in the Pool for each name, update
          the vector of incremental basket losses (sorted by default time)
          for this basket. If zeroRecovery is set to true, losses are full
          notional amounts, otherwise loss give defaults.
         */
        void updateScenarioLoss(bool zeroRecovery = false);
        //! Cumulative tranche loss up to end date under the current scenario
        Real scenarioTrancheLoss(Date endDate) const;
        //! Vector of incremental basket losses under the current scenario
        std::vector<Loss> scenarioIncrementalBasketLosses() const;
        //! Vector of incremental tranche losses under the current scenario
        std::vector<Loss> scenarioIncrementalTrancheLosses(
                                Date startDate = Date::minDate(),
                                Date endDate = Date::maxDate()) const;
      private:
        // LazyObject interface
         void performCalculations() const;

        std::vector<std::string> names_;
        std::vector<Real> notionals_;
        boost::shared_ptr<Pool> pool_;
        // seniorities, events, currencies for which each name
        // enters the contract
        std::vector<DefaultProbKey> defaultKeys_;
        // rr models for each name, each one points to the names RR quote
        std::vector<boost::shared_ptr<RecoveryRateModel> > rrModels_;
        Real attachmentRatio_;
        Real detachmentRatio_;
        Real basketNotional_;
        mutable Real basketLGD_;
        Real trancheNotional_;
        Real attachmentAmount_;
        Real detachmentAmount_;
        //! Individual names expected LGDs at the reference date.
        mutable std::vector<Real> LGDs_;
        std::vector<Loss> scenarioLoss_;
    };

}


#endif
