#include "ql/InterestRateModelling/model.hpp"

#include "ql/Math/matrix.hpp"
#include "ql/Optimization/leastsquare.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        using Math::Matrix;
        using Optimization::LeastSquareProblem;
        using Optimization::NonLinearLeastSquare;

        class Model::CalibrationProblem : public LeastSquareProblem {
          public:
            CalibrationProblem(Handle<Model>& model, 
                std::vector<Handle<CalibrationHelper> >& instruments,
                const std::vector<double>& volatilities)
            : model_(model), instruments_(instruments), 
              prices_(instruments.size()) {
                for (unsigned i=0; i<instruments_.size(); i++)
                    prices_[i] = 
                        instruments_[i]->blackPrice(volatilities[i]);
            }
            //! Destructor
            virtual ~CalibrationProblem() {}

            //! Size of the least square problem
            virtual int size() { return instruments_.size(); }

            //! return function and target values
            virtual void targetAndValue(const Array& params, Array& target, 
                Array& fct2fit) {
                target = prices_;
                model_->setParameters(params);
                for (signed i=0; i<size(); i++) {
                    fct2fit[i] = instruments_[i]->value(model_);
                }
            }

            //! return function, target and first derivatives values
            virtual void targetValueAndfirstDerivative(const Array& params, 
                Matrix& grad_fct2fit, Array& target, Array& fct2fit) { 
                target = prices_;
                model_->setParameters(params);
                Array newParams(params);
                for (signed i=0; i<size(); i++)
                    fct2fit[i] = instruments_[i]->value(model_);

                for (unsigned j=0; j<params.size(); j++) {
                    double off = 1e-6;
                    newParams[j] -= off;
                    model_->setParameters(newParams);
                    Array newValues(size());
                    for (signed i=0; i<size(); i++) 
                        newValues[i] = instruments_[i]->value(model_);
                    newParams[j] += 2.0*off;
                    model_->setParameters(newParams);
                    for (signed i=0; i<size(); i++) {
                        double diffValue = instruments_[i]->value(model_);
                        diffValue -= newValues[i];
                        grad_fct2fit[i][j] = diffValue/(2.0*off);
                    }
                    newParams[j] = params[j];
                }
            }

          private:
            Handle<Model>& model_;
            std::vector<Handle<CalibrationHelper> >& instruments_;
            Array prices_;
        };

        void Model::calibrate( 
            const Handle<Minimizer>&,
            std::vector<Handle<CalibrationHelper> > instruments,
            std::vector<double> volatilities) {

            // Accuracy of the optimization method
            double accuracy = 1e-10;// It is the square of the accuracy
            // Maximum number of iterations
            int maxiter = 10000;

            Array initialValue(4, 0.1);
            
            // Least square optimizer
            NonLinearLeastSquare lsqnonlin(accuracy,maxiter);

            // Define the least square problem
            CalibrationProblem problem(self_, instruments, volatilities);
         
            // Set initial values
            lsqnonlin.setInitialValue(Array(nbParams_, 0.1));
            // perform fitting
            Array solution = lsqnonlin.Perform(problem);

            setParameters(solution);
            cout << "Model calibrated to the following values:" << endl;
            for (unsigned i=0; i<nbParams_; i++)
                cout << i << "    " << solution[i]*100.0 << "%" << endl;
        }

    }

}
