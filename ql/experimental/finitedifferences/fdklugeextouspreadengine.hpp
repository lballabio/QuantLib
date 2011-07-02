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

/*! \file fdklugeextoutspreadengine.hpp
    \brief Finite Differences for a simple power-gas spread option
           Kluge process vs. extended Ornstein-Uhlenbeck process
*/

#ifndef quantlib_fd_kluge_extou_spread_engine_hpp
#define quantlib_fd_kluge_extou_spread_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class ExtOUWithJumpsProcess;
    class ExtendedOrnsteinUhlenbeckProcess;

    class FdKlugeExtOUSpreadEngine
        : public GenericEngine<VanillaOption::arguments,
                               VanillaOption::results> {
      public:
          FdKlugeExtOUSpreadEngine(
              Real rho,
              const boost::shared_ptr<ExtOUWithJumpsProcess>& kluge,
              const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>& extOU,
              const boost::shared_ptr<YieldTermStructure>& rTS,
              Size tGrid = 25,
              Size xGrid = 50, Size yGrid = 10, Size uGrid=25,
              const FdmSchemeDesc& schemeDesc=FdmSchemeDesc::Hundsdorfer());

        void calculate() const;

      private:
        const Real rho_;
        const boost::shared_ptr<ExtOUWithJumpsProcess> kluge_;
        const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> extOU_;
        const boost::shared_ptr<YieldTermStructure> rTS_;
        const Size tGrid_, xGrid_, yGrid_, uGrid_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif
