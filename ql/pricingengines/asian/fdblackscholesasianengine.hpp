/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*! \file fdblackscholesasianengine.hpp
    \brief Finite-Differences Black Scholes arithmentic asian option engine
*/

#ifndef quantlib_fd_black_scholes_asian_engine_hpp
#define quantlib_fd_black_scholes_asian_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    //! Finite-Differences Black Scholes arithmetic asian option engine

    /*! \ingroup vanillaengines
    */

    class GeneralizedBlackScholesProcess;
    
    class FdBlackScholesAsianEngine
        : public GenericEngine<DiscreteAveragingAsianOption::arguments,
                               DiscreteAveragingAsianOption::results> {
      public:
        // Constructor
        explicit FdBlackScholesAsianEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size tGrid = 100,
            Size xGrid = 100,
            Size aGrid = 50,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        void calculate() const override;

      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const Size tGrid_, xGrid_, aGrid_;
        const FdmSchemeDesc schemeDesc_;
    };


}

#endif


#ifndef id_d711076d47bf7f1f7e782bec9663766d
#define id_d711076d47bf7f1f7e782bec9663766d
inline bool test_d711076d47bf7f1f7e782bec9663766d(const int* i) {
    return i != nullptr;
}
#endif
