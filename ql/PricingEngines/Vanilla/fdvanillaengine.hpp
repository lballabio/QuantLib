/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002-2005 StatPro Italia srl
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdvanillaengine.hpp
    \brief Finite-differences vanilla-option engine
*/

#ifndef quantlib_fd_vanilla_engine_hpp
#define quantlib_fd_vanilla_engine_hpp


#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/FiniteDifferences/boundarycondition.hpp>
#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Math/sampledcurve.hpp>
#include <ql/Instruments/oneassetoption.hpp>

namespace QuantLib {

    //! Finite-differences pricing engine for BSM one asset options
    /*! \ingroup vanillaengines 
      
    The name is a misnomer as this is a base class for any finite difference
    scheme.  It's main job is to handle grid layout. */

    class FDVanillaEngine {
      public:
        FDVanillaEngine(Size timeSteps, Size gridPoints,
                        bool timeDependent = false)
        : timeSteps_(timeSteps), gridPoints_(gridPoints),
          timeDependent_(timeDependent), 
          intrinsicValues_(gridPoints), BCs_(2) {}
        virtual ~FDVanillaEngine() {};
        // accessors
        const Array& grid() const { return intrinsicValues_.grid(); }
      protected:
        // methods
        virtual void setupArguments(const OneAssetOption::arguments* args) 
            const;
        virtual void setGridLimits() const;
        virtual void setGridLimits(Real, Time) const;
        virtual void initializeGrid() const;
        virtual void initializeInitialCondition() const;
        virtual void initializeOperator() const;
        virtual Time getResidualTime() const;
        // removed - replace with getProcess()->time(d) const
        //        virtual Time getYearFraction(const Date &d) const;
        // data
        Size timeSteps_, gridPoints_;
        bool timeDependent_;
        mutable boost::shared_ptr<BlackScholesProcess> process_;
        mutable Real requiredGridValue_;
        mutable Date exerciseDate_;
        mutable boost::shared_ptr<Payoff> payoff_;
        mutable TridiagonalOperator finiteDifferenceOperator_;
        mutable SampledCurve intrinsicValues_;
        typedef BoundaryCondition<TridiagonalOperator> bc_type;
        mutable std::vector<boost::shared_ptr<bc_type> > BCs_;
        // temporaries
        mutable Real sMin_, center_, sMax_;
      protected:
      private:
        // temporaries
        mutable Real gridLogSpacing_;
        Size safeGridPoints(Size gridPoints,
                            Time residualTime) const;
        void insureStrikeInGrid() const;
        static const Real safetyZoneFactor_;
    };

}


#endif
