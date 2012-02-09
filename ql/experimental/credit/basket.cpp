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

    Basket::Basket(const vector<string>& names,
                   const vector<Real>& notionals,
                   const boost::shared_ptr<Pool> pool,
                   const vector<DefaultProbKey>& defaultKeys,
                   const vector<boost::shared_ptr<RecoveryRateModel> >&
                       rrModels,
                   Real attachment,
                   Real detachment)
        : names_(names),
          notionals_(notionals),
          pool_(pool),
          defaultKeys_(defaultKeys),
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
        QL_REQUIRE(names_.size() == notionals_.size() &&
                   notionals_.size() == defaultKeys_.size() &&
                   defaultKeys_.size() == rrModels_.size(),
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

    Size Basket::size() const {
        return names_.size();
    }

    const vector<string>& Basket::names() const {
        return names_;
    }

    const vector<Real>& Basket::notionals() const {
        return notionals_;
    }

    const std::vector<DefaultProbKey>&
    Basket::defaultKeys() const {
        return defaultKeys_;
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

    Real Basket::attachmentRatio() const {
        return attachmentRatio_;
    }

    Real Basket::detachmentRatio() const {
        return detachmentRatio_;
    }

    Real Basket::basketNotional() const {
        return basketNotional_;
    }

    Real Basket::basketLGD() const {
        calculate();
        return basketLGD_;
    }

    Real Basket::trancheNotional() const {
        return trancheNotional_;
    }

    Real Basket::attachmentAmount() const {
        return attachmentAmount_;
    }

    Real Basket::detachmentAmount() const {
        return detachmentAmount_;
    }

    vector<Real> Basket::probabilities(const Date& d) const {
        vector<Real> prob (names_.size());
        for (Size j = 0; j < names_.size(); j++)
            prob[j] = pool_->get(names_[j]).defaultProbability(
                defaultKeys_[j])->defaultProbability(d);
        return prob;
    }

    Real Basket::cumulatedLoss(const Date& startDate,
                               const Date& endDate) const {
        Real loss = 0.0;
        for (Size i = 0; i < names_.size(); i++) {
            boost::shared_ptr<DefaultEvent> credEvent =
                pool_->get(names_[i]).defaultedBetween(startDate,
                                                       endDate,
                                                       defaultKeys_[i]);
            // to do: adjust for settlement notional accrued convexity, see doc
            if (credEvent) {
                if(credEvent->hasSettled()) {
                    loss += notionals_[i] * (1. -
                        credEvent->settlement().recoveryRate(
                          defaultKeys_[i].seniority()));
                }else{// name defaulted but did not settled/confirm
                    loss += notionals_[i] * (1. -
                        rrModels_[i]->recoveryValue(credEvent->date(),
                                                    defaultKeys_[i]
                                                ));
                }
            }
        }
        return loss;
    }

    Real Basket::remainingNotional(const Date& startDate,
                                   const Date& endDate) const {
        Real notional = 0;
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defaultKeys_[i]))
                notional += notionals_[i];
        }
        return notional;
    }

    vector<Real> Basket::remainingNotionals(const Date& startDate,
                                            const Date& endDate) const {
        vector<Real> notionals;
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defaultKeys_[i]))
                notionals.push_back(notionals_[i]);
        }
        return notionals;
    }

    vector<string> Basket::remainingNames(const Date& startDate,
                                          const Date& endDate) const {
        vector<string> names;
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defaultKeys_[i]))
                names.push_back(names_[i]);
        }
        return names;
    }

    vector<boost::shared_ptr<RecoveryRateModel> >
        Basket::remainingRecModels(const Date& startDate,
                                          const Date& endDate) const {
        vector<boost::shared_ptr<RecoveryRateModel> > models;
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defaultKeys_[i]))
                models.push_back(rrModels_[i]);
        }
        return models;
    }

        vector<DefaultProbKey>
            Basket::remainingDefaultKeys(const Date& startDate,
                                              const Date& endDate) const {
        vector<DefaultProbKey> keys;
        for (Size i = 0; i < names_.size(); i++) {
            if (!pool_->get(names_[i]).defaultedBetween(startDate,
                                                        endDate,
                                                        defaultKeys_[i]))
                keys.push_back(defaultKeys_[i]);
        }
        return keys;
    }

    Real Basket::remainingAttachmentAmount(const Date& startDate,
                                           const Date& endDate) const {
        Real loss = cumulatedLoss(startDate, endDate);
        return std::max(0.0, attachmentAmount_ - loss);
    }

    Real Basket::remainingAttachmentRatio(const Date& startDate,
                                          const Date& endDate) const {
        return remainingAttachmentAmount(startDate, endDate)
            / remainingNotional(startDate, endDate);
    }

    Real Basket::remainingDetachmentAmount(const Date& startDate,
                                           const Date& endDate) const {
        Real loss = cumulatedLoss(startDate, endDate);
        return std::max(0.0, detachmentAmount_ - loss);
    }

    Real Basket::remainingDetachmentRatio(const Date& startDate,
                                          const Date& endDate) const {
        return remainingDetachmentAmount(startDate, endDate)
            / remainingNotional(startDate, endDate);
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

    void Basket::performCalculations() const {
        Date today = Settings::instance().evaluationDate();
        for (Size i = 0; i < notionals_.size(); i++) {
            //we are registered, the quote might have changed.
            QL_REQUIRE(
                rrModels_[i]->appliesToSeniority(defaultKeys_[i].seniority()),
                "Recovery model does not match basket member seniority.");

            LGDs_[i] = notionals_[i]
            * (1.0 - rrModels_[i]->recoveryValue(today,
                                                 defaultKeys_[i]
                                                 ));
            basketLGD_ += LGDs_[i];
        }
    }


}

