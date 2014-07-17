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

#include <boost/make_shared.hpp>

#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/loss.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>

using namespace std;

namespace QuantLib {

    Basket::Basket(const Date& refDate,
        const vector<string>& names,
        const vector<Real>& notionals,
        const boost::shared_ptr<Pool> pool,
        ////const vector<boost::shared_ptr<RecoveryRateModel> >&
        ////    rrModels,
        Real attachment,
        Real detachment,
        const boost::shared_ptr<Claim>& claim
        )
    : refDate_(refDate),
      claim_(claim),
      notionals_(notionals),
      pool_(pool),
 ////     rrModels_(rrModels),
      attachmentRatio_(attachment),
      detachmentRatio_(detachment),
      basketNotional_(0.0),
   //////   basketLGD_(0.0),
      trancheNotional_(0.0),
      attachmentAmount_(0.0),
      detachmentAmount_(0.0)/*,
      LGDs_(notionals.size(), 0.0)*//*,
      scenarioLoss_(names.size(), Loss(0.0, 0.0))*/ 
    {
        QL_REQUIRE(!notionals_.empty(), "notionals empty");
        QL_REQUIRE (attachmentRatio_ >= 0 &&
                    attachmentRatio_ <= detachmentRatio_ &&
                    detachmentRatio_ <= 1,
                    "invalid attachment/detachment ratio");
        QL_REQUIRE(pool_, "Empty pool pointer.");
        QL_REQUIRE(notionals_.size() == pool_->size(), 
                   "unmatched data entry sizes in basket");

        //////////for(Size i=0; i<notionals_.size(); i++)
        //////////    registerWith(rrModels_[i]);
        // registrations relevant to the loss status, not to the expected loss values; those are through models.
        registerWith(Settings::instance().evaluationDate());
        registerWith(claim_);

        // At this point Issuers in the pool might or might not have
        //   probability term structures for the defultKeys(eventType+
        //   currency+seniority) entering in this basket. This is not
        //   neccessarily a problem.
        for (Size i = 0; i < notionals_.size(); i++) {
            basketNotional_ += notionals_[i];
            attachmentAmount_ += notionals_[i] * attachmentRatio_;
            detachmentAmount_ += notionals_[i] * detachmentRatio_;
        }
        trancheNotional_ = detachmentAmount_ - attachmentAmount_;
    }

/*\todo Alternatively send a relinkable handle so it can be changed from the outside. In that case reconsider the observability chain.
*/
    void Basket::setLossModel(
        const boost::shared_ptr<DefaultLossModel>& lossModel) {

        if (lossModel_)
            unregisterWith(lossModel_);
        lossModel_ = lossModel;
        if (lossModel_) {
            //recovery quotes, defaults(once Issuer is observable)etc might 
            //  trigger us:
            registerWith(lossModel_);////////is this doubling what it is done at default loss model relinking???????????????????????????????????????????????
            // some magnitudes depend on the Loss Model:
  ////////////////////////          calculated_ = false;///this is done by Lazy::update()--------------------------------------

            /* Initialization of the model allows to cache any generic computations which are not specific to any magnitude. These might have different results for different baskets and have to be recomputed on each basket reassignment. The default loss model always have the option of being a lazy object but thats left as a choice.

            */
// SET BASKET NOW? the only difference is that the eval date magnitudes are not ready yet..
      // now the resposibility of Basket::performCalculations(), hmm not, the work is done in concrete DLM implementation of setupBasket(...)----->      lossModel_->initialize(*this);
        }
        // No local update, theres no need to recalculate realized losses; just reset and notify
        //update();



        LazyObject::update(); //<- just set calc=false
        //update();// or we might be called from an statistic member without being intialized yet (first called)
    }

