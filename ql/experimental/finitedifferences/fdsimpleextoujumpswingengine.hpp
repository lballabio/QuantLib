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

/*! \file fdsimpleextoujumpswingengine.hpp
    \brief Finite Differences engine for simple swing options
*/

#ifndef quantlib_fd_simple_ext_ou_jump_swing_engine_hpp
#define quantlib_fd_simple_ext_ou_jump_swing_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/vanillaswingoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpmodelinnervalue.hpp>

namespace QuantLib {

    //! Finite-Differences engine for simple swing options

    class YieldTermStructure;
    class ExtOUWithJumpsProcess;

    class FdSimpleExtOUJumpSwingEngine
        : public GenericEngine<VanillaSwingOption::arguments,
                               VanillaSwingOption::results> {
      public:
        typedef FdmExtOUJumpModelInnerValue::Shape Shape;

        FdSimpleExtOUJumpSwingEngine(
            std::shared_ptr<ExtOUWithJumpsProcess> p,
            std::shared_ptr<YieldTermStructure> rTS,
            Size tGrid = 50,
            Size xGrid = 200,
            Size yGrid = 50,
            std::shared_ptr<Shape> shape = std::shared_ptr<Shape>(),
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        void calculate() const override;

      private:
        const std::shared_ptr<ExtOUWithJumpsProcess> process_;
        const std::shared_ptr<YieldTermStructure> rTS_;
        const std::shared_ptr<Shape> shape_;
        const Size tGrid_, xGrid_, yGrid_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif

