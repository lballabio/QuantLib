/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file genericmodelengine.hpp
    \brief Generic option engine based on a model
*/

#ifndef quantlib_generic_model_engine_hpp
#define quantlib_generic_model_engine_hpp

#include <ql/handle.hpp>
#include <ql/pricingengine.hpp>
#include <utility>

namespace QuantLib {

    //! Base class for some pricing engine on a particular model
    /*! Derived engines only need to implement the <tt>calculate()</tt>
        method
    */
    template<class ModelType, class ArgumentsType, class ResultsType>
    class GenericModelEngine
        : public GenericEngine<ArgumentsType, ResultsType> {
      public:
        explicit GenericModelEngine(Handle<ModelType> model = Handle<ModelType>())
        : model_(std::move(model)) {
            this->registerWith(model_);
        }
        explicit GenericModelEngine(const ext::shared_ptr<ModelType>& model)
        : model_(model) {
            this->registerWith(model_);
        }
      protected:
        Handle<ModelType> model_;
    };

}


#endif



#ifndef id_87e639d8a2d6245a5c3e765fd1b41c77
#define id_87e639d8a2d6245a5c3e765fd1b41c77
inline bool test_87e639d8a2d6245a5c3e765fd1b41c77(int* i) { return i != 0; }
#endif
