/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_drift_calculator_hpp
#define quantlib_drift_calculator_hpp

#ifndef QL_EXTRA_SAFETY_CHECKS
#   define QL_EXTRA_SAFETY_CHECKS
#endif

#include <ql/Math/matrix.hpp>

namespace QuantLib {

    /*! <insert formula here >
    */
    class DriftCalculator {

    public:

        DriftCalculator(const Matrix& pseudo,
                        const Array& displacements,
                        const Array& taus,
                        Size numeraire,
                        Size alive);
        
		void compute(const Array& forwards,
                     Array& drifts) const;

    private:

	Matrix C_;

    Size size_, numeraire_, alive_;
    Array displacements_, taus_;
	Matrix pseudo_;

		// temporary variables
        // to be added later
    };

}


#endif
