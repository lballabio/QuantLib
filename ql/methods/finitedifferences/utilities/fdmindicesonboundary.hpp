/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file fdmindicesonboundary.hpp
    \brief helper class to extract the indices on a boundary
*/

#ifndef quantlib_fdm_indices_on_boundary_hpp
#define quantlib_fdm_indices_on_boundary_hpp

#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>

namespace QuantLib {

    class FdmLinearOpLayout;

    class FdmIndicesOnBoundary {
      public:
        FdmIndicesOnBoundary(const ext::shared_ptr<FdmLinearOpLayout>& l,
                              Size direction, FdmDirichletBoundary::Side side);

        const std::vector<Size>& getIndices() const;
      private:
        std::vector<Size> indices_;
    };
}
#endif



#ifndef id_140724504eb4e810af861ffd2878975a
#define id_140724504eb4e810af861ffd2878975a
inline bool test_140724504eb4e810af861ffd2878975a(const int* i) {
    return i != nullptr;
}
#endif
