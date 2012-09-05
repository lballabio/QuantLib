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

/*! \file fdmlinearopiterator.hpp
    \brief iterator for a linear fdm operator
*/

#ifndef quantlib_linear_op_iterator_hpp
#define quantlib_linear_op_iterator_hpp

#include <ql/types.hpp>
#include <ql/utilities/disposable.hpp>
#include <vector>
#include <numeric>

namespace QuantLib {

    class FdmLinearOpIterator {
      public:
        explicit FdmLinearOpIterator(Size index = 0)
        : index_(index) {}

        explicit FdmLinearOpIterator(const std::vector<Size>& dim)
        : index_(0),
          dim_(dim),
          coordinates_(dim.size(), 0) {}

        FdmLinearOpIterator(const std::vector<Size>& dim,
            const std::vector<Size>& coordinates, Size index)
        : index_(index),
          dim_(dim),
          coordinates_(coordinates) {}

        FdmLinearOpIterator(const Disposable<FdmLinearOpIterator> & from) {
            swap(const_cast<Disposable<FdmLinearOpIterator> & >(from));
        }

        void operator++() {
            ++index_;
            for (Size i=0; i < dim_.size(); ++i) {
                if (++coordinates_[i] == dim_[i]) {
                    coordinates_[i] = 0;
                }
                else {
                    break;
                }
            }
        }

        bool operator!=(const FdmLinearOpIterator& iterator) {
            return index_ != iterator.index_;
        }

        Size index() const {
            return index_;
        }

        const std::vector<Size> & coordinates() const {
            return coordinates_;
        }

        void swap(FdmLinearOpIterator& iter) {
            std::swap(iter.index_, index_);
            dim_.swap(iter.dim_);
            coordinates_.swap(iter.coordinates_);
        }

      private:
        Size index_;
        std::vector<Size> dim_;
        std::vector<Size> coordinates_;
    };
}

#endif
