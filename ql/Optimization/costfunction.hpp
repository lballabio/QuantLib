// Emacs will be in -*- Mode: c++ -*-
//
// ************ DO NOT REMOVE THIS BANNER ****************
//
//  Nicolas Di Cesare <Nicolas.Dicesare@free.fr>
//  http://acm.emath.fr/~dicesare
//
//********************************************************
//
//  Cost function abstract class
//
//********************************************************

#ifndef quantlib_optimization_costfunction_h
#define quantlib_optimization_costfunction_h

/*! 
  Cost function abstract class for unconstrained optimization pb
*/

namespace QuantLib {

    namespace Optimization {

        class CostFunction {
          public:
            typedef double value_type;

            //! method to overload to compute the cost functon value in x
            virtual value_type value (const Array & x) = 0;

            //! method to overload to compute grad_f, the first derivative of the cost function with respect to x
            virtual void firstDerivative(Array & grad_f, const Array & x)	{
                value_type eps = finiteDifferenceEpsilon (), fp, fm;
                Array xx = x;
                int i, sz = x.size ();
                for (i = 0; i < sz; ++i) {
                      xx[i] += eps;
                      fp = value (xx);
                      xx[i] -= 2. * eps;
                      fm = value (xx);
                      grad_f[i] = 0.5 * (fp - fm) / eps;
                }
            }

            //! method to overload to compute grad_f, the first derivative of the cost function with respect to x and also the cost function
            virtual value_type valueAndFirstDerivative (Array & grad_f, const Array & x) {
                firstDerivative(grad_f, x);
                return value (x);
            }

            //! member to update things if required
            virtual void Update () {}

            //! Default epsilon for finite difference method : 
            virtual double finiteDifferenceEpsilon () { return 1e-8; }
        };

    }

}

#endif
