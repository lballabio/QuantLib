/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

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

/*! \file jointstochasticprocess.cpp
    \brief multi model process for hybrid products
*/

#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/processes/jointstochasticprocess.hpp>
#include <utility>

namespace QuantLib {

    JointStochasticProcess::JointStochasticProcess(
        std::vector<ext::shared_ptr<StochasticProcess> > l, Size factors)
    : l_(std::move(l)), size_(0), factors_(factors), modelFactors_(0) {

        for (const auto& iter : l_) {
            registerWith(iter);
        }

        vsize_.reserve   (l_.size()+1);
        vfactors_.reserve(l_.size()+1);

        for (const auto& iter : l_) {
            vsize_.push_back(size_);
            size_ += iter->size();

            vfactors_.push_back(modelFactors_);
            modelFactors_ += iter->factors();
        }

        vsize_.push_back(size_);
        vfactors_.push_back(modelFactors_);

        if (factors_ == Null<Size>()) {
            factors_ = modelFactors_;
        } else {
            QL_REQUIRE(factors_ <= size_, "too many factors given");
        }
    }

    Size JointStochasticProcess::size() const {
        return size_;
    }

    Size JointStochasticProcess::factors() const {
        return factors_;
    }

    Array JointStochasticProcess::slice(const Array& x,
                                        Size i) const {
        // cut out the ith process' variables
        Size n = vsize_[i+1]-vsize_[i];
        Array y(n);
        std::copy(x.begin()+vsize_[i], x.begin()+vsize_[i+1], y.begin());
        return y;
    }

    Array JointStochasticProcess::initialValues() const {
        Array retVal(size());

        for (auto iter = l_.begin(); iter != l_.end(); ++iter) {
            const Array& pInitValues = (*iter)->initialValues();

            std::copy(pInitValues.begin(), pInitValues.end(),
                      retVal.begin()+vsize_[iter - l_.begin()]);
        }

        return retVal;
    }


    Array JointStochasticProcess::drift(Time t,
                                        const Array& x) const {
        Array retVal(size());

        for (Size i=0; i < l_.size(); ++i) {

            const Array& pDrift = l_[i]->drift(t, slice(x,i));

            std::copy(pDrift.begin(), pDrift.end(),
                      retVal.begin()+vsize_[i]);
        }

        return retVal;
    }

    Array JointStochasticProcess::expectation(Time t0,
                                              const Array& x0,
                                              Time dt) const {
        Array retVal(size());

        for (Size i=0; i < l_.size(); ++i) {

            const Array& pExpectation = l_[i]->expectation(t0, slice(x0,i), dt);

            std::copy(pExpectation.begin(), pExpectation.end(),
                      retVal.begin()+ vsize_[i]);
        }

        return retVal;
    }


    Matrix JointStochasticProcess::diffusion(Time t, const Array& x) const {
        // might need some improvement in the future
        const Time dt = 0.001;
        return pseudoSqrt(covariance(t, x, dt)/dt);
    }


    Matrix JointStochasticProcess::covariance(Time t0,
                                              const Array& x0,
                                              Time dt) const {

        // get the model intrinsic covariance matrix
        Matrix retVal(size(), size(), 0.0);

        for (Size j=0; j < l_.size(); ++j) {

            const Size vs = vsize_[j];
            const Matrix& pCov = l_[j]->covariance(t0, slice(x0,j), dt);

            for (Size i=0; i < pCov.rows(); ++i) {
                std::copy(pCov.row_begin(i), pCov.row_end(i),
                          retVal.row_begin(vs+i) + vs);
            }
        }

        // add the cross model covariance matrix
        const Array& volatility = Sqrt(retVal.diagonal());
        Matrix crossModelCovar = this->crossModelCorrelation(t0, x0);

        for (Size i=0; i < size(); ++i) {
            for (Size j=0; j < size(); ++j) {
                crossModelCovar[i][j] *= volatility[i]*volatility[j];
            }
        }

        retVal += crossModelCovar;

        return retVal;
    }


    Matrix JointStochasticProcess::stdDeviation(Time t0,
                                                const Array& x0,
                                                Time dt) const {
        return pseudoSqrt(covariance(t0, x0, dt));
    }


    Array JointStochasticProcess::apply(const Array& x0,
                                        const Array& dx) const {
        Array retVal(size());

        for (Size i=0; i < l_.size(); ++i) {
            const Array& pApply = l_[i]->apply(slice(x0,i), slice(dx,i));

            std::copy(pApply.begin(), pApply.end(),
                      retVal.begin()+vsize_[i]);
        }

        return retVal;
    }

    Array JointStochasticProcess::evolve(
        Time t0, const Array& x0, Time dt, const Array& dw) const {
        Array dv(modelFactors_);

        if (   correlationIsStateDependent()
            || correlationCache_.count(CachingKey(t0, dt)) == 0) {
            Matrix cov  = covariance(t0, x0, dt);

            const Array& sqrtDiag = Sqrt(cov.diagonal());
            for (Size i=0; i < cov.rows(); ++i) {
                for (Size j=i; j < cov.columns(); ++j) {
                    const Real div = sqrtDiag[i]*sqrtDiag[j];

                    cov[i][j] = cov[j][i] = ( div > 0) ? Real(cov[i][j]/div) : 0.0;
                }
            }

            Matrix diff(size(), modelFactors_, 0.0);

            for (Size j = 0; j < l_.size(); ++j) {
                const Size vs = vsize_   [j];
                const Size vf = vfactors_[j];

                Matrix stdDev = l_[j]->stdDeviation(t0, slice(x0,j), dt);

                for (Size i=0; i < stdDev.rows(); ++i) {
                    const Volatility vol = std::sqrt(
                        std::inner_product(stdDev.row_begin(i),
                                           stdDev.row_end(i),
                                           stdDev.row_begin(i), Real(0.0)));
                    if (vol > 0.0) {
                        std::transform(stdDev.row_begin(i), stdDev.row_end(i),
                                       stdDev.row_begin(i),
                                       [=](Real x) -> Real { return x / vol; });
                    }
                    else {
                        // keep the svd happy
                        std::fill(stdDev.row_begin(i), stdDev.row_end(i),
                                  100*i*QL_EPSILON);
                    }
                }

                SVD svd(stdDev);
                const Array& s = svd.singularValues();
                Matrix w(s.size(), s.size(), 0.0);
                for (Size i=0; i < s.size(); ++i) {
                    if (std::fabs(s[i]) > std::sqrt(QL_EPSILON)) {
                        w[i][i] = 1.0/s[i];
                    }
                }

                const Matrix inv = svd.U() * w * transpose(svd.V());

                for (Size i=0; i < stdDev.rows(); ++i) {
                    std::copy(inv.row_begin(i), inv.row_end(i),
                              diff.row_begin(i+vs)+vf);
                }
            }

            Matrix rs = rankReducedSqrt(cov, factors_, 1.0,
                                        SalvagingAlgorithm::Spectral);

            if (rs.columns() < factors_) {
                // less eigenvalues than expected factors.
                // fill the rest with zero's.
                Matrix tmp = Matrix(cov.rows(), factors_, 0.0);
                for (Size i=0; i < cov.rows(); ++i) {
                    std::copy(rs.row_begin(i), rs.row_end(i),
                              tmp.row_begin(i));
                }
                rs = tmp;
            }

            const Matrix m = transpose(diff) * rs;

            if (!correlationIsStateDependent()) {
                correlationCache_[CachingKey(t0,dt)] = m;
            }
            dv = m*dw;
        }
        else {
            if (!correlationIsStateDependent()) {
                dv = correlationCache_[CachingKey(t0,dt)] * dw;
            }
        }

        this->preEvolve(t0, x0, dt, dv);


        Array retVal(size());
        for (auto iter = l_.begin(); iter != l_.end(); ++iter) {
            const Size i = iter - l_.begin();

            Array dz((*iter)->factors());
            std::copy(dv.begin()+vfactors_[i],
                      dv.begin()+vfactors_[i] + (*iter)->factors(),
                      dz.begin());
            Array x((*iter)->size());
            std::copy(x0.begin()+vsize_[i],
                      x0.begin()+vsize_[i] + (*iter)->size(),
                      x.begin());
            const Array r = (*iter)->evolve(t0, x, dt, dz);
            std::copy(r.begin(), r.end(), retVal.begin()+vsize_[i]);
        }

        return this->postEvolve(t0, x0, dt, dv, retVal);
    }

    const std::vector<ext::shared_ptr<StochasticProcess> > &
                          JointStochasticProcess::constituents() const {
        return l_;
    }

    Time JointStochasticProcess::time(const Date& date) const {
        QL_REQUIRE(!l_.empty(), "process list is empty");

        return l_[0]->time(date);
    }

    void JointStochasticProcess::update() {
        // clear all caches
        correlationCache_.clear();

        this->StochasticProcess::update();
    }
}
