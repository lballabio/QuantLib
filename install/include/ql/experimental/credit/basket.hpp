/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009, 2014 Jose Aparicio

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

#include <ql/instruments/claim.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/experimental/credit/defaultprobabilitykey.hpp>
#include <ql/experimental/credit/issuer.hpp>
#include <ql/experimental/credit/recoveryratemodel.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/experimental/credit/loss.hpp>

namespace QuantLib {

    class DefaultLossModel;

    /*! Credit Basket.\par
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
        Basket() = default;
        /*! Constructs a basket of simple collection of constant notional 
          positions subject to default risk only.
          
          The refDate parameter is the basket inception date, that is,
          the date at which defaultable events are relevant. (There
          are no constraints on forward baskets but models assigned
          should be consistent.)
        */
        Basket(const Date& refDate,
               const std::vector<std::string>& names,
               std::vector<Real> notionals,
               ext::shared_ptr<Pool> pool,
               Real attachmentRatio = 0.0,
               Real detachmentRatio = 1.0,
               ext::shared_ptr<Claim> claim = ext::shared_ptr<Claim>(new FaceValueClaim()));
        void update() override {
            computeBasket();
            LazyObject::update();
        }
        void computeBasket() const {
            Date today = Settings::instance().evaluationDate();
            /* update cache values at the calculation date (work as arguments 
              to the Loss Models)
            \to do: IMPORTANT: notice that defaults added to Issuers dont get
            notify as the codes stnds today. Issuers need to be observables.
            */
            //this one must remain on top since there are dependencies
            evalDateLiveKeys_      = remainingDefaultKeys(today);
            evalDateSettledLoss_   = settledLoss(today);
            evalDateRemainingNot_  = remainingNotional(today);
            evalDateLiveNotionals_ = remainingNotionals(today);
            evalDateLiveNames_     = remainingNames(today);
            evalDateAttachAmount_  = remainingAttachmentAmount(today);
            evalDateDetachAmmount_ = 
                remainingDetachmentAmount(today);
            evalDateLiveList_ = liveList(today);
        }
        //! Basket inception number of counterparties.
        Size size() const;
        //! Basket counterparties names at inception.
        const std::vector<std::string>& names() const {return pool_->names();}
        //! Basket counterparties notionals at inception.
        const std::vector<Real>& notionals() const;
        //! Basket total notional at inception.
        Real notional() const;
        //! Returns the total expected exposures for that name.
        Real exposure(const std::string& name, const Date& = Date()) const;
        //! Underlying pool
        const ext::shared_ptr<Pool>& pool() const;
        //! The keys each counterparty enters the basket with (sensitive to)
        std::vector<DefaultProbKey> defaultKeys() const;
        /*! Loss Given Default for all issuers/notionals based on
            expected recovery rates for the respective issuers.
        */
        //! Basket inception date.
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
        //! default claim, same for all positions and counterparties
        ext::shared_ptr<Claim> claim() const {return claim_;}
        /*! Vector of cumulative default probability to date d for all
            issuers in the basket.
        */
        std::vector<Probability> probabilities(const Date& d) const;
        /*! Realized basket losses between the reference date and the 
            calculation date, taking the actual recovery rates of loss events 
            into account. 
            Only default events that have settled (have a realized RR) are 
            accounted for. For contingent losses after a default you need
            to compute the losses through a DefaultLossModel

            Optionally one can pass a date in the future and that will collect 
            events stored in the issuers list. This shows the effect of 
            'programmed' (after today's) events on top of past ones. The 
            intention is to be used in risk analysis (jump to default, etc).
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
        std::vector<Real> remainingNotionals(const Date&) const;
        /*! Vector of surviving issuers after defaults between the reference 
          basket date and the given (or evaluation) date.
        */
        const std::vector<std::string>& remainingNames() const;
        std::vector<std::string> remainingNames(const Date&) const;
        /*! Default keys of non defaulted counterparties
        */
        const std::vector<DefaultProbKey>& remainingDefaultKeys() const;
        std::vector<DefaultProbKey> remainingDefaultKeys(const Date&) const;
        //! Number of counterparties alive on the requested date.
        Size remainingSize() const;
        Size remainingSize(const Date&) const;
        /*! Vector of cumulative default probability to date d for all
            issuers still (at the evaluation date) alive in the basket.
        */
        std::vector<Probability> remainingProbabilities(const Date& d) const;
        /*!
          Attachment amount of the equivalent (after defaults) remaining basket
          The remaining attachment amount is
          RAA = max (0, attachmentAmount - cumulatedLoss())

          The remaining attachment ratio is then
          RAR = RAA / remainingNotional()
        */
        Real remainingAttachmentAmount() const;
        Real remainingAttachmentAmount(const Date& endDate) const;

        /*!
          Detachment amount of the equivalent remaining basket.
          The remaining detachment amount is
          RDA = max (0, detachmentAmount - cumulatedLoss())

          The remaining detachment ratio is then
          RDR = RDA / remainingNotional()
        */
        Real remainingDetachmentAmount() const;
        Real remainingDetachmentAmount(const Date& endDate) const;

        //! Remaining basket tranched notional on calculation date
        Real remainingTrancheNotional() const {
            calculate();
            return evalDateDetachAmmount_ - evalDateAttachAmount_;
        }
        /*! Expected basket tranched notional on the requested date
            according to the basket model. Model should have been assigned.
        */
        Real remainingTrancheNotional(const Date& endDate) const {
            calculate();
            return remainingDetachmentAmount(endDate) - 
                remainingAttachmentAmount(endDate);
        }
        //!Indexes of remaining names. Notice these are names and not positions.
        const std::vector<Size>& liveList() const;
        std::vector<Size> liveList(const Date&) const;//?? keep?
        //! Assigns the default loss model to this basket. Resets calculations.
        void setLossModel(
            const ext::shared_ptr<DefaultLossModel>& lossModel);
        /*! \name Basket Loss Statistics
            Methods providing statistical metrics on the loss or value 
            distribution of the basket. Most calculations rely on the pressence
            of a model assigned to the basket.
        */
        //@{
        Real expectedTrancheLoss(const Date& d) const;
        /*! The lossFraction is the fraction of losses expressed in 
            inception (no losses) tranche units (e.g. 'attach level'=0%, 
            'detach level'=100%)
        */
        Probability probOverLoss(const Date& d, Real lossFraction) const;
        /*! 
        */
        Real percentile(const Date& d, Probability prob) const;
        /*! ESF 
        */
        Real expectedShortfall(const Date& d, Probability prob) const;
        /* Split a portfolio loss along counterparties. Typically loss 
        corresponds to some percentile.*/
        std::vector<Real> splitVaRLevel(const Date& date, Real loss) const;
        /*! Full loss distribution
        */
        std::map<Real, Probability> lossDistribution(const Date&) const;
        Real densityTrancheLoss(const Date& d, Real lossFraction) const;
        Real defaultCorrelation(const Date& d, Size iName, Size jName) const;
        /*! Probability vector that each of the remaining live names (at eval
          date) is the n-th default by date d.

          The n parameter is the internal index to the name; it should
          be alive at the evaluation date.

        ---------TO DO: Implement with a string passed----------------------
        ---------TO DO: Perform check the name is alive---------------------
        */
        std::vector<Probability> probsBeingNthEvent(
            Size n, const Date& d) const;
        /*! Returns the probaility of having a given or larger number of 
        defaults in the basket portfolio at a given time.
        */
        Probability probAtLeastNEvents(Size n, const Date& d) const;
        /*! Expected recovery rate of the underlying position as a fraction of 
          its exposure value at date d _given_ it has defaulted _on_ that date.
          NOTICE THE ARG IS THE CTPTY....SHOULDNT IT BE THE POSITION/INSTRUMENT?????<<<<<<<<<<<<<<<<<<<<<<<
        */
        Real recoveryRate(const Date& d, Size iName) const;
        //@}
      private:
        // LazyObject interface
        void performCalculations() const override;

        std::vector<Real> notionals_;
        ext::shared_ptr<Pool> pool_;
        //! The claim is the same for all names
        const ext::shared_ptr<Claim> claim_;

        Real attachmentRatio_;
        Real detachmentRatio_;
        Real basketNotional_;
        //! basket tranched inception attachment amount:
        mutable Real attachmentAmount_;
        //! basket tranched inception detachment amount:
        mutable Real detachmentAmount_;
        //! basket tranched notional amount:
        mutable Real trancheNotional_;
        /* Caches. Most of the times one wants statistics on the distribution of
        futures losses at arbitrary dates but some problems (e.g. derivatives 
        pricing) work with todays (evalDate) magnitudes which do not require a 
        loss model and would be too expensive to recompute on every call.
        */
        mutable Real evalDateSettledLoss_,
            evalDateRemainingNot_,
            evalDateAttachAmount_,
            evalDateDetachAmmount_;
        mutable std::vector<Size> evalDateLiveList_;
        mutable std::vector<Real> evalDateLiveNotionals_;
        mutable std::vector<std::string> evalDateLiveNames_;
        mutable std::vector<DefaultProbKey> evalDateLiveKeys_;
        //! Basket inception date.
        const Date refDate_;
        /* It is the basket responsibility to ensure that the model assigned it 
          is properly initialized to the basket current data. 
          This might not be the case for various reasons: the basket data might
          have been updated, the evaluation date has changed or the model has 
          received another request from another basket pointing to it. For
          this last reason we can never be sure between calls that this is the 
          case (and that is true in a single thread environment only).
        */
        ext::shared_ptr<DefaultLossModel> lossModel_;
    };

    // ------------ Inlines -------------------------------------------------

    inline Size Basket::size() const {
        return pool_->size();
    }

    inline const std::vector<Real>& Basket::notionals() const {
        return notionals_;
    }

    inline std::vector<DefaultProbKey> Basket::defaultKeys() const {
        return pool_->defaultKeys();
    }

    inline const ext::shared_ptr<Pool>& Basket::pool() const {
        return pool_;
    }

    inline const std::vector<Size>& Basket::liveList() const {
        return evalDateLiveList_;
    }

    inline Real Basket::remainingDetachmentAmount() const {
        return evalDateDetachAmmount_;
    }

    inline Real Basket::remainingAttachmentAmount() const {
        return evalDateAttachAmount_;
    }

    inline const std::vector<std::string>& Basket::remainingNames() const {
        return evalDateLiveNames_;
    }

    inline const std::vector<Real>& Basket::remainingNotionals() const {
        return evalDateLiveNotionals_;
    }

    inline Real Basket::cumulatedLoss() const {
        return this->evalDateSettledLoss_;
    }
    
    inline Real Basket::settledLoss() const {
        return evalDateSettledLoss_;
    }

    inline const std::vector<DefaultProbKey>& 
        Basket::remainingDefaultKeys() const 
    {
        return evalDateLiveKeys_;
    }

}


#endif
