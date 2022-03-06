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

/*! \file fdklugeextouspreadengine.hpp
    \brief FD Kluge/extended Ornstein-Uhlenbeck engine
           for a simple power-gas spread option
*/

#ifndef quantlib_fd_kluge_extou_spread_engine_hpp
#define quantlib_fd_kluge_extou_spread_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpmodelinnervalue.hpp>
#include <ql/experimental/finitedifferences/fdmexpextouinnervaluecalculator.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class ExtOUWithJumpsProcess;
    class ExtendedOrnsteinUhlenbeckProcess;

    class FdKlugeExtOUSpreadEngine
        : public GenericEngine<VanillaOption::arguments,
                               VanillaOption::results> {
      public:
          typedef FdmExtOUJumpModelInnerValue::Shape GasShape;
          typedef FdmExtOUJumpModelInnerValue::Shape PowerShape;

          FdKlugeExtOUSpreadEngine(
              ext::shared_ptr<KlugeExtOUProcess> klugeOUProcess,
              ext::shared_ptr<YieldTermStructure> rTS,
              Size tGrid = 25,
              Size xGrid = 50,
              Size yGrid = 10,
              Size uGrid = 25,
              ext::shared_ptr<GasShape> gasShape = ext::shared_ptr<GasShape>(),
              ext::shared_ptr<PowerShape> powerShape = ext::shared_ptr<PowerShape>(),
              const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

          void calculate() const override;

        private:
          const ext::shared_ptr<KlugeExtOUProcess> klugeOUProcess_;
          const ext::shared_ptr<YieldTermStructure> rTS_;
          const Size tGrid_, xGrid_, yGrid_, uGrid_;
          const ext::shared_ptr<GasShape> gasShape_;
          const ext::shared_ptr<PowerShape> powerShape_;
          const FdmSchemeDesc schemeDesc_;
    };
}

#endif


#ifndef id_1e83d33a4c4efa7160e67fd5f0e01926
#define id_1e83d33a4c4efa7160e67fd5f0e01926
inline bool test_1e83d33a4c4efa7160e67fd5f0e01926(const int* i) {
    return i != nullptr;
}
#endif
