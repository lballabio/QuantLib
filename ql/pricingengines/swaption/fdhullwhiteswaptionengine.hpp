/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdhullwhiteswaptionengine.hpp
    \brief finite differences swaption engine
*/

#ifndef quantlib_fd_hull_white_swaption_engine_hpp
#define quantlib_fd_hull_white_swaption_engine_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class FdHullWhiteSwaptionEngine
        : public GenericModelEngine<HullWhite, 
                                    Swaption::arguments, Swaption::results> {
      public:
        explicit FdHullWhiteSwaptionEngine(
            const std::shared_ptr<HullWhite>& model,
            Size tGrid = 100, Size xGrid = 100,
            Size dampingSteps = 0, Real invEps = 1e-5,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        void calculate() const override;

      private:
        const Size tGrid_, xGrid_, dampingSteps_;
        const Real invEps_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif

