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

/*! \file cclgmparametrization.hpp
    \brief cross currency lgm parametrization
*/

#ifndef quantlib_cclgm_parametrization_hpp
#define quantlib_cclgm_parametrization_hpp

#include <ql/math/integrals/integral.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/patterns/curiouslyrecurring.hpp>
#include <ql/experimental/models/lgmparametrization.hpp>
#include <ql/experimental/models/lgmfxparametrization.hpp>

#include <vector>

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

namespace QuantLib {

namespace detail {

template <class Impl, class ImplFx, class ImplLgm>
class CcLgmParametrization : public CuriouslyRecurringTemplate<Impl> {
  public:
    //! constructor
    CcLgmParametrization(
        const std::vector<boost::shared_ptr<
            detail::LgmFxParametrization<ImplFx> > > &fxParametrizations,
        const std::vector<boost::shared_ptr<
            detail::LgmParametrization<ImplLgm> > > &lgmParametrizations)
        : n_(fxParametrizations.size()),
          fxParametrizations_(fxParametrizations),
          lgmParametrizations_(lgmParametrizations) {

        QL_REQUIRE(
            fxParametrizations.size() + 1 == lgmParametrizations.size(),
            "number of fx parametrizations ("
                << fxParametrizations.size()
                << ") must be equal to the number of lgm parametrizations ("
                << lgmParametrizations.size() << ") minus one");

        // if no integrator is given, we provide a simple one as default
        integrator_ = boost::make_shared<SimpsonIntegral>(1E-10, 100);
    }

    void update() const;

    //! inspectors
    const Size n() const;

    const Real H_i(const Size i, const Real t) const;
    const Real zeta_i(const Size i, const Real t) const;
    const Real alpha_i_alpha_j(const Size i, const Size j, const Real t) const;
    const Real alpha_i_sigma_j(const Size i, const Size j, const Real t) const;
    const Real sigma_i_sigma_j(const Size i, const Size j, const Real t) const;
    const Real H_i_alpha_i_alpha_j(const Size i, const Size j,
                                   const Real t) const;
    const Real H_i_H_j_alpha_i_alpha_j(const Size i, const Size j,
                                       const Real t) const;
    const Real H_i_alpha_i_sigma_j(const Size i, const Size j,
                                   const Real t) const;

    const Real int_alpha_i_alpha_j(const Size i, const Size j, const Real a,
                                   const Real b) const;
    const Real int_alpha_i_sigma_j(const Size i, const Size j, const Real a,
                                   const Real b) const;
    const Real int_sigma_i_sigma_j(const Size i, const Size j, const Real a,
                                   const Real b) const;
    const Real int_H_i_alpha_i_alpha_j(const Size i, const Size j, const Real a,
                                       const Real b) const;
    const Real int_H_i_H_j_alpha_i_alpha_j(const Size i, const Size j,
                                           const Real a, const Real b) const;
    const Real int_H_i_alpha_i_sigma_j(const Size i, const Size j, const Real a,
                                       const Real b) const;
    const Real rho_alpha_alpha(const Size i, const Size j) const;
    const Real rho_alpha_sigma(const Size i, const Size j) const;
    const Real rho_sigma_sigma(const Size i, const Size j) const;

  protected:
    //! interface (required)

    const Real rho_alpha_alpha_impl(const Size i, const Size j) const;
    const Real rho_alpha_sigma_impl(const Size i, const Size j) const;
    const Real rho_sigma_sigma_impl(const Size i, const Size j) const;

    //! interface (optional, default implementation uses numerical integration)

    const void update_impl() const;
    const Real int_alpha_i_alpha_j_impl(const Size i, const Size j,
                                        const Real a, const Real b) const;
    const Real int_alpha_i_sigma_j_impl(const Size i, const Size j,
                                        const Real a, const Real b) const;
    // for i=j this is not used, but the native variance implementation
    // in LgmFxParametrization
    const Real int_sigma_i_sigma_j_impl(const Size i, const Size j,
                                        const Real a, const Real b) const;
    const Real int_H_i_alpha_i_alpha_j_impl(const Size i, const Size j,
                                            const Real a, const Real b) const;
    const Real int_H_i_H_j_alpha_i_alpha_j_impl(const Size i, const Size j,
                                                const Real a,
                                                const Real b) const;
    const Real int_H_i_alpha_i_sigma_j_impl(const Size i, const Size j,
                                            const Real a, const Real b) const;