    void Basket::performCalculations() const {
        // Calculations for status
        computeBasket();// or we might be called from an statistic member without being intialized yet (first called)

        // Calculations for set up of arguments to the model, and model notification(reset)
        QL_REQUIRE(lossModel_, "Basket has no default loss model assigned.");/// NOW I CAN REMOVE THE TESTS ON THE METHODS! FASTER!!

        // The model must notify us if the another basket calls it for reasignment. The basket works as an argument to the deafult loss models so, even if the models dont cache anything, they will be using the wrong defautl TS. \todo: This has a possible optimization: the basket incorporates trancheability and many models do their compuations independently of that (some do but do it inefficiently when asked for two tranches on the same basket; e,g, recursive model) so it might be more efficient sending the pool only; however the modtionals and other basket info are still used.
        lossModel_->setBasket(const_cast<Basket*>(this));// now ready for calls to model

// what happens to other basket pointing and registerd with the previous lossmodel?; they are still registerd and will be notified for no use....!!!!

        // THIS CAN NOT BE LIKE THIS!! ITS CYCLIC AND ITS A GREAT BUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        ////////////////if(lossModel_){
        ////////////////        evalDateCumulContingentLoss_ = cumulatedLoss(today);
        ////////////////    }else{
        ////////////////      evalDateCumulContingentLoss_ = evalDateSettledLoss_;
        ////////////////    }
    }


    //////////////////////////////Real Basket::basketLGD() const {
    //////////////////////////////    calculate();
    //////////////////////////////    return basketLGD_;
    //////////////////////////////}

    Disposable<vector<Real> > Basket::probabilities(const Date& d) const {
        vector<Real> prob(size());
        vector<DefaultProbKey> defKeys = defaultKeys();
        for (Size j = 0; j < size(); j++)
            prob[j] = pool_->get(pool_->names()[j]).defaultProbability(
                defKeys[j])->defaultProbability(d);
        return prob;
    }


    Real Basket::cumulatedLoss(const Date& endDate) const {
        calculate();
        // maybe return zero directly instead?:
        QL_REQUIRE(endDate >= refDate_, 
            "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?
        Real loss = 0.0;
        for (Size i = 0; i < size(); i++) {
            boost::shared_ptr<DefaultEvent> credEvent =
                pool_->get(pool_->names()[i]).defaultedBetween(refDate_, //startDate,
                    endDate, pool_->defaultKeys()[i]);
            if (credEvent) {
                /* \todo If the event has not settled one would need to introduce some model recovery rate (independently of a loss model) This remains to be done.
                  */  
                if(credEvent->hasSettled()) /////////////{
                    loss += claim_->amount(credEvent->date(),
                            // notionals_[i],
                            exposure(pool_->names()[i], credEvent->date()),
                            credEvent->settlement().recoveryRate(
                                pool_->defaultKeys()[i].seniority()));
       ////////////////         }else{//////////////////////////////////////////////////////////////////////////////////////////
       ////////////////             // might perform redundant checks and inits but gives us a chance of not needing one
       ////////////////             QL_REQUIRE(lossModel_, "Loss model not set for basket");
       ////////////////             //lock model
       ///////////////////// done in performCalcs.....             lossModel_->initialize(*this);// NOT CALLING CALCULATE, see if thats ok for the Loss Model, or it needs the basket to be computed....
       ////////////////             loss += claim_->amount(credEvent->date(),
       ////////////////                     //// notionals_[i],
       ////////////////                     exposure(pool_->names()[i], credEvent->date()),
       ////////////////                     lossModel_->recoveryValueImpl(credEvent->date(), ///////////////AND THIS IS A SECOND PROBLEM : FORCING THE RR CONCEPT INTO ALL MODELS!!!!
       ////////////////                         i, pool_->defaultKeys()));
       ////////////////             // unlock model
       ////////////////         }
            }
        }
        return loss;
    }
/*
    Real Basket::cumulatedLoss(const Date& startDate,
                               const Date& endDate) const {
        Real loss = 0.0;
        vector<DefaultProbKey> defKeys = defaultKeys();

        for (Size i = 0; i < names_.size(); i++) {
            boost::shared_ptr<DefaultEvent> credEvent =
                pool_->get(names_[i]).defaultedBetween(startDate,
                                                       endDate,
                                                       defKeys[i]);
            // to do: adjust for settlement notional accrued convexity, see doc
            if (credEvent) {
                if(credEvent->hasSettled()) {
                    loss += notionals_[i] * (1. -
                        credEvent->settlement().recoveryRate(
                          defKeys[i].seniority()));
                }else{// name defaulted but did not settled/confirm
                    loss += notionals_[i] * (1. -
                        rrModels_[i]->recoveryValue(credEvent->date(),
                                                    defKeys[i]
                                                ));
                }
            }
        }
        return loss;
    }
    */

    Real Basket::settledLoss(const Date& endDate) const {
        calculate();
        // maybe return zero directly instead?:
        QL_REQUIRE(endDate >= refDate_, 
            "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?
        
        Real loss = 0.0;
        for (Size i = 0; i < size(); i++) {
            boost::shared_ptr<DefaultEvent> credEvent =
                pool_->get(pool_->names()[i]).defaultedBetween(refDate_, // startDate,
                    endDate, pool_->defaultKeys()[i]);
            if (credEvent) {
                if(credEvent->hasSettled()) {
                    loss += claim_->amount(credEvent->date(),
                            //notionals_[i],
                            exposure(pool_->names()[i], credEvent->date()),//NOtice I am forcely requesting an exposure in the past...
                            // also the seniority does not belong to the counterparty anymore but to the poition.....
                            credEvent->settlement().recoveryRate(
                                pool_->defaultKeys()[i].seniority()));
                }
            }
        }
        return loss;
    }


    Real Basket::remainingNotional() const {
        calculate();
        return evalDateRemainingNot_;
    }

    Disposable<std::vector<Size> > 
        Basket::liveList(const Date& endDate) const {///// SHOULD RETURN  NAME STRING VECTOR!!!
        
        calculate();

        std::vector<Size> calcBufferLiveList;
        for (Size i = 0; i < size(); i++)
            if (!pool_->get(pool_->names()[i]).defaultedBetween(
                    refDate_,
                    endDate,
                    pool_->defaultKeys()[i]))
                calcBufferLiveList.push_back(i);

        return calcBufferLiveList;
    }

    Real Basket::remainingNotional(//const Date& startDate,
                                   const Date& endDate) const {
        Real notional = 0;
        vector<DefaultProbKey> defKeys = defaultKeys();
        for (Size i = 0; i < size(); i++) {
            if (!pool_->get(pool_->names()[i]).defaultedBetween(refDate_, // startDate,
                                                        endDate,
                                                        defKeys[i]))
                notional += notionals_[i];
        }
        return notional;
    }

    Disposable<vector<Real> > 
        Basket::remainingNotionals(const Date& endDate) const {
        calculate();

        QL_REQUIRE(endDate >= refDate_, 
            "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?

        std::vector<Real> calcBufferNotionals;
        const std::vector<Size>& alive = liveList(endDate);
        for(Size i=0; i<alive.size(); i++)
            calcBufferNotionals.push_back(
                // this is returning by position:
                //////positions_[alive[i]]->expectedExposure(endDate)
                exposure(pool_->names()[i], endDate)/////// OPTIMIZE , ITS BETTER TO LOOP ONCE OVER THE MAP
                ////------SHOULD BE USING INCEPTION NOTIONAL----?????? or a new method in instrPos allowing for amortization???
                );// some better way to trim it? 

        return calcBufferNotionals;
    }


    Disposable<std::vector<Probability> > 
        Basket::remainingProbabilities(const Date& d) const {

        calculate();
        QL_REQUIRE(d >= refDate_, "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?
        vector<Real> prob;
        const std::vector<Size>& alive = liveList();

        for(Size i=0; i<alive.size(); i++)
            prob.push_back(pool_->get(pool_->names()[i]).defaultProbability(
                pool_->defaultKeys()[i])->defaultProbability(d, true));
        return prob;
    }


        // maybe I need to have two terms: 'notionals' (by position) and 'exposures'(by counterparty)

    /**************************************************
            // These two are messy: say, ok I got the array, now the order(I am not returning a map) is what? that of the pool? that might mean a reordering of the lists at construction time.... or have a vector of names without duplicates?
            //All names, defaults ignored. i.e. programmed 
    Disposable<std::vector<Real> > Basket::exposures(const Date& d) const {

        // THESE MIGHT BE MEMBERS::::
        typedef std::multimap<std::string ,boost::shared_ptr<InstrumentPosition> >::const_iterator portfIter;
        typedef std::pair<std::string ,boost::shared_ptr<InstrumentPosition> > portfItem;

        std::set<std::string> tmpSet(names_.begin(), names_.end());// only they are ordered now
        std::vector<std::string> uniqueNames(tmpSet.begin(), tmpSet.end());
        Size numUniqueNames = uniqueNames.size();// SHOULD MATCH pool size
        QL_REQUIRE(numUniqueNames == pool_->size(), "Portfolio and pool counterparties should match exactly.");
        std::multimap<std::string, boost::shared_ptr<InstrumentPosition> > positionsMap;
        for(Size i=0; i<names_.size(); i++)
            positionsMap.insert(portfItem(names_[i], positions_[i]));
        
        vector<Real> cumul(numUniqueNames, 0.);
        for(Size i=0; i<numUniqueNames; i++) {
            std::pair<portfIter, portfIter> match = positionsMap.equal_range(uniqueNames[i]);
            do{
                cumul[i] += match.first->second->expectedExposure(d);
                match.first++;
            }while(match.first != match.second);
        }

        ////vector<Real> cumul;
        ////std::transform(positions_.begin(), positions_.end(), 
        ////    std::back_inserter(cumul),
        ////    boost::bind(
        ////        &InstrumentPosition::expectedExposure, 
        ////        _1, 
        ////        boost::cref(d)
        ////        ));
        return cumul;
    }
    ********************************/

    //! It is supossed to return the addition of ALL notionals from the requested ctpty......
    Real Basket::exposure(const std::string& name, const Date& d) const {
        // WRONG! There might be several positions associated to a given name. Might be worth splitting this method, one overload without date argument (meaning, at inception) at another one with.
        //////////////////////const std::vector<std::string>& poolNames = pool_->names();
// IF I HAVE CHANGED NOW TO A SINGLE ENTRY POOL THERES NO POINT IN MAINTAINING THIS ALGORITHM DESIGNED FOR MULTIPLE POSITIONS.
        //remember that 'this->names_' contains duplicates, contrary to 'pool->names'
        std::vector<std::string>::const_iterator match =  
 //           std::find(names_.begin(), names_.end(), name);
            std::find(pool_->names().begin(), pool_->names().end(), name);
        QL_REQUIRE(match != pool_->names().end(), "Name not in basket.");
        Real totalNotional = 0.;
        do{
            totalNotional += 
             //////   NOT IMPLEMENTED YET----   positions_[std::distance(names_.begin(), match)]->expectedExposure(d);
                notionals_[std::distance(pool_->names().begin(), match)];
            match++;
            match = std::find(match, pool_->names().end(), name);
        }while(match != pool_->names().end());

        return totalNotional;
        //Size position = std::distance(poolNames.begin(), 
        //    std::find(poolNames.begin(), poolNames.end(), name));
        //QL_REQUIRE(position < pool_->size(), "Name not in pool list");

        //return positions_[position]->expectedExposure(d);
    }



/*
    Disposable<std::vector<Real> > Basket::remainingNotionals(
        const Date& startDate, const Date& endDate) const 
    {
        vector<Real> notionals;
        vector<DefaultProbKey> defKeys = defaultKeys();
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defKeys[i]))
                notionals.push_back(notionals_[i]);
        }
        return notionals;
    }
*/
    /*
    Disposable<vector<string> > Basket::remainingNames(const Date& startDate,
                                          const Date& endDate) const {
        vector<string> names;
        vector<DefaultProbKey> defKeys = defaultKeys();
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defKeys[i]))
                names.push_back(names_[i]);
        }
        return names;
    }
*/

    Disposable<std::vector<std::string> >
        Basket::remainingNames(const Date& endDate) const {

        calculate();
        // maybe return zero directly instead?:
        QL_REQUIRE(endDate >= refDate_, 
            "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?

        const std::vector<Size>& alive = liveList(endDate);
        std::vector<std::string> calcBufferNames;
        for(Size i=0; i<alive.size(); i++)
            calcBufferNames.push_back(pool_->names()[alive[i]]);
/*
        std::transform(alive.begin(), alive.end(), 
            std::back_inserter(calcBufferNames),
 boost::cref(names_)[boost::lambda::_1]
       ///     boost::lambda::bind(std::vector<std::string>::operator[], boost::cref(names_), boost::lambda::_1)
            //[&names_](Size iAlive)names_ {return names_[iAlive];}
        );
*/
        return calcBufferNames;
    }

    ////////////vector<boost::shared_ptr<RecoveryRateModel> >
    ////////////    Basket::remainingRecModels(const Date& startDate,
    ////////////                                      const Date& endDate) const {
    ////////////    vector<boost::shared_ptr<RecoveryRateModel> > models;
    ////////////    vector<DefaultProbKey> defKeys = defaultKeys();
    ////////////    for (Size i = 0; i < names_.size(); i++) {
    ////////////        if (!pool_->get(names_[i]).defaultedBetween(startDate,
    ////////////                                                    endDate,
    ////////////                                                    defKeys[i]))
    ////////////            models.push_back(rrModels_[i]);
    ////////////    }
    ////////////    return models;
    ////////////}
/*
    Disposable<vector<DefaultProbKey> >
            Basket::remainingDefaultKeys(const Date& startDate,
                                              const Date& endDate) const {
        vector<DefaultProbKey> keys;
        vector<DefaultProbKey> defKeys = defaultKeys();
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defKeys[i]))
                keys.push_back(defKeys[i]);
        }
        return keys;
    }
*/

    Disposable<vector<DefaultProbKey> >
        Basket::remainingDefaultKeys(const Date& endDate) const {
        calculate();

        // maybe return zero directly instead?:
        QL_REQUIRE(endDate >= refDate_, // should dtae be in the future?????
            "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?

        const std::vector<Size>& alive = liveList(endDate);
        vector<DefaultProbKey> defKeys;
        for(Size i=0; i<alive.size(); i++)
            defKeys.push_back(pool_->defaultKeys()[alive[i]]);
        return defKeys;
    }


    Size Basket::remainingSize() const {
        calculate();
        return evalDateLiveList_.size();
    }


/*
    Real Basket::remainingAttachmentAmount(const Date& startDate,
                                           const Date& endDate) const {
        Real loss = cumulatedLoss(startDate, endDate);
        return std::max(0.0, attachmentAmount_ - loss);
    }
*/
    /*
    Real Basket::remainingAttachmentRatio(const Date& startDate,
                                          const Date& endDate) const {
        return remainingAttachmentAmount(startDate, endDate)
            / remainingNotional(startDate, endDate);
    }
*/
/*
    Real Basket::remainingDetachmentAmount(const Date& startDate,
                                           const Date& endDate) const {
        Real loss = cumulatedLoss(startDate, endDate);
        return std::max(0.0, detachmentAmount_ - loss);
    }
*/
    /*
    Real Basket::remainingDetachmentRatio(const Date& startDate,
                                          const Date& endDate) const {
        return remainingDetachmentAmount(startDate, endDate)
            / remainingNotional(startDate, endDate);
    }
*/
    //! computed on the inception values, notice the positions might have amortized or changed in value and the total outstanding notional might differ from the inception one.
    Real Basket::remainingDetachmentAmount(const Date& endDate) const {
        return detachmentAmount_;
        /*
        calculate();

        // maybe return zero directly instead?:
        QL_REQUIRE(endDate >= refDate_, 
            "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?
        // WHAT IF DATE IN THE FUTURE???????????? can be the case of a scenario (say a MC) while we R computing an expected value

        Real loss = settledLoss(endDate);
        return std::max(0.0, detachmentAmount_ - loss);
        */
    }

    //! computed on the inception values, notice the positions might have amortized or changed in value and the total outstanding notional might differ from the inception one.-----------Maybe the problem is not so impossible: at the time of a default the exposures are never stochastic by definition since they are in the past by definition. They renormalize the tranche but then it is much more complex..... It becomes a path problem.
    Real Basket::remainingAttachmentAmount(const Date& endDate) const {
        calculate();

        // maybe return zero directly instead?:
        QL_REQUIRE(endDate >= refDate_, 
            "Target date lies before basket inception");// MIGHT B A FORWARD BASKET! ret zero?
        // WHAT IF DATE IN THE FUTURE???????????? can be the case of a scenario (say a MC) while we R computing an expected value
        Real loss = settledLoss(endDate);
        return std::min(detachmentAmount_, attachmentAmount_ + std::max(0.0, loss - attachmentAmount_));
    }

    /*
    void Basket::updateScenarioLoss(bool zeroRecovery) {
        calculate();
        for (Size i = 0; i < size(); i++) {
            if (zeroRecovery)
                scenarioLoss_[i].amount = notionals_[i];
            else
                scenarioLoss_[i].amount = LGDs_[i];
            scenarioLoss_[i].time = pool_->getTime(pool_->names()[i]);
        }
        std::sort(scenarioLoss_.begin(), scenarioLoss_.end());
    }

    vector<Loss> Basket::scenarioIncrementalBasketLosses() const {
        return scenarioLoss_;
    }

    Real Basket::scenarioTrancheLoss(Date endDate) const {
        Real A = attachmentAmount_;
        Real D = detachmentAmount_;
        Date today = Settings::instance().evaluationDate();
        Real t2 = ActualActual().yearFraction(today, endDate);
        Real L = 0.0;
        for (Size i = 0; i < scenarioLoss_.size(); i++) {
            if (scenarioLoss_[i].time <= t2)
                L += scenarioLoss_[i].amount;
            else break;
        }
        return std::min(L, D) - std::min(L, A);
    }

    vector<Loss> Basket::scenarioIncrementalTrancheLosses(Date startDate,
                                                          Date endDate) const {
        vector<Loss> losses;
        Real A = attachmentAmount_;
        Real D = detachmentAmount_;
        Date today = Settings::instance().evaluationDate();
        Real tmin = ActualActual().yearFraction(today, startDate);
        Real tmax = ActualActual().yearFraction(today, endDate);
        Real TL1 = 0.0;
        Real L = 0.0;
        for (Size i = 0; i < scenarioLoss_.size(); i++) {
            Real t = scenarioLoss_[i].time;
            if (t > tmax && endDate != Date::maxDate()) break;
            if (t < tmin && startDate != Date::minDate()) continue;
            L += scenarioLoss_[i].amount;
            Real TL2 = std::min(L, D) - std::min(L, A);
            Real increment = TL2 - TL1;
            TL1 = TL2;
            losses.push_back(Loss(t, increment));
        }
        return losses;
    }

*/

    Probability Basket::probOverLoss(const Date& d, Real lossFraction) const {
        // check loss fraction units.............

        // convert initial basket fraction to remaining basket fraction
        calculate();
        // if eaten up all the tranche the prob of losing any amount is 1 (we have already lost it)
        if(evalDateRemainingNot_ == 0.) return 1.;

        // Turn to live (remaining) tranche units to feed into the model request. 
        // (should this be left to be done by the model too??, this is cheap and reduces the information in the model):
        Real xPtfl = attachmentAmount_ + 
            (detachmentAmount_-attachmentAmount_)*lossFraction;
        Real xPrim = (xPtfl- evalDateAttachAmount_)/
            (detachmentAmount_-evalDateAttachAmount_);// in live tranche fractional units
        // if the level falls within realized losses the prob is 1.
        if(xPtfl < 0.) return 1.;

   /////////////////     QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        // lock loss model
  ///SEE comment in expectedTrancheLoss      lossModel_->initialize(*this);//// SHOULD THIS BE AUTOMATIC??????????????????
        return lossModel_->probOverLoss(d, xPrim);
        // unlock loss model
    }

    Real Basket::percentile(const Date& d, Probability prob) const {
        calculate();
 /////////////////       QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
      ///SEE comment in expectedTrancheLoss        lossModel_->initialize(*this);
        return lossModel_->percentile(d, prob);

        //// ASSUMING NOW THE MODEL RETURNS TRANCHE PERCENTILE>>>>>>>
        Real percLiveFract = lossModel_->percentile(d, prob);     
        // unlock loss model
        // Model returns live tranche fractional units, turn into original tranche units:
        return (percLiveFract*(detachmentAmount_-evalDateAttachAmount_) + attachmentAmount_ - evalDateAttachAmount_)
            /(detachmentAmount_-attachmentAmount_);
    }

//////////////////////////////////////////////  ---- INLINE SOME OF THESE -----------------------------
Real Basket::expectedTrancheLoss(const Date& d) const {
        calculate();
 //////////////////////////       QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        // lock loss model
  ///////Redundant....? Also called when the model was asigned to this basket. Only if there has been registered observables that were updated in between....add a call in the update()???? Also a model might be referenced by several baskets...maybe the call to remove is the one in the asignment-------------------------------------------------------------      lossModel_->initialize(*this);
        /* All these initialize have a cost which in most times it <<<<<<<<<<<<<<<<<<<< TACKLE THIS POINT
        can be avoided. This is more important the heavier the 
        model's initialize(...) is, see  the BC for an example. But 
        could it be possible to perform the initialize of the model in 
        the calculate update only? The thing is that it would be a dirty 
        trick since the model itself at the initialize call, since it has a 
        handle to the basket, can be made to register, unregister and 
        selectively init, I am not doing this though, I think in that case 
        its better to make the model hold a reference to the basket.
        */
        return cumulatedLoss() + lossModel_->expectedTrancheLoss(d);
        // unlock loss model
    }


    Disposable<std::vector<Real> > 
        Basket::splitVaRLevel(const Date& date, Real loss) const {
 //////////////////////       QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        calculate();
    ///SEE comment in expectedTrancheLoss          lossModel_->initialize(*this);
        return lossModel_->splitVaRLevel(date, loss);
    }

    Real Basket::expectedShortfall(const Date& d, Probability prob) const {
 //////////////////////       QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        calculate();
   ///SEE comment in expectedTrancheLoss           lossModel_->initialize(*this);
        return lossModel_->expectedShortfall(d, prob);
    }

    Disposable<std::map<Real, Probability> > 
        Basket::lossDistribution(const Date& d) const {
//////////////////////        QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        calculate();
        // lock loss model
   ///SEE comment in expectedTrancheLoss           lossModel_->initialize(*this);
        return lossModel_->lossDistribution(d);
    }

    std::vector<Probability> 
        Basket::probsBeingNthEvent(Size n, const Date& d) const {
 //////////////////////       QL_REQUIRE(lossModel_, "Basket has no loss model assigned");

        Size alreadyDefaulted = pool_->size() - remainingNames().size();
        if(alreadyDefaulted >=n) 
            return std::vector<Probability>(remainingNames().size(), 0.);

        calculate();
        // lock loss model
     ///SEE comment in expectedTrancheLoss         lossModel_->initialize(*this);
        return lossModel_->probsBeingNthEvent(n-alreadyDefaulted, d);
    }


    Real Basket::defaultCorrelation(const Date& d, Size iName, Size jName) const{
//////////////////////        QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        calculate();
        return lossModel_->defaultCorrelation(d, iName, jName);

    }
    /*! Returns the probaility of having a given or larger number of 
    defaults in the basket portfolio at a given time.
    */
    Probability Basket::probAtLeastNEvents(Size n, const Date& d) const{
 //////////////////////       QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        calculate();
        return lossModel_->probAtLeastNEvents(n, d);

    }






    Real Basket::recoveryRate(const Date& d, Size iName) const {
//////////////////////        QL_REQUIRE(lossModel_, "Basket has no loss model assigned");
        calculate();
    ///SEE comment in expectedTrancheLoss          lossModel_->initialize(*this);
        return lossModel_->expectedRecovery(d, iName, pool_->defaultKeys()[iName]);//or remaining keys????
    }


}

