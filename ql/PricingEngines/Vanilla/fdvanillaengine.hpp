
/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002-2005 StatPro Italia srl
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdvanillaengine.hpp
    \brief Finite-differences vanilla-option engine
*/

#ifndef quantlib_fd_vanilla_engine_hpp
#define quantlib_fd_vanilla_engine_hpp

#include <ql/Instruments/vanillaoption.hpp>
#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/FiniteDifferences/boundarycondition.hpp>

namespace QuantLib {

    //! Finite-differences pricing engine for BSM vanilla options
    /*! \ingroup vanillaengines */
    class FDVanillaEngine : public VanillaOption::engine {
      public:
        FDVanillaEngine(Size timeSteps, Size gridPoints,
                        bool timeDependent = false)
        : timeSteps_(timeSteps), gridPoints_(gridPoints),
          timeDependent_(timeDependent), grid_(gridPoints),
          intrinsicValues_(gridPoints), BCs_(2) {}
        // accessors
        const Array& grid() const{ return grid_; }
      protected:
        // methods
        virtual void setGridLimits() const;
        virtual void initializeGrid() const;
        virtual void initializeInitialCondition() const;
        virtual void initializeOperator() const;
        virtual Time getResidualTime() const;
        // data
        Size timeSteps_, gridPoints_;
        bool timeDependent_;
        mutable Array grid_;
        mutable TridiagonalOperator finiteDifferenceOperator_;
        mutable Array intrinsicValues_;
        typedef BoundaryCondition<TridiagonalOperator> BoundaryCondition;
        mutable std::vector<boost::shared_ptr<BoundaryCondition> > BCs_;
        // temporaries
        mutable Real sMin_, center_, sMax_;
      private:
        // temporaries
        mutable Real gridLogSpacing_;
        Size safeGridPoints(Size gridPoints,
                            Time residualTime) const;
    };

}


#endif
