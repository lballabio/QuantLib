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

/*! \file fdmsnapshotcondition.hpp
    \brief step condition for value inspection
*/

#ifndef quantlib_fdm_snapshot_condition_hpp
#define quantlib_fdm_snapshot_condition_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>

namespace QuantLib {

    class FdmSnapshotCondition : public StepCondition<Array> {
    public:
        explicit FdmSnapshotCondition(Time t);

        void applyTo(Array& a, Time t) const override;
        Time getTime() const;       
        const Array& getValues() const;

    private:
        const Time t_;
        mutable Array values_;
    };
}
#endif
