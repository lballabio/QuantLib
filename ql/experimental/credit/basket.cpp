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

#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/loss.hpp>
#include <ql/time/daycounters/actualactual.hpp>

using namespace std;

namespace QuantLib {

    Basket::Basket(const Date& refDate,
        const vector<string>& names,
                   const vector<Real>& notionals,
                   const boost::shared_ptr<Pool> pool,
                   const vector<boost::shared_ptr<RecoveryRateModel> >&
                       rrModels,
                   Real attachment,
                   Real detachment)
        : refDate_(refDate),
        names_(names),
          notionals_(notionals),
          pool_(pool),
          rrModels_(rrModels),
          attachmentRatio_(attachment),
          detachmentRatio_(detachment),
          basketNotional_(0.0),
          basketLGD_(0.0),
          trancheNotional_(0.0),
          attachmentAmount_(0.0),
          detachmentAmount_(0.0),
          LGDs_(notionals.size(), 0.0),
          scenarioLoss_(names.size(), Loss(0.0, 0.0)) {
        QL_REQUIRE(!names_.empty(), "no names given");
        QL_REQUIRE(!notionals_.empty(), "notionals empty");
        QL_REQUIRE (attachmentRatio_ >= 0 &&
                    attachmentRatio_ <= detachmentRatio_ &&
                    detachmentRatio_ <= 1,
                    "invalid attachment/detachment ratio");
        QL_REQUIRE(pool_, "Empty pool pointer.");
        QL_REQUIRE(names_.size() == notionals_.size() &&
                   notionals_.size() == pool_->size(), 
                   "unmatched data entry sizes in basket");

        for(Size i=0; i<notionals_.size(); i++)
            registerWith(rrModels_[i]);
        registerWith(Settings::instance().evaluationDate());

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


    void Basket::performCalculations() const {
        Date today = Settings::instance().evaluationDate();
            /* the methods called now invoke calculate(), this is not recursive
              since we have the calculated flag set to true by now.
            */
            /* update cache values at the calculation date (work as arguments 
              to the Loss Models)
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
     //       if(lossModel_){
     //           evalDateCumulContingentLoss_ = cumulatedLoss(today);
     //       }else{
                evalDateCumulContingentLoss_ = evalDateSettledLoss_;
     //       }

            /*  

        vector<DefaultProbKey> defKeys = defaultKeys();

        for (Size i = 0; i < notionals_.size(); i++) {
            //we are registered, the quote might have changed.
            QL_REQUIRE(
                rrModels_[i]->appliesToSeniority(defKeys[i].seniority()),
                "Recovery model does not match basket member seniority.");

            LGDs_[i] = notionals_[i]
            * (1.0 - rrModels_[i]->recoveryValue(today,
                                                 defKeys[i]
                                                 ));
            basketLGD_ += LGDs_[i];
        }
        */
    }

    Size Basket::size() const {
        return names_.size();
    }

    //const vector<string>& Basket::names() const {
    //    return names_;
    //}

    const vector<Real>& Basket::notionals() const {
        return notionals_;
    }

    Disposable<std::vector<DefaultProbKey> >
    Basket::defaultKeys() const {
        return pool_->defaultKeys();
    }

    const std::vector<boost::shared_ptr<RecoveryRateModel> >&
    Basket::recoveryModels() const {
        return rrModels_;
    }

    boost::shared_ptr<Pool> Basket::pool() const {
        return pool_;
    }

    const vector<Real>& Basket::LGDs() const {
        calculate();
        return LGDs_;
    }

    //////////////////////////////Real Basket::basketLGD() const {
    //////////////////////////////    calculate();
    //////////////////////////////    return basketLGD_;
    //////////////////////////////}

    Disposable<vector<Real> > Basket::probabilities(const Date& d) const {
        vector<Real> prob(names_.size());
        vector<DefaultProbKey> defKeys = defaultKeys();
        for (Size j = 0; j < names_.size(); j++)
            prob[j] = pool_->get(names_[j]).defaultProbability(
                defKeys[j])->defaultProbability(d);
        return prob;
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
        for (Size i = 0; i < names_.size(); i++) {
            boost::shared_ptr<DefaultEvent> credEvent =
                pool_->get(names_[i]).defaultedBetween(
                    refDate_, // startDate,
                    endDate,
                    pool_->defaultKeys()[i]);
            if (credEvent) {
                if(credEvent->hasSettled()) {
                    loss += 
                        claim_->amount(
                            credEvent->date(),

                            //notionals_[i],
                            exposure(names_[i], credEvent->date()),//NOtice I am forcely requesting an exposure in the past...
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
        for (Size i = 0; i < names_.size(); i++)
            if (!pool_->get(names_[i]).defaultedBetween(
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
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(refDate_, // startDate,
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
                exposure(names_[i], endDate)/////// OPTIMIZE , ITS BETTER TO LOOP ONCE OVER THE MAP
                ////------SHOULD BE USING INCEPTION NOTIONAL----?????? or a new method in instrPos allowing for amortization???
                );// some better way to trim it? 

        return calcBufferNotionals;
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
            calcBufferNames.push_back(names_[alive[i]]);
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

    vector<boost::shared_ptr<RecoveryRateModel> >
        Basket::remainingRecModels(const Date& startDate,
                                          const Date& endDate) const {
        vector<boost::shared_ptr<RecoveryRateModel> > models;
        vector<DefaultProbKey> defKeys = defaultKeys();
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defKeys[i]))
                models.push_back(rrModels_[i]);
        }
        return models;
    }
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

    void Basket::updateScenarioLoss(bool zeroRecovery) {
        calculate();
        for (Size i = 0; i < names_.size(); i++) {
            if (zeroRecovery)
                scenarioLoss_[i].amount = notionals_[i];
            else
                scenarioLoss_[i].amount = LGDs_[i];
            scenarioLoss_[i].time = pool_->getTime(names_[i]);
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

}

