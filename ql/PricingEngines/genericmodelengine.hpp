
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file genericmodelengine.hpp
    \brief Generic option engine based on a model
*/

#ifndef quantlib_generic_model_engine_h
#define quantlib_generic_model_engine_h

#include <ql/pricingengine.hpp>

namespace QuantLib {

    //! Base class for some pricing engine on a particular model
    /*! Derived engines only need to implement the <tt>calculate()</tt>
        method
    */
    template<class ModelType, class ArgumentsType, class ResultsType>
    class GenericModelEngine 
        : public GenericEngine<ArgumentsType, ResultsType>,
          public Observer {
      public:
        GenericModelEngine() {}
        GenericModelEngine(const boost::shared_ptr<ModelType>& model)
        : model_(model) {
            registerWith(model_);
        }
        void setModel(const boost::shared_ptr<ModelType>& model) {
            unregisterWith(model_);
            model_ = model;
            QL_REQUIRE(!model_.isNull(), "no adequate model given");
            registerWith(model_);
            update();
        }
        virtual void update() {
            notifyObservers();
        }
      protected:
        boost::shared_ptr<ModelType> model_;
    };

}


#endif

