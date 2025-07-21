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

/*! \file fdmlinearoplayout.hpp
    \brief memory layout of a fdm linear operator
*/

#ifndef quantlib_linear_op_layout_hpp
#define quantlib_linear_op_layout_hpp

#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>
#include <functional>

namespace QuantLib {

    class FdmLinearOpLayout {
      public:
        explicit FdmLinearOpLayout(std::vector<Size> dim)
        : dim_(std::move(dim)), spacing_(dim_.size()) {
            spacing_[0] = 1;
            std::partial_sum(dim_.begin(), dim_.end()-1,
                spacing_.begin()+1, std::multiplies<>());

            size_ = spacing_.back()*dim_.back();
        }

        FdmLinearOpIterator begin() const {
            return FdmLinearOpIterator(dim_);
        }

        FdmLinearOpIterator end() const {
            return FdmLinearOpIterator(size_);
        }

        const std::vector<Size>& dim() const {
            return dim_;
        }

        const std::vector<Size>& spacing() const {
            return spacing_;
        }

        Size size() const {
            return size_;
        }

        Size index(const std::vector<Size>& coordinates) const {
            return std::inner_product(coordinates.begin(),
                                      coordinates.end(),
                                      spacing_.begin(), Size(0));
        }

        Size neighbourhood(const FdmLinearOpIterator& iterator,
                           Size i, Integer offset) const;

        Size neighbourhood(const FdmLinearOpIterator& iterator,
                           Size i1, Integer offset1,
                           Size i2, Integer offset2) const;

        // smart but sometimes too slow
        FdmLinearOpIterator iter_neighbourhood(
            const FdmLinearOpIterator& iterator, Size i, Integer offset) const;

      private:
        Size size_;
        std::vector<Size> dim_, spacing_;
    };
}

#endif
