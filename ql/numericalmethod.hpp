
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file numericalmethod.hpp
    \brief Numerical method class
*/

#ifndef quantlib_numerical_method_h
#define quantlib_numerical_method_h

#include <ql/grid.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    class DiscretizedAsset;

    //! Numerical method (tree, finite-differences) base class
    class NumericalMethod {
      public:
        NumericalMethod(const TimeGrid& timeGrid) : t_(timeGrid) {}
        virtual ~NumericalMethod() {}

        const TimeGrid& timeGrid() const { return t_; }

        virtual void initialize(const boost::shared_ptr<DiscretizedAsset>&,
                                Time time) const = 0;

        virtual void rollback(const boost::shared_ptr<DiscretizedAsset>&,
                              Time to) const = 0;

        virtual void rollAlmostBack(const boost::shared_ptr<DiscretizedAsset>&,
                                    Time to) const = 0;
      protected:
        TimeGrid t_;
    };

}


#endif
