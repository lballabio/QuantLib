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

    //! Finite-differences pricing engine for BSM one asset options
    /*! The name is a misnomer as this is a base class for any finite
        difference scheme.  Its main job is to handle grid layout.

        \ingroup vanillaengines
    */
    class FDVanillaEngine {
      public:
        FDVanillaEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                        Size timeSteps,
                        Size gridPoints,
                        bool timeDependent = false)
        : process_(std::move(process)), timeSteps_(timeSteps), gridPoints_(gridPoints),
          timeDependent_(timeDependent), intrinsicValues_(gridPoints), BCs_(2) {}
        virtual ~FDVanillaEngine() = default;
        // accessors
        const Array& grid() const { return intrinsicValues_.grid(); }
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
        mutable SampledCurve intrinsicValues_;
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

    template <typename base, typename engine>
    class FDEngineAdapter : public base, public engine {
      public:
        FDEngineAdapter(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : base(process, timeSteps, gridPoints,timeDependent) {
            this->registerWith(process);
        }
      private:
        using base::calculate;
        void calculate() const override {
            base::setupArguments(&(this->arguments_));
            base::calculate(&(this->results_));
        }
    };

}

#endif


#ifndef id_6db881fc4ff1cee913bf6b0f465f9f1c
#define id_6db881fc4ff1cee913bf6b0f465f9f1c
inline bool test_6db881fc4ff1cee913bf6b0f465f9f1c(int* i) { return i != 0; }
#endif
