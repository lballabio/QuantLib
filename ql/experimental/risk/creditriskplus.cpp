/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2013 Peter Caspers

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

#include <ql/experimental/risk/creditriskplus.hpp>
#include <map>
#include <utility>

using std::sqrt;

namespace QuantLib {

    CreditRiskPlus::CreditRiskPlus(std::vector<Real> exposure,
                                   std::vector<Real> defaultProbability,
                                   std::vector<Size> sector,
                                   std::vector<Real> relativeDefaultVariance,
                                   Matrix correlation,
                                   const Real unit)
    : exposure_(std::move(exposure)), pd_(std::move(defaultProbability)),
      sector_(std::move(sector)), relativeDefaultVariance_(std::move(relativeDefaultVariance)),
      correlation_(std::move(correlation)), unit_(unit) {

        m_ = exposure_.size();

        QL_REQUIRE(m_ > 0, "no exposures given");
        QL_REQUIRE(m_ == pd_.size(), "number of exposures ("
                                         << m_
                                         << ") must be equal to number of pds ("
                                         << pd_.size() << ")");
        QL_REQUIRE(m_ == sector_.size(),
                   "number of exposures ("
                       << m_
                       << ") must be equal to number of exposure sectors ("
                       << sector_.size() << ")");

        n_ = correlation_.rows();
        QL_REQUIRE(correlation_.columns() == n_,
                   "correlation matrix (" << n_ << "," << correlation_.columns()
                                          << ") must be a square matrix");

        QL_REQUIRE(relativeDefaultVariance_.size() == n_,
                   "number of relative default variances ("
                       << relativeDefaultVariance_.size() << ")"
                       << " must be equal to number of sectors (" << n_ << ")");

        exposureSum_ = 0.0;
        el_ = 0.0;
        el2_ = 0.0;
        for (Size i = 0; i < m_; ++i) {
            QL_REQUIRE(exposure_[i] >= 0.0, "exposure #"
                                                << i << " is negative ("
                                                << exposure_[i] << ")");
            QL_REQUIRE(pd_[i] > 0.0, "pd #" << i << " is negative (" << pd_[i]
                                            << ")");
            QL_REQUIRE(sector_[i] < n_, "sector #" << i << " (" << sector_[i]
                                                   << ") is out of range 0..."
                                                   << (n_ - 1));
            exposureSum_ += exposure_[i];
            el_ += pd_[i] * exposure_[i];
            el2_ += pd_[i] * exposure_[i]*exposure_[i];
        }

        QL_REQUIRE(unit_ > 0.0, "loss unit (" << unit_ << ") must be positive");

        compute();
    }

    Real CreditRiskPlus::lossQuantile(const Real p) {

        Size i = 0;
        Real sum = loss_[0];
        while(i < upperIndex_-1 && sum < p) {
            ++i;
            sum += loss_[i];
        }

        if(loss_[0] >= p)
            return 0.0;

        Real p1 = sum - loss_[i];
        Real p2 = sum >= p ? sum : 1.0;
        Real l1 = (i - 1) * unit_;
        Real l2 = i * unit_;

        return l1 + (p - p1) / (p2 - p1) * (l2 - l1);
    }

