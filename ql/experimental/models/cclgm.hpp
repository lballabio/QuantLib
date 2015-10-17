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

/*! \file cclgm.hpp
    \brief multicurrency lgm model with generic parametrization

    Reference: Lichters, Stamm, Gallagher: Modern Derivatives Pricing
               and Credit Exposure Analysis, Palgrave Macmillan, 2015
*/

#ifndef quantlib_multicurrency_lgm_hpp
#define quantlib_multicurrency_lgm_hpp

#include <ql/experimental/models/lgm.hpp>
#include <ql/experimental/models/cclgmparametrization.hpp>
#include <ql/experimental/models/cclgmprocess.hpp>

namespace QuantLib {

template <class Impl, class ImplFx, class ImplLgm>
class CcLgm : public virtual Observer, public virtual Observable {
  public:
    const boost::shared_ptr<
        detail::CcLgmParametrization<Impl, ImplFx, ImplLgm> >
    parametrization() const {
        return parametrization_;
    }

    const boost::shared_ptr<Lgm<ImplLgm> > model(const Size i) const {
        return models_[i];
    }

    const Handle<YieldTermStructure> termStructure(const Size i) const {
        return boost::static_pointer_cast<CcLgmProcess<Impl, ImplFx, ImplLgm> >(
                   stateProcess())
            ->termStructure(i);
    }

    const Size n() const { return n_; }

    boost::shared_ptr<StochasticProcess> stateProcess() const {
        return process_;
    }

  protected:
    CcLgm(const std::vector<boost::shared_ptr<Lgm<ImplLgm> > > &models);

  protected:
    void
    setParametrization(const boost::shared_ptr<detail::CcLgmParametrization<
                           Impl, ImplFx, ImplLgm> > parametrization) {
        parametrization_ = parametrization;
        QL_REQUIRE(parametrization->n() == n(),
                   "parametrization's dimension (n="
                       << parametrization->n()
                       << ") is inconsistent with the number of models ("
                       << n() + 1 << "=n+1)");
        parametrization_->update();
    }

    void update() {
        parametrization_->update();
        boost::static_pointer_cast<CcLgmProcess<Impl, ImplFx, ImplLgm> >(
            stateProcess())
            ->flushCache();
        this->notifyObservers();
    }

    boost::shared_ptr<StochasticProcess> process_;

  private:
    Size n_;
    boost::shared_ptr<detail::CcLgmParametrization<Impl, ImplFx, ImplLgm> >
        parametrization_;
    const std::vector<boost::shared_ptr<Lgm<ImplLgm> > > models_;
};

// implementation

template <class Impl, class ImplFx, class ImplLgm>
CcLgm<Impl, ImplFx, ImplLgm>::CcLgm(
    const std::vector<boost::shared_ptr<Lgm<ImplLgm> > > &models)
    : n_(models.size() - 1), models_(models) {
    QL_REQUIRE(models.size() >= 2, "at least two models ("
                                       << models.size() << ") must be given");
    for (Size i = 1; i < models.size(); ++i) {
        QL_REQUIRE(models_[i]->termStructure()->referenceDate() ==
                       models_[0]->termStructure()->referenceDate(),
                   "model #" << i << " has a different reference date ("
                             << models_[i]->termStructure()->referenceDate()
                             << ") than model #0 ("
                             << models_[0]->termStructure()->referenceDate());
    }
    for (Size i = 0; i < models.size(); ++i) {
        registerWith(models_[i]);
    }
}

} // namespace QuantLib

#endif
