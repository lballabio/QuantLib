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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/credit/gaussianlhplossmodel.hpp>

#ifndef QL_PATCH_SOLARIS


using std::sqrt;

namespace QuantLib {

    CumulativeNormalDistribution const GaussianLHPLossModel::phi_ = 
        CumulativeNormalDistribution();

    GaussianLHPLossModel::GaussianLHPLossModel(
            const Handle<Quote>& correlQuote,
            const std::vector<Handle<RecoveryRateQuote> >& quotes)
        : LatentModel<GaussianCopulaPolicy>(sqrt(correlQuote->value()),
            quotes.size(),
            //g++ complains default value not seen as typename
            GaussianCopulaPolicy::initTraits()),
          sqrt1minuscorrel_(std::sqrt(1.-correlQuote->value())),
          correl_(correlQuote),
          rrQuotes_(quotes), 
          beta_(sqrt(correlQuote->value())),
          biphi_(-sqrt(correlQuote->value()))
        {
            registerWith(correl_);
            for (const auto& quote : quotes)
                registerWith(quote);
    }

    GaussianLHPLossModel::GaussianLHPLossModel(
            Real correlation,
            const std::vector<Real>& recoveries)
        : LatentModel<GaussianCopulaPolicy>(sqrt(correlation),
            recoveries.size(),
            //g++ complains default value not seen as typename
            GaussianCopulaPolicy::initTraits()),
          sqrt1minuscorrel_(std::sqrt(1.-correlation)),
          correl_(Handle<Quote>(ext::make_shared<SimpleQuote>(correlation))),
          beta_(sqrt(correlation)),
          biphi_(-sqrt(correlation))
        {
        for (Real recoverie : recoveries)
            rrQuotes_.emplace_back(ext::make_shared<RecoveryRateQuote>(recoverie));
        }

        GaussianLHPLossModel::GaussianLHPLossModel(
            const Handle<Quote>& correlQuote,
            const std::vector<Real>& recoveries)
        : LatentModel<GaussianCopulaPolicy>(sqrt(correlQuote->value()),
            recoveries.size(),
            //g++ complains default value not seen as typename
            GaussianCopulaPolicy::initTraits()),
          sqrt1minuscorrel_(std::sqrt(1.-correlQuote->value())),
          correl_(correlQuote),
          beta_(sqrt(correlQuote->value())),
          biphi_(-sqrt(correlQuote->value()))
        {
            registerWith(correl_);
            for (Real recoverie : recoveries)
                rrQuotes_.emplace_back(ext::make_shared<RecoveryRateQuote>(recoverie));
        }


        Real GaussianLHPLossModel::expectedTrancheLossImpl(
            Real remainingNot, // << at the given date 'd'
            Real prob, // << at the given date 'd'
            Real averageRR, // << at the given date 'd'
            // these are percentual values:
            Real attachLimit, Real detachLimit) const 
        {

            if (attachLimit >= detachLimit) return 0.;// or is it an error?
            // expected remaining notional:
            if (remainingNot == 0.) return 0.;

            const Real one = 1.0 - 1.0e-12;  // FIXME DUE TO THE INV CUMUL AT 1
            const Real k1 = std::min(one, attachLimit /(1.0 - averageRR)
                ) + QL_EPSILON;
            const Real k2 = std::min(one, detachLimit /(1.0 - averageRR)
                ) + QL_EPSILON;

            if (prob > 0) {
                const Real ip = InverseCumulativeNormal::standard_value(prob);
                const Real invFlightK1 = 
                    (ip-sqrt1minuscorrel_ * 
                        InverseCumulativeNormal::standard_value(k1))/beta_;
                const Real invFlightK2 = (ip-sqrt1minuscorrel_*
                    InverseCumulativeNormal::standard_value(k2))/beta_;

                return remainingNot * (detachLimit * phi_(invFlightK2) 
                    - attachLimit * phi_(invFlightK1) + (1.-averageRR) * 
                    (biphi_(ip, -invFlightK2) - biphi_(ip, -invFlightK1)) );
            }
            else return 0.0;
        }

        Real GaussianLHPLossModel::probOverLoss(const Date& d,
            Real remainingLossFraction) const {
            // these test goes into basket<<<<<<<<<<<<<<<<<<<<<<<<<
            QL_REQUIRE(remainingLossFraction >=0., "Incorrect loss fraction.");
            QL_REQUIRE(remainingLossFraction <=1., "Incorrect loss fraction.");

            Real remainingAttachAmount = basket_->remainingAttachmentAmount();
            Real remainingDetachAmount = basket_->remainingDetachmentAmount();
            // live unerlying portfolio loss fraction (remaining portf fraction)

            const Real remainingBasktNot = basket_->remainingNotional(d);
            const Real attach = 
                std::min(remainingAttachAmount / remainingBasktNot, 1.);
            const Real detach = 
                std::min(remainingDetachAmount / remainingBasktNot, 1.);

            Real portfFract = 
                attach + remainingLossFraction * (detach - attach);

            Real averageRR = averageRecovery(d);
            Real maxAttLossFract = (1.-averageRR);
            if(portfFract > maxAttLossFract) return 0.;

            // for non-equity losses add the probability jump at zero tranche 
            //   losses (since this method returns prob of losing more or 
            //   equal to)
            if(portfFract <= QL_EPSILON) return 1.;

            Probability prob = averageProb(d);

            Real ip = InverseCumulativeNormal::standard_value(prob);
            Real invFlightK = (ip-sqrt1minuscorrel_*
                InverseCumulativeNormal::standard_value(portfFract
                    /(1.-averageRR)))/beta_;

            return  phi_(invFlightK);//probOver
        }

        Real GaussianLHPLossModel::expectedShortfall(const Date& d, 
            Probability perctl) const 
        {
            // loss as a fraction of the live portfolio
            Real ptflLossPerc = percentilePortfolioLossFraction(d, perctl);
            Real remainingAttachAmount = basket_->remainingAttachmentAmount();
            Real remainingDetachAmount = basket_->remainingDetachmentAmount();

            const Real remainingNot = basket_->remainingNotional(d);
            const Real attach = 
                std::min(remainingAttachAmount / remainingNot, 1.);
            const Real detach = 
                std::min(remainingDetachAmount / remainingNot, 1.);

            if(ptflLossPerc >= detach-QL_EPSILON) 
                return remainingNot * (detach-attach);//equivalent

            Real maxLossLevel = std::max(attach, ptflLossPerc);
            Probability prob = averageProb(d);
            Real averageRR = averageRecovery(d);

            Real valA = expectedTrancheLossImpl(remainingNot, prob, 
                averageRR, maxLossLevel, detach);
            Real valB = // probOverLoss(d, maxLossLevel);//in live tranche units
            // from fraction of basket notional to fraction of tranche notional
                probOverLoss(d, std::min(std::max((maxLossLevel - attach)
                /(detach - attach), 0.), 1.));
            return ( valA + (maxLossLevel - attach) * remainingNot * valB )
                / (1.-perctl);
        }

        Real GaussianLHPLossModel::percentilePortfolioLossFraction(
            const Date& d, Real perctl) const 
        {
            // this test goes into basket<<<<<<<<<<<<<<<<<<<<<<<<<
            QL_REQUIRE(perctl >= 0. && perctl <=1., 
                "Percentile argument out of bounds.");

            if(perctl==0.) return 0.;// portfl == attach
            if(perctl==1.) perctl = 1. - QL_EPSILON; // portfl == detach

            return (1.-averageRecovery(d)) * 
                phi_( ( InverseCumulativeNormal::standard_value(averageProb(d))
                    + beta_ * InverseCumulativeNormal::standard_value(perctl) )
                        /sqrt1minuscorrel_);
        }

}

#endif
