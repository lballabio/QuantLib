/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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


#ifndef quantlib_tap_correlations_hpp
#define quantlib_tap_correlations_hpp

#include <ql/types.hpp>
#include <vector>


namespace QuantLib {
    class Matrix;
    //! Returns the Triangular Angles Parametrized correlation matrix 
    /*!  The matrix \f$ m \f$ is filled with values corresponding to 
            angles given in the \f$ angles \f$ vector. See equation (24) in 
            Parameterizing correlations: a geometric interpretation
            from Francesco Rapisarda, Damiano Brigo, Fabio Mercurio
            
            http://www.fabiomercurio.it/riskcorr.pdf

            \test
            - the correctness of the results is tested by reproducing
              known good data.
            - the correctness of the results is tested by checking
              returned values against numerical calculations.
    */
    void setTriangularAnglesParametrization(const std::vector<Real>& angles, 
                                                                Matrix& m);
    //! Returns the rank reduced Triangular Angles Parametrized correlation matrix 
    /*!  The matrix \f$ m \f$ is filled with values corresponding to 
            angles corresponding  to the 3D spherical spiral paramterized by 
            \f$ alpha \f$, \f$ t0 \f$, \f$ epsilon \f$ values. See equation (32) in 
            Parameterizing correlations: a geometric interpretation
            from Francesco Rapisarda, Damiano Brigo, Fabio Mercurio
            
            http://www.fabiomercurio.it/riskcorr.pdf

            \test
            - the correctness of the results is tested by reproducing
              known good data.
            - the correctness of the results is tested by checking
              returned values against numerical calculations.
    */
    void setTriangularAnglesParametrizationRankThree(Real alpha, Real t0, 
                                                Real epsilon, Matrix& m);
}

#endif
