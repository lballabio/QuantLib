/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmmesherintegral.hpp
    \brief mesher based integral over target function.
*/

#ifndef quantlib_fdm_mesher_integral_hpp
#define quantlib_fdm_mesher_integral_hpp

#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/functional.hpp>

namespace QuantLib {
    class FdmMesherIntegral {
      public:
        FdmMesherIntegral(
            const ext::shared_ptr<FdmMesherComposite>& mesher,
            const std::function<Real(const Array&, const Array&)>&
                integrator1d);

        Real integrate(const Array& f) const;

      private:
        const std::vector<ext::shared_ptr<Fdm1dMesher> > meshers_;
        const std::function<Real(const Array&, const Array&)>& integrator1d_;
    };
}
#endif