    void CreditRiskPlus::compute() {

        std::vector<Real> sectorPdSum_, sectorSpecTerms_;

        sectorPdSum_ = std::vector<Real>(n_, 0.0);
        sectorExposure_ = std::vector<Real>(n_, 0.0);
        sectorEl_ = std::vector<Real>(n_, 0.0);
        sectorSpecTerms_ = std::vector<Real>(n_, 0.0);
        sectorUl_ = std::vector<Real>(n_, 0.0);
        marginalLoss_ = std::vector<Real>(m_, 0.0);

        std::vector<Real> pdAdj(m_, 0.0);

        // compute exposure bands

        unsigned long maxNu_ = 0;
        upperIndex_ = 0;

        // map of nuC_ to expected loss
        std::map<unsigned long, Real, std::less<unsigned long> > epsNuC_;

        std::map<unsigned long, Real, std::less<unsigned long> >::iterator iter;

        for (Size k = 0; k < m_; ++k) {
            auto exUnit = (unsigned long)(std::floor(0.5 + exposure_[k] / unit_)); // round
            if (exposure_[k] > 0 && exUnit == 0)
                exUnit = 1; // but avoid zero exposure
            if (exUnit > maxNu_)
                maxNu_ = exUnit;
            pdAdj[k] = exposure_[k] > 0.0
                           ? exposure_[k] * pd_[k] / (exUnit * unit_)
                           : Real(0.0); // adjusted pd
            Real el = exUnit * pdAdj[k];
            if (exUnit > 0) {
                iter = epsNuC_.find(exUnit);
                if (iter == epsNuC_.end()) {
                    epsNuC_.insert(std::pair<unsigned long, Real>(exUnit, el));
                } else {
                    (*iter).second += el;
                }
                upperIndex_ += exUnit;
            }
        }

        // compute per sector figures

        Real pdSum_ = 0;
        for (Size k = 0; k < m_; ++k) {
            pdSum_ += pdAdj[k];
            sectorPdSum_[sector_[k]] += pd_[k];
            sectorExposure_[sector_[k]] += exposure_[k];
            sectorEl_[sector_[k]] += exposure_[k] * pd_[k];
        }

        for (Size i = 0; i < n_; ++i) {

            // precompute sector specific terms (formula 15 in [1])

            sectorSpecTerms_[i] += relativeDefaultVariance_[i] * sectorEl_[i];
            for (Size j = 0; j < n_; ++j) {
                if (j != i) {
                    sectorSpecTerms_[i] +=
                        correlation_[i][j] *
                        std::sqrt(relativeDefaultVariance_[i] *
                                  relativeDefaultVariance_[j]) *
                        sectorEl_[j];
                }
            }
        }

        // compute synthetic standard deviation (formula 12 in [1])

        ul_ = 0.0;
        for (Size i = 0; i < n_; ++i) {
            sectorUl_[i] =
                relativeDefaultVariance_[i] * sectorEl_[i] * sectorEl_[i];
            ul_ += sectorUl_[i];
            for (Size j = 0; j < n_; ++j) {
                if (j != i) {
                    ul_ += correlation_[i][j] *
                           std::sqrt(relativeDefaultVariance_[i] *
                                     relativeDefaultVariance_[j]) *
                           sectorEl_[i] * sectorEl_[j];
                }
            }
        }

        Real matchUl_ = ul_; // formula 13 in [1], rhs
        for (Size k = 0; k < m_; ++k) {
            Real tmp = pd_[k] * exposure_[k] * exposure_[k];
            sectorUl_[sector_[k]] += tmp;
            ul_ += tmp;
        }
        ul_ = std::sqrt(ul_);
        for (Size i = 0; i < n_; ++i)
            sectorUl_[i] = std::sqrt(sectorUl_[i]);

        // compute risk contributions (formula 15 in [1])

        for (Size k = 0; k < m_; ++k) {
            marginalLoss_[k] = pd_[k] * exposure_[k] / ul_ *
                               (sectorSpecTerms_[sector_[k]] + exposure_[k]);
        }

        // compute sigmaC_ and deduced figures

        Real sigmaC_ = pdSum_ * sqrt(matchUl_ / (el_ * el_));
        Real alphaC_ = pdSum_ * pdSum_ / (sigmaC_ * sigmaC_);
        Real betaC_ = sigmaC_ * sigmaC_ / pdSum_;
        Real pC_ = betaC_ / (1.0 + betaC_);

        // compute loss distribution

        loss_.clear();
        loss_.push_back(std::pow(1.0 - pC_, alphaC_)); // A(0)

        Real res;
        for (unsigned long n = 0; n < upperIndex_ - 1; ++n) { // compute A(n+1)
                                                              // recursively
            res = 0.0;
            for (unsigned long j = 0;
                 j <= std::min<unsigned long>(maxNu_ - 1, n); ++j) {
                iter = epsNuC_.find(j + 1);
                if (iter != epsNuC_.end()) {
                    res += (*iter).second * loss_[n - j] * alphaC_;
                    if (j <= n - 1)
                        res += (*iter).second / ((Real)(j + 1)) *
                               ((Real)(n - j)) * loss_[n - j];
                }
            }
            loss_.push_back(res * pC_ / (pdSum_ * ((Real)(n + 1))));
        }
    }
}
