/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file treecapfloor.hpp
    \brief Cap/Floor calculated using a tree

    \fullpath
    ql/Pricers/%treecapfloor.hpp
*/

// $Id$

#ifndef quantlib_pricers_tree_cap_floor_h
#define quantlib_pricers_tree_cap_floor_h

#include <ql/Pricers/capfloorpricer.hpp>
#include <ql/ShortRateModels/model.hpp>
#include <ql/Lattices/tree.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Cap/Floor priced in a tree
        class TreeCapFloor : public
            CapFloorPricer<ShortRateModels::Model> {
          public:
            TreeCapFloor(const Handle<ShortRateModels::Model>& model,
                         Size timeSteps);

            TreeCapFloor(const Handle<ShortRateModels::Model>& model,
                         const TimeGrid& timeGrid);

            void update();
            void calculate() const;
          private:
            TimeGrid timeGrid_;
            Size timeSteps_;
            Handle<Lattices::Tree> tree_;
        };

    }

}

#endif
