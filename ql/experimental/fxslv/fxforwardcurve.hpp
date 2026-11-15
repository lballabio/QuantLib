/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file fxforwardcurve.hpp
    \brief Discount curve of a currency implied by FX forward points
*/

#ifndef quantlib_fx_forward_curve_hpp
#define quantlib_fx_forward_curve_hpp

#include <ql/experimental/fxslv/conventions/fxpairconventions.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <string>
#include <vector>

namespace QuantLib {

    //! Discount curve of a currency collateralised in another, from FX forward points
    /*! FX forwards are not priced off each currency's own curve: with
        cross-currency basis, the curve of a currency collateralised in,
        say, USD differs from its own OIS curve.  Given the collateral
        currency's curve, the forward points of the pair imply the other
        currency's curve on the same collateral, so that the pair's
        forwards are repriced.  With every curve collateralised in one
        currency, the forwards of all pairs are then consistent, crosses
        included.

        The forward of tenor \f$ T \f$ is \f$ S + p_T \f$ for delivery on
        the date \f$ T \f$ after the spot date, rolled Following on the
        pair's calendar; the points are in units of the spot rate, not
        pips.  The curve is bootstrapped log-linearly in discount factors,
        with Actual/365 (Fixed) times from the as-of date; before the
        spot date it is flat, as no overnight or tom-next points are given.

        \param asOf               the as-of date, which must be the evaluation date.
        \param conventions        the pair's conventions, with its forward tenors.
        \param spot               the pair's spot rate.
        \param forwardPoints      the forward points of each forward tenor.
        \param collateralCurrency one of the pair's currencies.
        \param collateralCurve    the curve of the collateral currency.
        \return the curve of the pair's other currency, collateralised in
                the collateral currency, with extrapolation enabled.
    */
    ext::shared_ptr<YieldTermStructure>
    bootstrapFxCurve(const Date& asOf,
                     const FxPairConventions& conventions,
                     Real spot,
                     const std::vector<Real>& forwardPoints,
                     const std::string& collateralCurrency,
                     const Handle<YieldTermStructure>& collateralCurve);

}

#endif
