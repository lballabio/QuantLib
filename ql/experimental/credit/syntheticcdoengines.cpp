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

#include <ql/experimental/credit/syntheticcdoengines.hpp>
#include <ql/experimental/credit/loss.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/time/daycounters/actualactual.hpp>

using namespace std;

namespace QuantLib {

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void IntegralCDOEngine::calculate() const {
        Date today = Settings::instance().evaluationDate();
        const vector<Date>& dates = arguments_.schedule.dates();

        results_.protectionValue = 0.0;
        results_.premiumValue = 0.0;
        results_.upfrontPremiumValue = 0.0;
        results_.error = 0;
        results_.expectedTrancheLoss.clear();
        results_.expectedTrancheLoss.resize(dates.size(), 0.0);

        // set remainingBasket_, results_.remainingNotional,
        // vector results_.expectedTrancheLoss for all schedule dates
        initialize();

        Real e1 = 0;
        if (arguments_.schedule.dates().front() > today)
            e1 = expectedTrancheLoss (arguments_.schedule.dates()[0]);

        for (Size i = 1; i < arguments_.schedule.size(); i++) {
            Date d2 = arguments_.schedule.dates()[i];
            if (d2 < today)
                continue;

            Date d1 = arguments_.schedule.dates()[i-1];

            Date d, d0 = d1;
            do {
                d = NullCalendar().advance (d0 > today ? d0 : today,
                                            stepSize_);
                if (d > d2) d = d2;

                Real e2 = expectedTrancheLoss (d);

                results_.premiumValue
                    += (results_.remainingNotional - e2)
                    * arguments_.runningRate
                    * arguments_.dayCounter.yearFraction (d0, d)
                    * arguments_.yieldTS->discount (d);

                if (e2 < e1) results_.error ++;

                results_.protectionValue
                    += (e2 - e1) * arguments_.yieldTS->discount (d);

                d0 = d;
                e1 = e2;
            }
            while (d < d2);
        }

        if (arguments_.schedule.dates().front() >= today)
            results_.upfrontPremiumValue
                = results_.remainingNotional * arguments_.upfrontRate
                * arguments_.yieldTS->discount(arguments_.schedule.dates()[0]);

        if (arguments_.side == Protection::Buyer) {
            results_.protectionValue *= -1;
            results_.premiumValue *= -1;
            results_.upfrontPremiumValue *= -1;
        }

        results_.value = results_.premiumValue - results_.protectionValue
            + results_.upfrontPremiumValue;

        results_.errorEstimate = Null<Real>();
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void MidPointCDOEngine::calculate() const {
        Date today = Settings::instance().evaluationDate();

        results_.premiumValue = 0.0;
        results_.upfrontPremiumValue = 0.0;
        results_.protectionValue = 0.0;
        results_.expectedTrancheLoss.clear();

        // set remainingBasket_, results_.remainingNotional,
        // vector results_.expectedTrancheLoss for all schedule dates
        initialize();

        const vector<Date>& dates = arguments_.schedule.dates();
        if (dates.front() > today)
            results_.upfrontPremiumValue =
                arguments_.upfrontRate * results_.remainingNotional;

        vector<boost::shared_ptr<CashFlow> > premiumLeg =
            FixedRateLeg(arguments_.schedule)
            .withCouponRates(arguments_.runningRate, arguments_.dayCounter)
            .withPaymentAdjustment(arguments_.paymentConvention)
            .withNotionals(1.0);

        Real e1 = 0;
        if (dates[0] > today)
            e1 = expectedTrancheLoss (dates[0]);

        for (Size i = 0; i < premiumLeg.size(); i++) {
            boost::shared_ptr<Coupon> coupon =
                boost::dynamic_pointer_cast<Coupon>(premiumLeg[i]);
            Date paymentDate = coupon->date();
            Date startDate = std::max(coupon->accrualStartDate(),
                                      arguments_.yieldTS->referenceDate());
            Date endDate = coupon->accrualEndDate();
            Date defaultDate = startDate + (endDate-startDate)/2;
            if (paymentDate <= today)
                continue;

            Real e2 = expectedTrancheLoss(paymentDate);

            results_.premiumValue += (results_.remainingNotional - e2)
                * coupon->amount()
                * arguments_.yieldTS->discount(paymentDate);

            Real discount = arguments_.yieldTS->discount(defaultDate);
            results_.premiumValue += coupon->accruedAmount(defaultDate)
                * discount * (e2 - e1);
            results_.protectionValue += discount * (e2 - e1);

            e1 = e2;
        }

        if (arguments_.side == Protection::Buyer) {
            results_.protectionValue *= -1;
            results_.premiumValue *= -1;
            results_.upfrontPremiumValue *= -1;
        }

        results_.value = results_.premiumValue - results_.protectionValue
            + results_.upfrontPremiumValue;

        results_.errorEstimate = Null<Real>();
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void MonteCarloCDOEngine1::defaultScenarios() const {
        results_.expectedTrancheLoss.clear();
        const vector<Date>& dates = arguments_.schedule.dates();

        Date today = Settings::instance().evaluationDate();
        Real tmax = ActualActual().yearFraction(today, dates.back());
        QL_REQUIRE(tmax >= 0, "tmax < 0");

        /*
          1) Generate a vector of random default times in the single-factor
             Gaussian Copula framework
          2) Work out cumulative portfolio and tranche loss for each scenario
          3) Map cumulative tranche losses to schedule dates
          4) Average over many scenarios
         */

        const boost::shared_ptr<Pool> pool = remainingBasket_->pool();

        vector<vector<Real> > cumulativeTrancheLoss(samples_, vector<Real>());

        results_.expectedTrancheLoss.resize(dates.size(), 0.0);
        for (Size i = 0; i < samples_; i++) {
            rdm_->nextSequence(tmax);

            cumulativeTrancheLoss[i].resize(dates.size(), 0.0);
            remainingBasket_->updateScenarioLoss();
            for (Size k = 0; k < dates.size(); k++) {
                cumulativeTrancheLoss[i][k]
                    = remainingBasket_->scenarioTrancheLoss(dates[k]);
                // aggregate
                results_.expectedTrancheLoss[k] += cumulativeTrancheLoss[i][k];
            }
        }

        // normalize
        for (Size i = 0; i < dates.size(); i++)
            results_.expectedTrancheLoss[i] /= samples_;
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    void MonteCarloCDOEngine2::calculate() const {
        Date today = Settings::instance().evaluationDate();

        results_.protectionValue = 0.0;
        results_.premiumValue = 0.0;
        results_.expectedTrancheLoss.clear();

        // set remainingBasket_, results_.remainingNotional,
        initialize();

        const vector<Date>& dates = arguments_.schedule.dates();
        if (dates.front() > today)
            results_.upfrontPremiumValue =
                arguments_.upfrontRate * results_.remainingNotional;

        Real tmax = ActualActual().yearFraction(today, dates.back());
        //Real tmin = ActualActual().yearFraction(today, dates.front());
        QL_REQUIRE(tmax >= 0, "tmax < 0");

        vector<boost::shared_ptr<CashFlow> > premiumLeg =
            FixedRateLeg(arguments_.schedule)
            .withNotionals(1.0)
            .withCouponRates(arguments_.runningRate, arguments_.dayCounter)
            .withPaymentAdjustment(arguments_.paymentConvention);

        boost::shared_ptr<Pool> pool = remainingBasket_->pool();

        vector<Real> premiumValue(samples_, 0.0);
        vector<Real> protectionValue(samples_, 0.0);
        vector<Real> value(samples_, 0.0);
        vector<Real> fairPremium(samples_, 0.0);
        vector<vector<Real> > cumulativeTrancheLoss(samples_, vector<Real>());

        for (Size i = 0; i < samples_; i++) { //================================

            /******************************************************************
             * (1) Compute default times
             ******************************************************************/
            rdm_->nextSequence(tmax);

            /******************************************************************
             * (2) Cumulative tranche loss to schedule dates
             ******************************************************************/
            cumulativeTrancheLoss[i].resize(dates.size(), 0.0);
            remainingBasket_->updateScenarioLoss();
            for (Size k = 0; k < dates.size(); k++)
                cumulativeTrancheLoss[i][k]
                    = remainingBasket_->scenarioTrancheLoss(dates[k]);

            /*****************************************************************
             * (3) Contribution of this scenario to the protection leg
             *     - Loop through all incremental tranche loss events between
             *       start and end date
             *     - Pay and discount these increments as they occur
             *****************************************************************/
            vector<Loss> increments = remainingBasket_->scenarioIncrementalTrancheLosses(dates.front(), dates.back());
            for (Size k = 0; k < increments.size(); k++)
                protectionValue[i] += increments[k].amount
                    * arguments_.yieldTS->discount(increments[k].time);

            /*****************************************************************
             * (4) Contribution of this scenario to the premium leg
             *     - Loop through all coupon periods
             *     - Pay coupon at period end on effective notional
             *     - Effective notional:
             *       - Start with remaining notional minus cumulative loss
             *         on the tranche until period start =: N
             *       - Reduce N for each loss in the period by subtracting the
             *         the incremental tranche loss weighted with the time
             *         to period end
             *****************************************************************/
            for (Size j = 0; j < premiumLeg.size(); j++) {
                boost::shared_ptr<Coupon> coupon =
                    boost::dynamic_pointer_cast<Coupon>(premiumLeg[j]);
                Date startDate = std::max(coupon->accrualStartDate(),
                                          arguments_.yieldTS->referenceDate());
                Date endDate = coupon->accrualEndDate();
                Date paymentDate = coupon->date();
                if (paymentDate <= today)
                    continue;
                Real t1 = ActualActual().yearFraction(today, startDate);
                Real t2 = ActualActual().yearFraction(today, endDate);
                Real PL = cumulativeTrancheLoss[i][j];
                Real N = results_.remainingNotional - PL;
                for (Size k = 0; k < increments.size(); k++) {
                    Real t = increments[k].time;
                    if (t <= t1) continue;
                    if (t >= t2) break;
                    N -= (t2-t) / (t2-t1) * increments[k].amount;
                }
                Real discount = arguments_.yieldTS->discount(paymentDate);
                premiumValue[i] += N * coupon->amount() * discount;
            }

            /*****************
             * Aggregate
             *****************/
            results_.premiumValue += premiumValue[i];
            results_.protectionValue += protectionValue[i];
            value[i] = premiumValue[i] - protectionValue[i]
                + results_.upfrontPremiumValue;
            for (Size k = 0; k < dates.size(); k++)
                results_.expectedTrancheLoss[k] += cumulativeTrancheLoss[i][k];

            /*
            cout.setf (ios::fixed, ios::floatfield);
            cout << setprecision(0);
            for (Size k = 0; k < dates.size(); k++)
                cout << setw(3) << cumulativeTrancheLoss[i][k] << " ";
            cout << endl;

            cout << setprecision(2);
            for (Size k = 0; k < pool->size(); k++) {
                const string name = pool->names()[k];
                Real t =  pool->getTime(name);
                if (t < 6)
                    cout << setw(10) << name << " " << setw(5) << t << endl;
            }
            */
        } // end of loop over samples ==========================================

        /*****************************************
         * Expected values, normalize, switch sign
         *****************************************/
        results_.premiumValue /= samples_;
        results_.protectionValue /= samples_;
        for (Size k = 0; k < dates.size(); k++)
            results_.expectedTrancheLoss[k] /= samples_;

        if (arguments_.side == Protection::Buyer) {
            results_.protectionValue *= -1;
            results_.premiumValue *= -1;
            results_.upfrontPremiumValue *= -1;
        }

        results_.value = results_.premiumValue - results_.protectionValue
            + results_.upfrontPremiumValue;

        /*************************************************
         * Error estimates - NPV
         *************************************************/
        Real avg = 0.0;
        Real var = 0.0;
        for (Size i = 0; i < samples_; i++) {
            var += value[i] * value[i];
            avg += value[i];
        }
        avg /= samples_;
        var /= samples_;
        results_.errorEstimate = sqrt(var - avg * avg);

        /*****************************************************
         * Error estimates - fair premium
         * http://math.nyu.edu/~atm262/files/spring06/ircm/cdo
         *****************************************************/
        /*
        Real x = 0.0, xx = 0.0, y = 0.0, yy = 0.0, xy = 0.0;
        for (Size i = 0; i < samples_; i++) {
            Real dx = protectionValue[i] - results_.upfrontPremiumValue;
            Real dy = premiumValue[i];
            x += dx;
            xx += dx * dx;
            y += dy;
            yy += dy * dy;
            xy += dx * dy;
        }
        x /= samples_;
        y /= samples_;
        xx /= samples_;
        yy /= samples_;
        xy /= samples_;

        Real v = x*x/(y*y) * (xx/(x*x) + yy/(y*y) - 2.0 * xy/(x*y));
        Real stdFairPremium = sqrt(v) * arguments_.runningRate;
        */
    }

}
