
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
            OneFactorOperator(const Array& grid,
                const Handle<ShortRateProcess>& process);
            virtual ~OneFactorOperator() {}

            class SpecificTimeSetter : public TridiagonalOperator::TimeSetter{
              public:
                SpecificTimeSetter(double x0, double dx,
                    const Handle<ShortRateProcess>& process);
                virtual ~SpecificTimeSetter() {}
                virtual void setTime(Time t, TridiagonalOperator& L) const;
              private:
                double x0_;
                double dx_;
                const Handle<ShortRateProcess>& process_;
            };
        };

    }

}


#endif
