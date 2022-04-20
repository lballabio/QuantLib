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

#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/legacy/libormarketmodels/lfmhullwhiteparam.hpp>
#include <ql/math/functional.hpp>

namespace QuantLib {

    LfmHullWhiteParameterization::LfmHullWhiteParameterization(
        const ext::shared_ptr<LiborForwardModelProcess> & process,
        const ext::shared_ptr<OptionletVolatilityStructure> & capletVol,
        const Matrix& correlation, Size factors)
    : LfmCovarianceParameterization(process->size(), factors),
      diffusion_  (size_-1, factors_),
      fixingTimes_(process->fixingTimes()) {

        Matrix sqrtCorr(size_-1, factors_, 1.0);
        if (correlation.empty()) {
            QL_REQUIRE(factors_ == 1,
                       "correlation matrix must be given for "
                       "multi factor models");
        } else {
            QL_REQUIRE(   correlation.rows() == size_-1
                       && correlation.rows() == correlation.columns(),
                       "wrong dimesion of the correlation matrix");

            QL_REQUIRE(factors_ <= size_-1,
                       "too many factors for given LFM process");

            Matrix tmpSqrtCorr = pseudoSqrt(correlation,
                                            SalvagingAlgorithm::Spectral);

            // reduce to n factor model
            // "Reconstructing a valid correlation matrix from invalid data"
            // (<http://www.quarchome.org/correlationmatrix.pdf>)
            for (Size i=0; i < size_-1; ++i) {
                std::transform(
                    tmpSqrtCorr[i], tmpSqrtCorr[i]+factors_, sqrtCorr[i],
                    divide_by<Real>(std::sqrt(std::inner_product(
                                     tmpSqrtCorr[i],tmpSqrtCorr[i]+factors_,
                                     tmpSqrtCorr[i], 0.0))));
            }
        }

        std::vector<Volatility> lambda;
        const std::vector<Time> fixingTimes = process->fixingTimes();
        const std::vector<Date> fixingDates = process->fixingDates();

        for (Size i = 1; i < size_; ++i) {
            Real cumVar = 0.0;
            for (Size j = 1; j < i; ++j) {
                cumVar +=  lambda[i-j-1] * lambda[i-j-1]
                         * (fixingTimes[j+1] - fixingTimes[j]);
            }

            const Volatility vol =  capletVol->volatility(fixingDates[i], 0.0);
            const Volatility var = vol * vol
                * capletVol->dayCounter().yearFraction(fixingDates[0],
                                                       fixingDates[i]);

            lambda.push_back(std::sqrt(  (var - cumVar)
                                       / (fixingTimes[1] - fixingTimes[0])) );

            for (Size q=0; q<factors_; ++q) {
                diffusion_[i-1][q] = sqrtCorr[i-1][q] * lambda.back();
            }
        }

        covariance_ = diffusion_ * transpose(diffusion_);
    }


    Size LfmHullWhiteParameterization::nextIndexReset(Time t) const {
        return std::upper_bound(fixingTimes_.begin(), fixingTimes_.end(), t)
                 - fixingTimes_.begin();
    }


    Matrix LfmHullWhiteParameterization::diffusion(Time t, const Array&) const {
        Matrix tmp(size_, factors_, 0.0);
        const Size m = nextIndexReset(t);

        for (Size k=m; k<size_; ++k) {
            for (Size q=0; q<factors_; ++q) {
                tmp[k][q] = diffusion_[k-m][q];
            }
        }
        return tmp;
    }

    Matrix LfmHullWhiteParameterization::covariance(Time t, const Array&) const {
        Matrix tmp(size_, size_, 0.0);
        const Size m = nextIndexReset(t);

        for (Size k=m; k<size_; ++k) {
            for (Size i=m; i<size_; ++i) {
                tmp[k][i] = covariance_[k-m][i-m];
            }
        }

        return tmp;
    }

    Matrix LfmHullWhiteParameterization::integratedCovariance(Time t, const Array&) const {

        Matrix tmp(size_, size_, 0.0);

        Size last = std::lower_bound(fixingTimes_.begin(),
                                        fixingTimes_.end(), t)
                      - fixingTimes_.begin();

        for (Size i=0; i<last; ++i) {
            const Time dt = ((i+1<last)? fixingTimes_[i+1] : t )
                - fixingTimes_[i];

            for (Size k=i; k<size_-1; ++k) {
                for (Size l=i; l<size_-1; ++l) {
                    tmp[k+1][l+1]+= covariance_[k-i][l-i]*dt;
                }
            }
        }

        return tmp;
    }

}

