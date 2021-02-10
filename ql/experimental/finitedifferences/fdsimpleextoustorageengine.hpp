/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen
 Copyright (C) 2014 Ralph Schreyer

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

/*! \file fdsimpleextoustorageengine.hpp
    \brief Finite Differences extended OU engine for simple storage options
*/

#ifndef quantlib_fd_simple_ext_ou_storage_engine_hpp
#define quantlib_fd_simple_ext_ou_storage_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/vanillastorageoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class ExtendedOrnsteinUhlenbeckProcess;
    class YieldTermStructure;

    class FdSimpleExtOUStorageEngine
        : public GenericEngine<VanillaStorageOption::arguments,
                               VanillaStorageOption::results> {
      public:
        typedef std::vector<std::pair<Time, Real> > Shape;


        FdSimpleExtOUStorageEngine(ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> p,
                                   ext::shared_ptr<YieldTermStructure> rTS,
                                   Size tGrid = 50,
                                   Size xGrid = 100,
                                   Size yGrid = Null<Size>(),
                                   ext::shared_ptr<Shape> shape = ext::shared_ptr<Shape>(),
                                   const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        void calculate() const override;

      private:
        const ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> process_;
        const ext::shared_ptr<YieldTermStructure> rTS_;
        const Size tGrid_, xGrid_, yGrid_;
        const ext::shared_ptr<Shape> shape_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif
