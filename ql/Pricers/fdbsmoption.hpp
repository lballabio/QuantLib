
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file fdbsmoption.hpp
    \brief common code for numerical option evaluation
*/

#ifndef BSM_numerical_option_pricer_h
#define BSM_numerical_option_pricer_h

#include <ql/Pricers/singleassetoption.hpp>
#include <ql/FiniteDifferences/bsmoperator.hpp>
#include <ql/FiniteDifferences/boundarycondition.hpp>

namespace QuantLib {

    //! Black-Scholes-Merton option priced numerically
    class FdBsmOption : public SingleAssetOption {
      public:
        FdBsmOption(Option::Type type, Real underlying,
                    Real strike, Spread dividendYield, Rate riskFreeRate,
                    Time residualTime, Volatility volatility, Size gridPoints);
        // accessors
        virtual void calculate() const = 0;
        Real value() const;
        Real delta() const;
        Real gamma() const;
        const Array& getGrid() const{ return grid_; }
      protected:
        // methods
        virtual void setGridLimits(Real center,
                                   Real timeDelay) const;
        virtual void initializeGrid() const;
        virtual void initializeInitialCondition() const;
        virtual void initializeOperator() const;
        // input data
        Size gridPoints_;
        // results
        mutable Real value_, delta_, gamma_;
        mutable Array grid_;
        mutable BSMOperator finiteDifferenceOperator_;
        mutable Array intrinsicValues_;
        typedef BoundaryCondition<TridiagonalOperator> BoundaryCondition;
        mutable std::vector<boost::shared_ptr<BoundaryCondition> > BCs_;
        // temporaries
        mutable Real sMin_, center_, sMax_;
      private:
        // temporaries
        mutable Real gridLogSpacing_;
        Size safeGridPoints(Size gridPoints,
                            Time residualTime);
    };

    //! This is a safety check to be sure we have enough grid points.
    #define QL_NUM_OPT_MIN_GRID_POINTS            10
    //! This is a safety check to be sure we have enough grid points.
    #define QL_NUM_OPT_GRID_POINTS_PER_YEAR        2

    // The following is a safety check to be sure we have enough grid
    // points.
    inline Size FdBsmOption::safeGridPoints(Size gridPoints, 
                                            Time residualTime) {
        return QL_MAX(gridPoints, residualTime>1.0 ?
                      static_cast<Size>((QL_NUM_OPT_MIN_GRID_POINTS +
                                         (residualTime-1.0) *
                                         QL_NUM_OPT_GRID_POINTS_PER_YEAR))
                      : QL_NUM_OPT_MIN_GRID_POINTS);
    }

}


#endif
