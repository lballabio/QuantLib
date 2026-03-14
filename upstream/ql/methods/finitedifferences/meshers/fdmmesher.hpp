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

/*! \file fdmmesher.hpp
    \brief mesher for a fdm grid
*/
#ifndef quantlib_fdm_mesher_hpp
#define quantlib_fdm_mesher_hpp

#include <ql/math/array.hpp>
#include <ql/shared_ptr.hpp>
#include <utility>
#include <vector>

namespace QuantLib {
    class FdmLinearOpLayout;
    class FdmLinearOpIterator;

    class FdmMesher {
      public:
        explicit FdmMesher(ext::shared_ptr<FdmLinearOpLayout> layout)
        : layout_(std::move(layout)) {}
        virtual ~FdmMesher() = default;

        virtual Real dplus(const FdmLinearOpIterator& iter,
                           Size direction)  const = 0;
        virtual Real dminus(const FdmLinearOpIterator& iter,
                            Size direction) const = 0;
        virtual Real location(const FdmLinearOpIterator& iter ,
                              Size direction) const = 0;
        virtual Array locations(Size direction) const = 0;

        const ext::shared_ptr<FdmLinearOpLayout>& layout() const {
            return layout_;
        }

      protected:
        const ext::shared_ptr<FdmLinearOpLayout> layout_;
    };
}

#endif
