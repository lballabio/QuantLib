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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file uniformgridmesher.hpp
    \brief uniform grid mesher
*/

#ifndef quantlib_uniform_grid_mesher_hpp
#define quantlib_uniform_grid_mesher_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>
#include <memory>

namespace QuantLib {

    class UniformGridMesher : public FdmMesher {
      public:
        UniformGridMesher(
            const ext::shared_ptr<FdmLinearOpLayout> & index,
            const std::vector<std::pair<Real, Real> > & boundaries);

        Real dplus(const FdmLinearOpIterator&, Size direction) const override {
            return dx_[direction];
        }
        Real dminus(const FdmLinearOpIterator&, Size direction) const override {
            return dx_[direction];
        }

        Real location(const FdmLinearOpIterator& iter, Size direction) const override {
            return locations_[direction][iter.coordinates()[direction]];
        }

        Array locations(Size direction) const override;

      private:
        std::unique_ptr<Real[]> dx_;
        std::vector<std::vector<Real> > locations_;
    };
}

#endif
