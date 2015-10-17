/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file cclgmprocess.hpp
    \brief cross currency lgm model process
           the fx processes are modeled in log spot here
           (different from BlackScholesProcess where it is mixed
           log spot / spot depending on the method)
*/

#ifndef quantlib_cclgm_process_hpp
#define quantlib_cclgm_process_hpp

#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/experimental/models/cclgmparametrization.hpp>

#include <boost/unordered_map.hpp>

namespace QuantLib {

template <class Impl, class ImplFx, class ImplLgm>
class CcLgmProcess : public StochasticProcess {
  public:
    CcLgmProcess(const boost::shared_ptr<detail::CcLgmParametrization<
                     Impl, ImplFx, ImplLgm> > &parametrization,
                 const std::vector<Handle<Quote> > &fxSpots,
                 const std::vector<Handle<YieldTermStructure> > &curves);

    //! Stochastic process interface
    Size size() const;
    Size factors() const;
    Disposable<Array> initialValues() const;
    Disposable<Array> drift(Time t, const Array &x) const;      // not available
    Disposable<Matrix> diffusion(Time t, const Array &x) const; // not available
    Disposable<Array> expectation(Time t0, const Array &x0, Time dt) const;
    Disposable<Matrix> stdDeviation(Time t0, const Array &x0, Time dt) const;
    Disposable<Matrix> covariance(Time t0, const Array &x0, Time dt) const;

    //! clear cache
    void flushCache() const;

    //! curves inspector
    Handle<YieldTermStructure> termStructure(const Size i) const {
        QL_REQUIRE(i <= n_, "term structure index ("
                                << i << ") out of range 0..." << n_);
        return curves_[i];
    }

  private:
    const boost::shared_ptr<
        detail::CcLgmParametrization<Impl, ImplFx, ImplLgm> > p_;
    const std::vector<Handle<Quote> > fxSpots_;
    const std::vector<Handle<YieldTermStructure> > curves_;
    const Size n_;

    struct cache_key {
        double t0;
        double dt;
        const bool operator==(const cache_key &o) const {
            return (t0 == o.t0) && (dt == o.dt);
        }
    };

    struct cache_hasher : std::unary_function<cache_key, std::size_t> {
        std::size_t operator()(cache_key const &x) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, x.t0);
            boost::hash_combine(seed, x.dt);
            return seed;
        }
    };

    mutable boost::unordered_map<cache_key, Array, cache_hasher> cache_e_;
    mutable boost::unordered_map<cache_key, Matrix, cache_hasher> cache_v_,
        cache_s_;
};

// inline

template <class Impl, class ImplFx, class ImplLgm>
void CcLgmProcess<Impl, ImplFx, ImplLgm>::flushCache() const {
    cache_v_.clear();
    cache_s_.clear();
    cache_e_.clear();
}

template <class Impl, class ImplFx, class ImplLgm>
inline Size CcLgmProcess<Impl, ImplFx, ImplLgm>::size() const {
    return 2 * n_ + 1;
}

template <class Impl, class ImplFx, class ImplLgm>
inline Size CcLgmProcess<Impl, ImplFx, ImplLgm>::factors() const {
    return 2 * n_ + 1;
}

template <class Impl, class ImplFx, class ImplLgm>
inline Disposable<Array>
CcLgmProcess<Impl, ImplFx, ImplLgm>::initialValues() const {
    Array res(2 * n_ + 1);
    for (Size i = 0; i < n_; ++i) {
        res[i] = fxSpots_[i]->value();
    }
    for (Size i = n_; i < 2 * n_ + 1; ++i) {
        res[i] = 0.0;
    }
    return res;
}

template <class Impl, class ImplFx, class ImplLgm>
inline Disposable<Array>
CcLgmProcess<Impl, ImplFx, ImplLgm>::drift(Time t, const Array &x) const {
    QL_FAIL("drift not implemented");
}

template <class Impl, class ImplFx, class ImplLgm>
inline Disposable<Matrix>
CcLgmProcess<Impl, ImplFx, ImplLgm>::diffusion(Time t, const Array &x) const {
    QL_FAIL("diffusion not implemented");
}

template <class Impl, class ImplFx, class ImplLgm>
inline Disposable<Array>
CcLgmProcess<Impl, ImplFx, ImplLgm>::expectation(Time t0, const Array &x0,
                                                 Time dt) const {
    cache_key k = {t0, dt};
    typename boost::unordered_map<cache_key, Array>::iterator it =
        cache_e_.find(k);
    Array res(2 * n_ + 1, 0.0);
    if (it == cache_e_.end()) {
        // fx
        for (Size i = 0; i < n_; ++i) {
            res[i] =
                std::log(curves_[i + 1]->discount(t0 + dt) /
                         curves_[i + 1]->discount(t0) *
                         curves_[0]->discount(t0) /
                         curves_[0]->discount(t0 + dt)) -
                0.5 * p_->int_sigma_i_sigma_j(i, i, t0, t0 + dt) +
                0.5 * (p_->H_i(0, t0 + dt) * p_->H_i(0, t0 + dt) *
                           p_->zeta_i(0, t0 + dt) -
                       p_->H_i(0, t0) * p_->H_i(0, t0) * p_->zeta_i(0, t0) -
                       p_->int_H_i_H_j_alpha_i_alpha_j(0, 0, t0, t0 + dt)) -
                0.5 * (p_->H_i(i + 1, t0 + dt) * p_->H_i(i + 1, t0 + dt) *
                           p_->zeta_i(i + 1, t0 + dt) -
                       p_->H_i(i + 1, t0) * p_->H_i(i + 1, t0) *
                           p_->zeta_i(i + 1, t0) -
                       p_->int_H_i_H_j_alpha_i_alpha_j(i + 1, i + 1, t0,
                                                       t0 + dt)) +
                p_->int_H_i_alpha_i_sigma_j(0, i, t0, t0 + dt) -
                p_->H_i(i + 1, t0 + dt) *
                    (-p_->int_H_i_alpha_i_alpha_j(i + 1, i + 1, t0, t0 + dt) +
                     p_->int_H_i_alpha_i_alpha_j(0, i + 1, t0, t0 + dt) -
                     p_->int_alpha_i_sigma_j(i + 1, i, t0, t0 + dt)) -
                p_->int_H_i_H_j_alpha_i_alpha_j(i + 1, i + 1, t0, t0 + dt) +
                p_->int_H_i_H_j_alpha_i_alpha_j(0, i + 1, t0, t0 + dt) -
                p_->int_H_i_alpha_i_sigma_j(i + 1, i, t0, t0 + dt);
        }
        // lgm
        for (Size i = 1; i < n_ + 1; ++i) {
            res[n_ + i] = -p_->int_H_i_alpha_i_alpha_j(i, i, t0, t0 + dt) -
                          p_->int_alpha_i_sigma_j(i, i - 1, t0, t0 + dt) +
                          p_->int_H_i_alpha_i_alpha_j(0, i, t0, t0 + dt);
        }
        cache_e_.insert(std::make_pair(k, res));
    } else {
        res = it->second;
    }
    for (Size i = 0; i < n_; ++i) {
        res[i] +=
            x0[i] + (p_->H_i(0, t0 + dt) - p_->H_i(0, t0)) * x0[n_] -
            (p_->H_i(i + 1, t0 + dt) - p_->H_i(i + 1, t0)) * x0[n_ + i + 1];
    }
    for (Size i = 0; i < n_ + 1; ++i) {
        res[n_ + i] += x0[n_ + i];
    }
    return res;
}

template <class Impl, class ImplFx, class ImplLgm>
inline Disposable<Matrix>
CcLgmProcess<Impl, ImplFx, ImplLgm>::covariance(Time t0, const Array &x0,
                                                Time dt) const {
    cache_key k = {t0, dt};
    typename boost::unordered_map<cache_key, Matrix>::iterator i =
        cache_v_.find(k);
    if (i == cache_v_.end()) {
        Matrix res(2 * n_ + 1, 2 * n_ + 1, 0.0);
        // fx-fx
        for (Size i = 0; i < n_; ++i) {
            for (Size j = 0; j <= i; ++j) {
                res[i][j] = res[j][i] =
                    // row 1
                    p_->H_i(0, t0 + dt) * p_->H_i(0, t0 + dt) *
                        p_->int_alpha_i_alpha_j(0, 0, t0, t0 + dt) -
                    2.0 * p_->H_i(0, t0 + dt) *
                        p_->int_H_i_alpha_i_alpha_j(0, 0, t0, t0 + dt) +
                    p_->int_H_i_H_j_alpha_i_alpha_j(0, 0, t0, t0 + dt) -
                    // row 2
                    p_->H_i(0, t0 + dt) * p_->H_i(j + 1, t0 + dt) *
                        p_->int_alpha_i_alpha_j(0, j + 1, t0, t0 + dt) +
                    p_->H_i(j + 1, t0 + dt) *
                        p_->int_H_i_alpha_i_alpha_j(0, j + 1, t0, t0 + dt) +
                    p_->H_i(0, t0 + dt) *
                        p_->int_H_i_alpha_i_alpha_j(j + 1, 0, t0, t0 + dt) -
                    p_->int_H_i_H_j_alpha_i_alpha_j(0, j + 1, t0, t0 + dt) -
                    // row 3
                    p_->H_i(0, t0 + dt) * p_->H_i(i + 1, t0 + dt) *
                        p_->int_alpha_i_alpha_j(0, i + 1, t0, t0 + dt) +
                    p_->H_i(i + 1, t0 + dt) *
                        p_->int_H_i_alpha_i_alpha_j(0, i + 1, t0, t0 + dt) +
                    p_->H_i(0, t0 + dt) *
                        p_->int_H_i_alpha_i_alpha_j(i + 1, 0, t0, t0 + dt) -
                    p_->int_H_i_H_j_alpha_i_alpha_j(0, i + 1, t0, t0 + dt) +
                    // row 4
                    p_->H_i(0, t0 + dt) *
                        p_->int_alpha_i_sigma_j(0, j, t0, t0 + dt) -
                    p_->int_H_i_alpha_i_sigma_j(0, j, t0, t0 + dt) +
                    // row 5
                    p_->H_i(0, t0 + dt) *
                        p_->int_alpha_i_sigma_j(0, i, t0, t0 + dt) -
                    p_->int_H_i_alpha_i_sigma_j(0, i, t0, t0 + dt) -
                    // row 6
                    p_->H_i(i + 1, t0 + dt) *
                        p_->int_alpha_i_sigma_j(i + 1, j, t0, t0 + dt) +
                    p_->int_H_i_alpha_i_sigma_j(i + 1, j, t0, t0 + dt) -
                    // row 7
                    p_->H_i(j + 1, t0 + dt) *
                        p_->int_alpha_i_sigma_j(j + 1, i, t0, t0 + dt) +
                    p_->int_H_i_alpha_i_sigma_j(j + 1, i, t0, t0 + dt) +
                    // row 8
                    p_->H_i(i + 1, t0 + dt) * p_->H_i(j + 1, t0 + dt) *
                        p_->int_alpha_i_alpha_j(i + 1, j + 1, t0, t0 + dt) -
                    p_->H_i(j + 1, t0 + dt) *
                        p_->int_H_i_alpha_i_alpha_j(i + 1, j + 1, t0, t0 + dt) -
                    p_->H_i(i + 1, t0 + dt) *
                        p_->int_H_i_alpha_i_alpha_j(j + 1, i + 1, t0, t0 + dt) +
                    p_->int_H_i_H_j_alpha_i_alpha_j(i + 1, j + 1, t0, t0 + dt) +
                    // row 9
                    p_->int_sigma_i_sigma_j(i, j, t0, t0 + dt);
            }
        }
        // fx-lgm
        for (Size i = 0; i < n_ + 1; ++i) {
            for (Size j = 0; j < n_; ++j) {
                res[j][i + n_] = res[i + n_][j] =
                    p_->H_i(0, t0 + dt) *
                        p_->int_alpha_i_alpha_j(0, i, t0, t0 + dt) -
                    p_->int_H_i_alpha_i_alpha_j(0, i, t0, t0 + dt) -
                    p_->H_i(j + 1, t0 + dt) *
                        p_->int_alpha_i_alpha_j(j + 1, i, t0, t0 + dt) +
                    p_->int_H_i_alpha_i_alpha_j(j + 1, i, t0, t0 + dt) +
                    p_->int_alpha_i_sigma_j(i, j, t0, t0 + dt);
            }
        }
        // lgm-lgm
        for (Size i = 0; i < n_ + 1; ++i) {
            for (Size j = 0; j <= i; ++j) {
                res[i + n_][j + n_] = res[j + n_][i + n_] =
                    p_->int_alpha_i_alpha_j(i, j, t0, t0 + dt);
            }
        }
        cache_v_.insert(std::make_pair(k, res));
        return res;
    } else {
        // we need to make a copy here since a disposable is returned
        Matrix tmp = i->second;
        return tmp;
    }
}

template <class Impl, class ImplFx, class ImplLgm>
inline Disposable<Matrix>
CcLgmProcess<Impl, ImplFx, ImplLgm>::stdDeviation(Time t0, const Array &x0,
                                                  Time dt) const {
    cache_key k = {t0, dt};
    typename boost::unordered_map<cache_key, Matrix>::iterator i =
        cache_s_.find(k);
    if (i == cache_s_.end()) {
        Matrix tmp =
            pseudoSqrt(covariance(t0, x0, dt), SalvagingAlgorithm::Spectral);
        cache_s_.insert(std::make_pair(k, tmp));
        return tmp;
    } else {
        // we need to make a copy here since a disposable is returned
        Matrix tmp = i->second;
        return tmp;
    }
}

// implementation

template <class Impl, class ImplFx, class ImplLgm>
CcLgmProcess<Impl, ImplFx, ImplLgm>::CcLgmProcess(
    const boost::shared_ptr<
        detail::CcLgmParametrization<Impl, ImplFx, ImplLgm> > &parametrization,
    const std::vector<Handle<Quote> > &fxSpots,
    const std::vector<Handle<YieldTermStructure> > &curves)
    : p_(parametrization), fxSpots_(fxSpots), curves_(curves), n_(p_->n()) {

    QL_REQUIRE(fxSpots_.size() == n_,
               fxSpots_.size()
                   << " fx spots given, while parametrization suggests " << n_);
    QL_REQUIRE(curves_.size() == n_ + 1,
               curves_.size()
                   << " curves given, while parametrization suggests "
                   << (n_ + 1));

    for (Size i = 0; i < n_; ++i)
        registerWith(fxSpots_[i]);
}

} // namesapce QuantLib

#endif
