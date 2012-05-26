/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "PiiOptimization.h"
#include "lbfgs.h"
#include "lmmin.h"

static void lbfgsCallbackFunction(ap::real_1d_array x, double& f, ap::real_1d_array& g, void* data)
{
  PiiOptimization::GradientFunction<double>* func = reinterpret_cast<PiiOptimization::GradientFunction<double>*>(data);
  f = func->functionValue(x.getcontent());
  func->functionGradient(x.getcontent(), g.getcontent());
}

namespace PiiOptimization
{
  struct LmCallbackData
  {
    const ResidualFunction<double>* func;
    PiiMatrix<double>* matJacobian;
  };
}

static void lmCallbackFunction(double *par, int /*m_dat*/, double *fvec,
                                      void *data, int* /*info*/)
/* 
 *	par is an input array. At the end of the minimization, it contains
 *        the approximate solution vector.
 *
 *	m_dat is a positive integer input variable set to the number
 *	  of functions.
 *
 *	fvec is an output array of length m_dat which contains the function
 *        values the square sum of which ought to be minimized.
 *
 *	data is a read-only pointer to PiiOptimization::LmCallbackData.
 *
 *      info is an integer output variable. If set to a negative value, the
 *        minimization procedure will stop.
 */
{
  PiiOptimization::LmCallbackData* funcData = reinterpret_cast<PiiOptimization::LmCallbackData*>(data);
  funcData->func->residualValues(par, fvec);
}

/*
 * Parameters equal to the above, but fjac is a m-by-n Jacobian matrix
 * that needs to be filled.
 */
static void lmCallbackJacobianFunction(double *par, int /*m_dat*/, double* /*fjac*/, void *data)
{
  PiiOptimization::LmCallbackData* funcData = reinterpret_cast<PiiOptimization::LmCallbackData*>(data);
  funcData->func->jacobian(par, *funcData->matJacobian);
}


static void lmCallbackPrintFunction(int /*n_par*/, double* /*par*/, int /*m_dat*/, double* /*fvec*/,
                                    void* /*data*/, int /*iflag*/, int /*iter */, int /*nfev*/)
{
}

namespace PiiOptimization
{
  PiiMatrix<double> lmMinimize(const ResidualFunction<double>* function,
                               const PiiMatrix<double>& initialParams,
                               int maxIterations,
                               double ftol, double xtol, double gtol,
                               double epsilon, double stepBound)
  {
    PiiMatrix<double> matJacobian(function->functionCount(), initialParams.columns());
    LmCallbackData data = { function, &matJacobian };

    PiiMatrix<double> params(initialParams);
    /*
      typedef struct {
      double ftol;		// relative error desired in the sum of squares.
      double xtol;		// relative error between last two approximations.
      double gtol;		// orthogonality desired between fvec and its derivs.
      double epsilon;		// step used to calculate the jacobian.
      double stepbound;		// initial bound to steps in the outer loop.
      double fnorm;		// norm of the residue vector fvec.
      int maxcall;		// maximum number of iterations.
      int nfev;			// actual number of iterations.
      int info;			// status of minimization.
      } lm_control_type;
    */
    
    lm_control_type control;
    control.ftol = ftol;
    control.xtol = xtol;
    control.gtol = gtol;
    control.epsilon = epsilon;
    control.stepbound = stepBound;
    control.maxcall = maxIterations;
    control.info = 0;

    // If the function is provided with a Jacobian, we don't need to
    // spend time estimating it but let the callback function do the
    // task.
    lm_jacobian_ftype* jacobianCallback = function->hasJacobian() ? lmCallbackJacobianFunction : 0;
      
    lm_minimize(function->functionCount(),
                initialParams.columns(),
                params.row(0),
                lmCallbackFunction,
                lmCallbackPrintFunction,
                jacobianCallback,
                &data, &control,
                matJacobian.row(0));
    return params;
  }

  PiiMatrix<double> bfgsMinimize(const GradientFunction<double>* function,
                                 const PiiMatrix<double>& initialParams,
                                 double epsG, double epsF, double epsX,
                                 int maxIterations)
  {
    int info = 0;
    // Initialize an ap::real_1d_array data structure based on the
    // initial guess for the optimized parameters.
    ap::real_1d_array array;
    array.setbounds(1, initialParams.columns());
    for (int i=initialParams.columns(); i--; )
      array.getcontent()[i] = initialParams(i);

    // Call the low-level optimization function. Pass function as the
    // data pointer in order to get access to it in the callback
    // function.
    lbfgsminimize(initialParams.columns(),initialParams.columns(),
                  array,
                  epsG,epsF,epsX,
                  maxIterations,
                  info,
                  lbfgsCallbackFunction,
                  (void*)function);
    
    PiiMatrix<double> res(1,initialParams.columns());
    for(int i = initialParams.columns(); i--;)
      {
        res(0,i) = array(i+1);
      }
     return res;
   }
}
