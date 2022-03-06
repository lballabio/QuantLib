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

/*! \file fdg2swaptionengine.hpp
    \brief finite differences swaption engine
*/

#ifndef quantlib_fd_g2_swaption_engine_hpp
#define quantlib_fd_g2_swaption_engine_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class FdG2SwaptionEngine
        : public GenericModelEngine<G2,Swaption::arguments,Swaption::results> {
      public:
        explicit FdG2SwaptionEngine(
            const ext::shared_ptr<G2>& model,
            Size tGrid = 100, Size xGrid = 50, Size yGrid = 50,
            Size dampingSteps = 0, Real invEps = 1e-5,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        void calculate() const override;

      private:
        const Size tGrid_, xGrid_, yGrid_, dampingSteps_;
        const Real invEps_;
        const FdmSchemeDesc schemeDesc_;
    };
}
#endif


#ifndef id_ba7041e8b8c1408d7cd4ec5653e97331
#define id_ba7041e8b8c1408d7cd4ec5653e97331
inline bool test_ba7041e8b8c1408d7cd4ec5653e97331(int* i) { return i != 0; }
#endif
