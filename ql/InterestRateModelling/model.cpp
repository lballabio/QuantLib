
#include "ql/Math/matrix.hpp"
#include "ql/InterestRateModelling/model.hpp"
#include "ql/Optimization/leastsquare.hpp"
using std::cout;
using std::endl;

namespace QuantLib {

    namespace InterestRateModelling {

        using Math::Matrix;
        using Optimization::LeastSquareProblem;
        using Optimization::NonLinearLeastSquare;

        class Model::CalibrationProblem : public LeastSquareProblem {
          public:
            CalibrationProblem(Model* model, 
                std::vector<Handle<CalibrationHelper> >& instruments)
            : model_(model, false), instruments_(instruments), 
              prices_(instruments.size()) {
                for (unsigned i=0; i<instruments_.size(); i++)
                    prices_[i] = instruments_[i]->marketValue();
            }
            //! Destructor
            virtual ~CalibrationProblem() {}

            //! Size of the least square problem
            virtual int size() { return instruments_.size(); }

            //! return function and target values
            virtual void targetAndValue(const Array& params, Array& target, 
                Array& fct2fit) {
                target = prices_;
                model_->setParams(params);
                for (signed i=0; i<size(); i++) {
                    fct2fit[i] = instruments_[i]->modelValue(model_);
                }
            }

            //! return function, target and first derivatives values
            virtual void targetValueAndfirstDerivative(const Array& params, 
                Matrix& grad_fct2fit, Array& target, Array& fct2fit) { 
                target = prices_;
                model_->setParams(params);
                Array newParams(params);
                for (signed i=0; i<size(); i++)
                    fct2fit[i] = instruments_[i]->modelValue(model_);

                for (unsigned j=0; j<params.size(); j++) {
                    double off = 1e-6;
                    newParams[j] -= off;
                    model_->setParams(newParams);
                    Array newValues(size());
                    int i;
                    for (i=0; i<size(); i++) 
                        newValues[i] = instruments_[i]->modelValue(model_);
                    newParams[j] += 2.0*off;
                    model_->setParams(newParams);
                    for (i=0; i<size(); i++) {
                        double diffValue = instruments_[i]->modelValue(model_);
                        diffValue -= newValues[i];
                        grad_fct2fit[i][j] = diffValue/(2.0*off);
                    }
                    newParams[j] = params[j];
                }
            }

          private:
            Handle<Model> model_;
            std::vector<Handle<CalibrationHelper> >& instruments_;
            Array prices_;
        };

        void Model::calibrate( 
            const Handle<Minimizer>&,
            std::vector<Handle<CalibrationHelper> >& instruments) {

            // Accuracy of the optimization method
            double accuracy = 1e-5;
            // Maximum number of iterations
            int maxiter = 10000;

            // Least square optimizer
            NonLinearLeastSquare lsqnonlin(accuracy,maxiter);

            // Define the least square problem
            CalibrationProblem problem(this, instruments);
         
            // Set initial values
            std::cout << "param size " << params_.size() << endl;
            lsqnonlin.setInitialValue(Array(params_.size(), 0.1));
            // perform fitting
            Array solution = lsqnonlin.Perform(problem);

            setParams(solution);

            cout << "Model calibrated to the following values:" << endl;
            for (unsigned i=0; i<params_.size(); i++)
                cout << i << "    " << solution[i]*100.0 << "%" << endl;
        }

    }

}
