/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/legacy/libormarketmodels/liborforwardmodel.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <limits>

namespace QuantLib {

     LiborForwardModel::LiborForwardModel(
          const ext::shared_ptr<LiborForwardModelProcess> & process,
          const ext::shared_ptr<LmVolatilityModel> & volaModel,
          const ext::shared_ptr<LmCorrelationModel> & corrModel)
     : CalibratedModel(volaModel->params().size()+corrModel->params().size()),
       f_(process->size()),
       accrualPeriod_(process->size()),
       covarProxy_(new LfmCovarianceProxy(volaModel, corrModel)),
       process_(process)
     {

         const Size k=volaModel->params().size();
         std::copy(volaModel->params().begin(), volaModel->params().end(),
                   arguments_.begin());
         std::copy(corrModel->params().begin(), corrModel->params().end(),
                   arguments_.begin()+k);

         for (Size i=0; i < process->size(); ++i) {
             accrualPeriod_[i] =  process->accrualEndTimes()[i]
                                - process->accrualStartTimes()[i];
             f_[i] = 1.0/(1.0+accrualPeriod_[i]*process_->initialValues()[i]);
         }
    }

    void LiborForwardModel::setParams(const Array& params) {
        CalibratedModel::setParams(params);

        const Size k=covarProxy_->volatilityModel()->params().size();

        covarProxy_->volatilityModel()->setParams(
            std::vector<Parameter>(arguments_.begin(), arguments_.begin()+k));
        covarProxy_->correlationModel()->setParams(
            std::vector<Parameter>(arguments_.begin()+k, arguments_.end()));

        swaptionVola = ext::shared_ptr<SwaptionVolatilityMatrix>();
    }

    Real LiborForwardModel::discountBondOption(Option::Type type,
                                               Real strike, Time maturity,
                                               Time bondMaturity) const {

        const std::vector<Time> & accrualStartTimes
            = process_->accrualStartTimes();
        const std::vector<Time> & accrualEndTimes
            = process_->accrualEndTimes();

        QL_REQUIRE(   accrualStartTimes.front()<= maturity
                   && accrualStartTimes.back() >= maturity,
                   "capet maturity does not fit to the process");

        const Size i = std::lower_bound(accrualStartTimes.begin(),
                                        accrualStartTimes.end(),
                                        maturity) - accrualStartTimes.begin();

        QL_REQUIRE(   i<process_->size()
                   && std::fabs(maturity - accrualStartTimes[i])
                        < 100*std::numeric_limits<Real>::epsilon()
                   && std::fabs(bondMaturity - accrualEndTimes[i])
                        < 100*std::numeric_limits<Real>::epsilon(),
                   "irregular fixings are not (yet) supported");

        const Real tenor     = accrualEndTimes[i] - accrualStartTimes[i];
        const Real forward   = process_->initialValues()[i];
        const Real capRate   = (1.0/strike - 1.0)/tenor;
        const Volatility var = covarProxy_
            ->integratedCovariance(i, i, process_->fixingTimes()[i]);
        const DiscountFactor dis =
            process_->index()->forwardingTermStructure()->discount(bondMaturity);

        const Real black = blackFormula(
            (type==Option::Put ? Option::Call : Option::Put),
            capRate, forward, std::sqrt(var));

        const Real npv = dis * tenor * black;

        return npv / (1.0 + capRate*tenor);
    }

    Disposable<Array> LiborForwardModel::w_0(Size alpha, Size beta) const {
        Array omega(beta+1,0.0);
        QL_REQUIRE(alpha<beta, "alpha needs to be smaller than beta");

        Real s=0.0;
        for (Size k=alpha+1; k<=beta; ++k) {
            Real b=accrualPeriod_[k];
            for (Size j=alpha+1; j<=k; ++j) {
                b*=f_[j];
            }
            s+=b;
        }

        for (Size i=alpha+1; i<=beta; ++i) {
            Real a=accrualPeriod_[i];
            for (Size j=alpha+1; j<=i; ++j) {
                a*=f_[j];
            }

            omega[i] = a/s;
        }

        return omega;
    }

    Rate LiborForwardModel::S_0(Size alpha, Size beta) const {
        const Array w = w_0(alpha, beta);
        const Array f = process_->initialValues();

        Rate fwdRate=0.0;
        for (Size i=alpha+1; i <=beta; ++i) {
            fwdRate+=w[i]*f[i];
        }

        return fwdRate;
    }

    // calculating swaption volatility matrix using
    // Rebonatos approx. formula. Be aware that this
    // matrix is valid only for regular fixings and
    // assumes that the fix and floating leg have the
    // same frequency
    ext::shared_ptr<SwaptionVolatilityMatrix>
        LiborForwardModel::getSwaptionVolatilityMatrix() const {
        if (swaptionVola != nullptr) {
            return swaptionVola;
        }

        const ext::shared_ptr<IborIndex> index = process_->index();
        const Date today = process_->fixingDates()[0];

        const Size size=process_->size()/2;
        Matrix volatilities(size, size);

        std::vector<Date> exercises(process_->fixingDates().begin() + 1,
                                    process_->fixingDates().begin() + size+1);

        std::vector<Period> lengths(size);
        for (Size i=0; i < size; ++i) {
            lengths[i] = (i+1)*index->tenor();
        }

        const Array f = process_->initialValues();
        for (Size k=0; k < size; ++k) {
            const Size alpha  =k;
            const Time t_alpha=process_->fixingTimes()[alpha+1];

            Matrix var(size, size);
            for (Size i=alpha+1; i <= k+size; ++i) {
                for (Size j=i; j <= k+size; ++j) {
                    var[i-alpha-1][j-alpha-1] = var[j-alpha-1][i-alpha-1] =
                        covarProxy_->integratedCovariance(i, j, t_alpha);
                }
            }

            for (Size l=1; l <= size; ++l) {
                const Size beta =l + k;
                const Array w = w_0(alpha, beta);

                Real sum=0.0;
                for (Size i=alpha+1; i <= beta; ++i) {
                    for (Size j=alpha+1; j <= beta; ++j) {
                        sum+=w[i]*w[j]*f[i]*f[j]*var[i-alpha-1][j-alpha-1];
                    }
                }
                volatilities[k][l-1] =
                    std::sqrt(sum/t_alpha)/S_0(alpha, beta);
            }
        }

        return swaptionVola = ext::make_shared<SwaptionVolatilityMatrix>(
            today, NullCalendar(), Following,
            exercises, lengths, volatilities,
            index->dayCounter());
    }

    // the next two methods are meaningless within this context
    // we might remove them from the AffineModel interface
    DiscountFactor LiborForwardModel::discount(Time t) const {
        return process_->index()->forwardingTermStructure()->discount(t);
    }

    Real LiborForwardModel::discountBond(Time, Time maturity, Array) const {
        return discount(maturity);
    }

}
