/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file zabr.hpp
    \brief ZABR functions
    Reference: Andreasen, Huge: ZABR - Expansions for the masses, Preliminary
               Version, December 2011, http://ssrn.com/abstract=1980726
*/

#ifndef quantlib_zabr_hpp
#define quantlib_zabr_hpp

#include <ql/types.hpp>
#include <ql/utilities/disposable.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <vector>

namespace QuantLib {

class ZabrModel {

  public:
    ZabrModel(Real expiryTime, Real forward, Real alpha, Real beta, Real nu, Real rho, Real gamma);

    Real localVolatility(Real f) const;
    Disposable<std::vector<Real> >
    localVolatility(const std::vector<Real> &f) const;

    Real fdPrice(Real strike) const;
    Disposable<std::vector<Real> >
    fdPrice(const std::vector<Real> &strikes) const;

    Real fullFdPrice(Real strike) const;

    Real lognormalVolatility(Real strike) const;
    Disposable<std::vector<Real> >
    lognormalVolatility(const std::vector<Real> &strikes) const;

    Real normalVolatility(Real strike) const;
    Disposable<std::vector<Real> >
    normalVolatility(const std::vector<Real> &strikes) const;

    Real forward() const { return forward_; }
    Real expiryTime() const { return expiryTime_; }
    Real alpha() const { return alpha_; }
    Real beta() const { return beta_; }
    Real nu() const { return nu_; }
    Real rho() const { return rho_; }
    Real gamma() const { return gamma_; }

  private:
    const Real expiryTime_, forward_;
    const Real alpha_, beta_, nu_, rho_,
        gamma_; // nu_ here is a tranformed version of the input nu !

    Real x(Real strike) const;
    Disposable<std::vector<Real> > x(const std::vector<Real> &strikes) const;

    Real y(Real strike) const;

    Real F(Real y, Real u) const;

    Real lognormalVolatilityHelper(Real strike, Real x) const;
    Real normalVolatilityHelper(Real strike, Real x) const;
    Real localVolatilityHelper(Real f, Real x) const;
};
}

#endif


#ifndef id_cea381bf23b5fd84ff71e527988cc08c
#define id_cea381bf23b5fd84ff71e527988cc08c
inline bool test_cea381bf23b5fd84ff71e527988cc08c(const int* i) {
    return i != nullptr;
}
#endif
