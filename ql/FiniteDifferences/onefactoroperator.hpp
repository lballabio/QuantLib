/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file onefactoroperator.hpp
    \brief general differential operator for one-factor interest rate models

    \fullpath
    ql/FiniteDifferences/%onefactoroperator.hpp
*/

// $Id$

#ifndef quantlib_finite_differences_one_factor_operator_h
#define quantlib_finite_differences_one_factor_operator_h

#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/InterestRateModelling/shortrateprocess.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! Interest-rate single factor model differential operator
        class OneFactorOperator : public TridiagonalOperator {
          public:
            OneFactorOperator() {}
            OneFactorOperator(
                const Array& grid,
                const Handle<InterestRateModelling::ShortRateProcess>& process);
            virtual ~OneFactorOperator() {}

            class SpecificTimeSetter : public TridiagonalOperator::TimeSetter{
              public:
                SpecificTimeSetter(double x0, double dx,
                    const Handle<InterestRateModelling::ShortRateProcess>& 
                        process);
                virtual ~SpecificTimeSetter() {}
                virtual void setTime(Time t, TridiagonalOperator& L) const;
              private:
                double x0_;
                double dx_;
                const Handle<InterestRateModelling::ShortRateProcess>& process_;
            };
        };

    }

}


#endif
