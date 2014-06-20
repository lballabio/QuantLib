/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2014 Jose Aparicio

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

#ifndef quantlib_defaultlossmodel_hpp
#define quantlib_defaultlossmodel_hpp

#include <ql/instruments/claim.hpp>
#include <ql/experimental/credit/defaultprobabilitykey.hpp>
#include <ql/utilities/disposable.hpp>


/* Intended to replace LossDistribution in 
    ql/experimental/credit/lossdistribution, not sure its covering all the 
    functionality
*/

namespace QuantLib {

    class Basket;


    /*!
        - A concrete default model must provide a (joint or single) recovery rate implementation.
        - As it is now the clients have to remember to call 'calculate' at the begining of each functionality method. To avoid this split the work to a delegated virtual ---Impl private method. Drop the Lazyness? Is it that expensive to comp the current losses and basket and perf this in an update???

        -> Leaving the reference date of the basket open means passing it to the basket or to the model on every request. The upside is that
        the same basket or loss model objects can be associated with/to several models/baskets respectively. The downside is that, leaving
        the date open, we precise to compute the remaining basket at the beginning of every call to methods in the loss models.  It remains possible
        though to wrap the models with a reference date member and cache the remaining basket. This is why the method that computes the basket could be 
        left virtual. Methods in the derived class will have one less argument though, so one would need the type of the object. I leave this to be solved later on.
    */
    // Lazy for the current losses
   /// class DefaultLossModel : public LazyObject {
    class DefaultLossModel : public virtual Observer, 
                             public virtual Observable {// current losses now delegated in the Basket which access the loss model...
    public:
        /*!
            @param refDate Reference date at which defaults are relevant to the basket.
        */
        /* The basket ref date plays an argument role and it is arguable 
        for it to be a basket property
        */
        DefaultLossModel() { }

        void update() {notifyObservers();}


        //! \name Statistics
        //@{
        // Non mandatory implementations.

        /*! Expected amount lost due to default events (expressed in portfolio 
        currency units) at the requested date in the currently live portfolio. 
        Only the contingent amount is returned, this is, not included of the 
        losses from realized defaults. */
        ////////////////////////virtual Real expectedLoss(const Date&) const {
        ////////////////////////    QL_FAIL("expectedLoss Not implemented for this type.");
        ////////////////////////}
        /* Default implementation using the expectedLoss(Date) method. 
          Typically this method is called repeatedly with the same 
          date parameter which makes it innefficient. */
        virtual Real expectedTrancheLoss(const Date& d) const {
            QL_FAIL("expectedTrancheLoss Not implemented for this model.");
        }
        /* Other methods might be here or in derived classes. The interface is
        set up here to homogeneize the calls. */

        /*! Probability of the tranche losing the same or more than the 
          fractional amount given.

            @param lossFraction A fraction of losses over the tranche 
                                notional (not the portfolio)
        */
        virtual Probability probOverLoss(
            const Date& d, Real lossFraction) const {
            QL_FAIL("probOverLoss Not implemented for this model.");   
        }
        //! Value at Risk given a default loss percentile.
        virtual Real percentile(const Date& d, Real percentile) const {
            QL_FAIL("percentile Not implemented for this model.");   
        }
        //! Expected shortfall given a default loss percentile.
        virtual Real expectedShortfall(const Date& d, Real percentile) const {
            QL_FAIL("eSF Not implemented for this model.");   
        }
        //! Associated VaR fraction to each name.
        virtual Disposable<std::vector<Real> > 
            splitLossLevel(const Date& d, Real loss) const {
            QL_FAIL("fractionVaR Not implemented for this model.");   
        }
        //! Full loss distribution.
        virtual Disposable<std::map<Real, Probability> > 
            lossDistribution(const Date&) const {
            QL_FAIL("lossDistribution Not implemented for this model.");   
        }
        //! Probability density of a given loss fraction of the basket notional.
        virtual Real densityTrancheLoss(
            const Date& d, Real lossFraction) const {
            QL_FAIL("densityTrancheLoss Not implemented for this model.");
        }
        //!
        /*! Probabilities for each of the (remaining) basket elements in the 
        pool to have defaulted by time d and at the same time be the Nth 
        defaulting name to default in the basket. This method is oriented to 
        default order dependent portfolio pricing (e.g. NTDs)
            The the probabilities ordering in the vector coincides with the 
            pool order.
        */
        virtual Disposable<std::vector<Probability> > probsBeingNthEvent(
            Size n, const Date& d) const {
            QL_FAIL("probsBeingNthEvent Not implemented for this model.");
        }
        //! Pearsons' default probability correlation. 
        virtual Real defaultCorrelation(const Date& d, Size iName, 
            Size jName) const {
            QL_FAIL("defaultCorrelation Not implemented for this model.");
        }
        /*! Returns the probaility of having a given or larger number of 
        defaults in the basket portfolio at a given time.
        */
        virtual Probability probAtLeastNEvents(Size n, const Date& d) const {
            QL_FAIL("probAtLeastNEvents Not implemented for this model.");
        }

        //@}

        // VECTOR VERSION IN THE POOL ORDERING>>?????
        /*! To be delegated to a concrete RR Model (BasketConstantRRModel) 
        implemented in the concrete Default Loss Model. Derived classes should 
        register with the RR models for this class to perform its updates. */
        // An alternative is for this class to own a ptr to an abstract RR model and register with it.....
        //// There are several classes derived from this one not really knowing what to do with this one.......
        virtual Real recoveryValueImpl(const Date& defaultDate, Size iName, //const std::string& name <- from the Pool
            const std::vector<DefaultProbKey>& defKeys = std::vector<DefaultProbKey>()) const = 0;

        /* Each subclass should as it fits and needs, some might need to reference somehow the basket, others retain 
        locally some information about the basket. It should be as light as possible since it is called on every basket 
        request of model dependent information.

        Initialize gets call on model to basket asignment and on basket observability updates. Depending on the complexity of the initialize implementation in a particular model (or how much information from the basket it caches locally) it might trigger basket recalculation. Essentially the basket takes care of re-initializing when needed; if the loss model registers with its own magnitudes (e.g. correlation) it has to do its own work and taking any measures so that the post-initialization state is correct (this is the tricky part)

-> it might be possible that some models do nothing

        */
        //resets basket, which is treated by the model as an argument (theres a cyclic refeernce though)
     ///////////////////////////////////////////////////////////////   virtual void initialize(const Basket& basket) = 0;
        virtual void setupBasket(const boost::shared_ptr<Basket>& basket) = 0;
    };

}

#endif
