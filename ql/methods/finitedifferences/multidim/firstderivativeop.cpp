/*! \file firstderivativeop.cpp
    \brief first derivative linear operator
*/

#include <ql/methods/finitedifferences/multidim/firstderivativeop.hpp>

namespace QuantLib {

    FirstDerivativeOp::FirstDerivativeOp(
                                Size direction,
                                const boost::shared_ptr<FdmMesher>& mesher)
    : TripleBandLinearOp(direction, mesher) {

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();

        for (FdmLinearOpIterator iter = layout->begin(); iter!=endIter; ++iter) {
            const Size i = iter.index();
            const Real hm = mesher->dminus(iter, direction_);
            const Real hp = mesher->dplus(iter, direction_);

            const Real zetam1 = hm*(hm+hp);
            const Real zeta0  = hm*hp;
            const Real zetap1 = hp*(hm+hp);
            
            if (iter.coordinates()[direction_] == 0) {
                //upwinding scheme
                lower_[i] = 0.0;
                diag_[i]  = -(upper_[i] = 1/hp); 
            }
            else if (   iter.coordinates()[direction_] 
                     == layout->dim()[direction]-1) {
                 // downwinding scheme
                lower_[i] = -(diag_[i] = 1/hm);
                upper_[i] = 0.0;
            }
            else {
                lower_[i] = -hp/zetam1;
                diag_[i]  = (hp-hm)/zeta0;
                upper_[i] = hm/zetap1;
            }
        }
    }
}

