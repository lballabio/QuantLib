/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011, 2012 Klaus Spanderen

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
    \brief Finite Differences engine for simple vpp options
*/

#ifndef quantlib_fd_simple_kluge_ou_vpp_engine_hpp
#define quantlib_fd_simple_kluge_ou_vpp_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/experimental/finitedifferences/vanillavppoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpmodelinnervalue.hpp>

namespace QuantLib {

    //! Finite-Differences engine for simple vpp options

    class YieldTermStructure;
    class KlugeExtOUProcess;

    class FdSimpleKlugeExtOUVPPEngine
        : public GenericEngine<VanillaVPPOption::arguments,
                               VanillaVPPOption::results> {
      public:
        typedef FdmExtOUJumpModelInnerValue::Shape Shape;

        FdSimpleKlugeExtOUVPPEngine(ext::shared_ptr<KlugeExtOUProcess> process,
                                    ext::shared_ptr<YieldTermStructure> rTS,
                                    ext::shared_ptr<Shape> fuelShape,
                                    ext::shared_ptr<Shape> powerShape,
                                    Real fuelCostAddon,
                                    Size tGrid = 1,
                                    Size xGrid = 50,
                                    Size yGrid = 10,
                                    Size gGrid = 20,
                                    const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        void calculate() const override;

      private:
        const ext::shared_ptr<KlugeExtOUProcess> process_;
        const ext::shared_ptr<YieldTermStructure> rTS_;
        const Real fuelCostAddon_;
        const ext::shared_ptr<Shape> fuelShape_;
        const ext::shared_ptr<Shape> powerShape_;
        const Size tGrid_, xGrid_, yGrid_, gGrid_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif



#ifndef id_70d3e7754aac2922a640dd98d27d9c4e
#define id_70d3e7754aac2922a640dd98d27d9c4e
inline bool test_70d3e7754aac2922a640dd98d27d9c4e(int* i) { return i != 0; }
#endif
