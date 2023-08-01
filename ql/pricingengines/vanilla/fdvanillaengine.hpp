/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007 StatPro Italia srl
 Copyright (C) 2005 Joseph Wang

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

/*! \file fdvanillaengine.hpp
    \brief Finite-differences vanilla-option engine
*/

#ifndef quantlib_fd_vanilla_engine_hpp
#define quantlib_fd_vanilla_engine_hpp

#include <ql/math/sampledcurve.hpp>
#include <ql/methods/finitedifferences/boundarycondition.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/payoff.hpp>
#include <ql/pricingengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>


namespace QuantLib {

    /*! \deprecated Use the new finite-differences framework instead.
                    Deprecated in version 1.32.
    */
    class [[deprecated("Use the new finite-differences framework instead")]] FDVanillaEngine {
      public:
        QL_DEPRECATED_DISABLE_WARNING
        FDVanillaEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                        Size timeSteps,
                        Size gridPoints,
                        bool timeDependent = false)
        : process_(std::move(process)), timeSteps_(timeSteps), gridPoints_(gridPoints),
          timeDependent_(timeDependent), intrinsicValues_(gridPoints), BCs_(2) {}
        virtual ~FDVanillaEngine() = default;
        // accessors
        const Array& grid() const { return intrinsicValues_.grid(); }
        QL_DEPRECATED_ENABLE_WARNING
      protected:
        // methods
        virtual void setupArguments(const PricingEngine::arguments*) const;
        virtual void setGridLimits() const;
        virtual void setGridLimits(Real, Time) const;
        virtual void initializeInitialCondition() const;
        virtual void initializeBoundaryConditions() const;
        virtual void initializeOperator() const;
        virtual Time getResidualTime() const;
        // data
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_, gridPoints_;
        bool timeDependent_;
        mutable Date exerciseDate_;
        mutable ext::shared_ptr<Payoff> payoff_;
        mutable TridiagonalOperator finiteDifferenceOperator_;
        QL_DEPRECATED_DISABLE_WARNING
        mutable SampledCurve intrinsicValues_;
        QL_DEPRECATED_ENABLE_WARNING
        typedef BoundaryCondition<TridiagonalOperator> bc_type;
        mutable std::vector<ext::shared_ptr<bc_type> > BCs_;
        // temporaries
        mutable Real sMin_, center_, sMax_;

        void ensureStrikeInGrid() const;
      private:
        Size safeGridPoints(Size gridPoints,
                            Time residualTime) const;
        static const Real safetyZoneFactor_;
    };

}

#endif