    //! optionally changes the integrator
    void setIntegrator(const boost::shared_ptr<Integrator> &integrator) {
        integrator_ = integrator;
    }

  private:
    const Size n_;
    std::vector<boost::shared_ptr<detail::LgmFxParametrization<ImplFx> > >
        fxParametrizations_;
    std::vector<boost::shared_ptr<detail::LgmParametrization<ImplLgm> > >
        lgmParametrizations_;
    boost::shared_ptr<Integrator> integrator_;
};

// inline

template <class Impl, class ImplFx, class ImplLgm>
void CcLgmParametrization<Impl, ImplFx, ImplLgm>::update() const {
    this->impl().update_impl();
}

template <class Impl, class ImplFx, class ImplLgm>
const Size CcLgmParametrization<Impl, ImplFx, ImplLgm>::n() const {
    return n_;
}

template <class Impl, class ImplFx, class ImplLgm>
const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::H_i(const Size i,
                                                 const Real t) const {
    return lgmParametrizations_[i]->H(t);
}

template <class Impl, class ImplFx, class ImplLgm>
const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::zeta_i(const Size i,
                                                    const Real t) const {
    return lgmParametrizations_[i]->zeta(t);
}

template <class Impl, class ImplFx, class ImplLgm>
const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::alpha_i_alpha_j(
    const Size i, const Size j, const Real t) const {
    return lgmParametrizations_[i]->alpha(t) *
           lgmParametrizations_[j]->alpha(t) * rho_alpha_alpha(i, j);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::alpha_i_sigma_j(
    const Size i, const Size j, const Real t) const {
    return lgmParametrizations_[i]->alpha(t) *
           fxParametrizations_[j]->sigma(t) * rho_alpha_sigma(i, j);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::sigma_i_sigma_j(
    const Size i, const Size j, const Real t) const {
    return fxParametrizations_[i]->sigma(t) * fxParametrizations_[j]->sigma(t) *
           rho_sigma_sigma(i, j);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::H_i_alpha_i_alpha_j(
    const Size i, const Size j, const Real t) const {
    return lgmParametrizations_[i]->H(t) * lgmParametrizations_[i]->alpha(t) *
           lgmParametrizations_[j]->alpha(t) * rho_alpha_alpha(i, j);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::H_i_H_j_alpha_i_alpha_j(
    const Size i, const Size j, const Real t) const {
    return lgmParametrizations_[i]->H(t) * lgmParametrizations_[j]->H(t) *
           lgmParametrizations_[i]->alpha(t) *
           lgmParametrizations_[j]->alpha(t) * rho_alpha_alpha(i, j);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::H_i_alpha_i_sigma_j(
    const Size i, const Size j, const Real t) const {
    return lgmParametrizations_[i]->H(t) * lgmParametrizations_[i]->alpha(t) *
           fxParametrizations_[j]->sigma(t) * rho_alpha_sigma(i, j);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_alpha_i_alpha_j(
    const Size i, const Size j, const Real a, const Real b) const {
    return this->impl().int_alpha_i_alpha_j_impl(i, j, a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_alpha_i_sigma_j(
    const Size i, const Size j, const Real a, const Real b) const {
    return this->impl().int_alpha_i_sigma_j_impl(i, j, a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_sigma_i_sigma_j(
    const Size i, const Size j, const Real a, const Real b) const {
    // use the "native" implementation if possible
    if(i==j)
        return fxParametrizations_[i]->variance(b) -
               fxParametrizations_[i]->variance(a);
    return this->impl().int_sigma_i_sigma_j_impl(i, j, a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_H_i_alpha_i_alpha_j(
    const Size i, const Size j, const Real a, const Real b) const {
    return this->impl().int_H_i_alpha_i_alpha_j_impl(i, j, a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_H_i_H_j_alpha_i_alpha_j(
    const Size i, const Size j, const Real a, const Real b) const {
    return this->impl().int_H_i_H_j_alpha_i_alpha_j_impl(i, j, a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_H_i_alpha_i_sigma_j(
    const Size i, const Size j, const Real a, const Real b) const {
    return this->impl().int_H_i_alpha_i_sigma_j_impl(i, j, a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::rho_alpha_alpha(
    const Size i, const Size j) const {
    return this->impl().rho_alpha_alpha_impl(i, j);
}
template <class Impl, class ImplFx, class ImplLgm>
inline const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::rho_alpha_sigma(
    const Size i, const Size j) const {
    return this->impl().rho_alpha_sigma_impl(i, j);
}
template <class Impl, class ImplFx, class ImplLgm>
inline const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::rho_sigma_sigma(
    const Size i, const Size j) const {
    return this->impl().rho_sigma_sigma_impl(i, j);
}

// default implementation

template <class Impl, class ImplFx, class ImplLgm>
const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::rho_alpha_alpha_impl(
    const Size i, const Size j) const {
    QL_FAIL("correlation alpha-alpha not implemented");
}
template <class Impl, class ImplFx, class ImplLgm>
const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::rho_alpha_sigma_impl(
    const Size i, const Size j) const {
    QL_FAIL("correlation alpha-sigma not implemented");
}
template <class Impl, class ImplFx, class ImplLgm>
const Real CcLgmParametrization<Impl, ImplFx, ImplLgm>::rho_sigma_sigma_impl(
    const Size i, const Size j) const {
    QL_FAIL("correlation sigma-sigma not implemented");
}

template <class Impl, class ImplFx, class ImplLgm>
const void CcLgmParametrization<Impl, ImplFx, ImplLgm>::update_impl() const {
    for (Size i = 0; i < fxParametrizations_.size(); ++i) {
        fxParametrizations_[i]->update();
    }
    for (Size i = 0; i < lgmParametrizations_.size(); ++i) {
        lgmParametrizations_[i]->update();
    }
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_alpha_i_alpha_j_impl(
    const Size i, const Size j, const Real a, const Real b) const {
    return integrator_->operator()(
        boost::bind(
            &CcLgmParametrization<Impl, ImplFx, ImplLgm>::alpha_i_alpha_j, this,
            i, j, _1),
        a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_alpha_i_sigma_j_impl(
    const Size i, const Size j, const Real a, const Real b) const {

    return integrator_->operator()(
        boost::bind(
            &CcLgmParametrization<Impl, ImplFx, ImplLgm>::alpha_i_sigma_j, this,
            i, j, _1),
        a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_sigma_i_sigma_j_impl(
    const Size i, const Size j, const Real a, const Real b) const {

    return integrator_->operator()(
        boost::bind(
            &CcLgmParametrization<Impl, ImplFx, ImplLgm>::sigma_i_sigma_j, this,
            i, j, _1),
        a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_H_i_alpha_i_alpha_j_impl(
    const Size i, const Size j, const Real a, const Real b) const {
    return integrator_->operator()(
        boost::bind(
            &CcLgmParametrization<Impl, ImplFx, ImplLgm>::H_i_alpha_i_alpha_j,
            this, i, j, _1),
        a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_H_i_H_j_alpha_i_alpha_j_impl(
    const Size i, const Size j, const Real a, const Real b) const {
    return integrator_->operator()(
        boost::bind(&CcLgmParametrization<Impl, ImplFx,
                                          ImplLgm>::H_i_H_j_alpha_i_alpha_j,
                    this, i, j, _1),
        a, b);
}

template <class Impl, class ImplFx, class ImplLgm>
inline const Real
CcLgmParametrization<Impl, ImplFx, ImplLgm>::int_H_i_alpha_i_sigma_j_impl(
    const Size i, const Size j, const Real a, const Real b) const {
    return integrator_->operator()(
        boost::bind(
            &CcLgmParametrization<Impl, ImplFx, ImplLgm>::H_i_alpha_i_sigma_j,
            this, i, j, _1),
        a, b);
}

} // namespace detail

} // namespace QuantLib

#endif
