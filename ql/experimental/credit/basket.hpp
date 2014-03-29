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
#include <ql/utilities/disposable.hpp>

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
        /*! Constructs a basket of simple collection of constant notional 
          positions subject to default risk only.
          @param refDate Basket inception date. Date at which defaultable 
                events are relevant. (There are no constraints on forward
                baskets but models assigned should be consistent.)
        */
        Basket(
            const Date& refDate,
            const std::vector<std::string>& names,
               const std::vector<Real>& notionals,
               const boost::shared_ptr<Pool> pool,
               const std::vector<boost::shared_ptr<RecoveryRateModel> >& rrModels,
               Real attachmentRatio = 0.0,
               Real detachmentRatio = 1.0);

        void update() {LazyObject::update();}///............??????????????init losses here

        Size size() const;

        const std::vector<std::string>& names() const {return pool_->names();}

        const std::vector<Real>& notionals() const;
        Real notional();

        boost::shared_ptr<Pool> pool() const;

        Disposable<std::vector<DefaultProbKey> > defaultKeys() const;
        const std::vector<boost::shared_ptr<RecoveryRateModel> >&
            recoveryModels() const;

        /*! Loss Given Default for all issuers/notionals based on
            expected recovery rates for the respective issuers.
        */
        const std::vector<Real>& LGDs() const;
        Real lgd();

        const Date& refDate() const {return refDate_;}
        /*! Attachment point expressed as a fraction of the total inception 
          notional.
        */
        Real attachmentRatio() const {return attachmentRatio_;}
        //! Detachment point expressed as a fraction of the total pool notional
        Real detachmentRatio() const {return detachmentRatio_;}
        //! Original basket notional ignoring any losses.
        Real basketNotional() const {return basketNotional_;}
        //! Original tranche notional ignoring any realized losses.
        Real trancheNotional() const {return trancheNotional_;}
        //! Attachment amount = attachmentRatio() * basketNotional()
        Real attachmentAmount() const {return attachmentAmount_;}
        //! Detachment amount = detachmentRatio() * basketNotional()
        Real detachmentAmount() const {return detachmentAmount_;}
        /*! Vector of cumulative default probability to date d for all
            issuers in the basket.
        */
        Disposable<std::vector<Probability> > 
            probabilities(const Date& d) const;

        /*! Realized basket losses between the reference date and the 
            calculation date, taking the actual recovery rates of loss events 
            into account. 
            Only default events that have settled (have a realized RR) are 
            accounted for. For contingent losses after a default you need
            to compute the losses through a DefaultLossModel

            Optionally one can pass a date in the future and that will collect 
            events stored in the issuers list. This shows the effect of 
            'programmed' (after today's) events on top of past ones. The 
            intention is to be used in riak analysis (jump to default, etc).
        */
        Real settledLoss() const;
        Real settledLoss(const Date&) const;
        /*! Actual basket losses between the reference date and the calculation
            date, taking the actual recovery rates of loss events into account.
            If the event has not settled yet a model driven recovery is used.

            Returns the realized losses in this portfolio since the portfolio
            default reference date.
            This method relies on an implementation of the loss given default 
            since the events have not necessarily settled.
        */
        Real cumulatedLoss() const;
        Real cumulatedLoss(const Date&) const;

        // deprecated::----
        Real cumulatedLoss(const Date& d1, const Date& d2) const {return cumulatedLoss(d2);}

        /*! Remaining full basket (untranched) notional after settled losses 
          between the reference date and the given date.  The full notional 
          for defaulted names is subracted, recovery ignored.
        */
        Real remainingNotional() const;
        Real remainingNotional(const Date&) const;
        /*! Vector of surviving notionals after settled losses between the 
          reference date and the given date, recovery ignored.
        */
        const std::vector<Real>& remainingNotionals() const;
        Disposable<std::vector<Real> > remainingNotionals(const Date&) const;

        // deprecated::----
        Disposable<std::vector<Real> >  remainingNotionals(const Date& d1, const Date& d2) const {return remainingNotionals(d2);}

        /*! Vector of surviving issuers after defaults between the reference 
          basket date and the given (or evaluation) date.
        */
        const std::vector<std::string>& remainingNames() const;
        Disposable<std::vector<std::string> > 
            remainingNames(const Date&) const;

        // deprecated::----
        Disposable<std::vector<std::string> >  remainingNames(const Date& d1, const Date& d2) const {return remainingNames(d2);}

        /*!
        */
        const std::vector<DefaultProbKey>& remainingDefaultKeys() const;
        Disposable<std::vector<DefaultProbKey> > remainingDefaultKeys(
            const Date&) const;

        // deprecated::----
        Disposable<std::vector<DefaultProbKey> >  remainingDefaultKeys(const Date& d1, const Date& d2) const {return remainingDefaultKeys(d2);}

        //! Number of counterparties alive on the requested date.
        Size remainingSize() const;
        Size remainingSize(const Date&) const;
        /*! Vector of cumulative default probability to date d for all
            issuers still (at the evaluation date) alive in the basket.
        */
        Disposable<std::vector<Probability> > 
            remainingProbabilities(const Date& d) const;


        std::vector<boost::shared_ptr<RecoveryRateModel> >
            remainingRecModels(const Date& startDate,
                               const Date& endDate) const;
        /*!
          Attachment amount of the equivalent (after defaults) remaining basket
          The remaining attachment amount is
          RAA = max (0, attachmentAmount - cumulatedLoss())

          The remaining attachment ratio is then
          RAR = RAA / remainingNotional()
        */
        // REMOVED THE RATIOS NOW. MEANING NOT CLEAR WITH VARIABLE STRUCTURE SIZE.........
        //////////////////Real remainingAttachmentRatio() const;
        //////////////////Real remainingAttachmentRatio(const Date&) const;
        Real remainingAttachmentAmount() const;
        Real remainingAttachmentAmount(const Date& endDate) const;

        // deprecated::----
        Real remainingAttachmentAmount(const Date& d1, const Date& d2) const {return remainingAttachmentAmount(d2);}
        // deprecated::----
        Real remainingAttachmentRatio(const Date& startDate , const Date& endDate) const {return remainingAttachmentAmount(endDate)
            / remainingNotional(endDate);} 

        /*!
          Detachment amount of the equivalent remaining basket.
          The remaining detachment amount is
          RDA = max (0, detachmentAmount - cumulatedLoss())

          The remaining detachment ratio is then
          RDR = RDA / remainingNotional()
        */
        // REMOVED THE RATIOS NOW. MEANING NOT CLEAR WITH VARIABLE STRUCTURE SIZE.........
        //////////////////Real remainingDetachmentRatio() const;
        //////////////////Real remainingDetachmentRatio(const Date& endDate) const;
        Real remainingDetachmentAmount() const;
        Real remainingDetachmentAmount(const Date& endDate) const;

        // deprecated::----
        Real remainingDetachmentAmount(const Date& d1, const Date& d2) const {return remainingDetachmentAmount(d2);}
        // deprecated::----
        Real remainingDetachmentRatio(const Date& startDate , const Date& endDate) const {return remainingDetachmentAmount(endDate)
            / remainingNotional(endDate);} 


        Real remainingTrancheNotional() const {
            calculate();
            return evalDateDetachAmmount_ -
                evalDateAttachAmount_;
        }
        Real remainingTrancheNotional(const Date& endDate) const {
            calculate();
            return remainingDetachmentAmount(endDate) - 
                remainingAttachmentAmount(endDate);
        }

        //! Indexes of remaining names. Notice these are names and not positions.
        const std::vector<Size>& liveList() const;
        Disposable<std::vector<Size> > liveList(const Date&) const;


        /* The problem with this one is that this depends on the model, but not 
        some magnitudes, this makes them artificially dependent on the model
        */
        // Basket remainingBasket() const ;


        //! Assigns the loss model to this basket. Resets calculations.
        ////////////////////////////////void setLossModel(
        ////////////////////////////////    const boost::shared_ptr<DefaultLossModel>& lossModel);


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

        const std::vector<std::string> names_;///why?????????????  its in the pool-------------------
        std::vector<Real> notionals_;//// not in new version----------------------
        boost::shared_ptr<Pool> pool_;
        // rr models for each name, each one points to the names RR quote
        std::vector<boost::shared_ptr<RecoveryRateModel> > rrModels_;//// not in new version---------------------
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

        /* Most of the times one wants statistics on the distribution of 
        futures losses at arbitrary dates but some problems (e.g. derivatives 
        pricing) work with todays (evalDate) magnitudes which do not require a 
        loss model and would be too expensive to recompute on every call.
        */
        mutable Real evalDateSettledLoss_,
            evalDateCumulContingentLoss_,
            evalDateRemainingNot_,
            evalDateAttachRatio_,
            evalDateAttachAmount_,
            evalDateDetachRatio_,
            evalDateDetachAmmount_;
        mutable std::vector<Size> evalDateLiveList_;
        mutable std::vector<Real> evalDateLiveNotionals_;
        mutable std::vector<std::string> evalDateLiveNames_;
        mutable std::vector<DefaultProbKey> evalDateLiveKeys_;

        const Date refDate_;
    };


    inline const std::vector<Size>& Basket::liveList() const {
        calculate();
        return evalDateLiveList_;
    }

    inline Real Basket::remainingDetachmentAmount() const {
        calculate();
        return evalDateDetachAmmount_;
    }

    inline Real Basket::remainingAttachmentAmount() const {
        calculate();
        return evalDateAttachAmount_;
    }

    inline const std::vector<std::string>& Basket::remainingNames() const {
        calculate();
        return evalDateLiveNames_;
    }

    inline const std::vector<Real>& Basket::remainingNotionals() const {
        calculate();
        return evalDateLiveNotionals_;
    }

    inline Real Basket::settledLoss() const {
        calculate();
        return evalDateSettledLoss_;
    }

    inline const std::vector<DefaultProbKey>& 
        Basket::remainingDefaultKeys() const 
    {
        calculate();
        return evalDateLiveKeys_;
    }

}


#endif
