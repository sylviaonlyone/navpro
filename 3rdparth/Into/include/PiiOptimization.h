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

#ifndef _PIIOPTIMIZATION_H
#define _PIIOPTIMIZATION_H

#include <PiiMathException.h>
#include <PiiMatrix.h>
#include "PiiOptimizationGlobal.h"

/// @file

namespace PiiOptimization
{
  /**
   * An interface for functions that can be optimized.
   */
  template <class T> class Function
  {
  public:
    virtual ~Function() {}
    /**
     * Returns the value of a fuction at a given point.
     *
     * @param params a pointer to an N-element vector containing the
     * function parameters.
     *
     * @return the value of the function at the given point
     */
    virtual T functionValue(const T* params) const = 0;
  };

  /**
   * An interface for optimizable functions that are provided with
   * gradient information. Some optimization methods such as the bfgs
   * needs the gradient in addition to the value of a function.
   */
  template <class T> class GradientFunction : public Function<T>
  {
  public:
    /**
     * Returns the gradient of an optimized fuction at the given point.
     *
     * @param params a N-element vector containing the function
     * parameters.
     *
     * @param gradient an N-element vector that will store the
     * function's gradient at the given point.
     */
    virtual void functionGradient(const T* params, T* gradient) const = 0;
  };

  /**
   * An interface for functions optimized with respect to residual
   * values. This type of function can be optimized with the
   * Levenberg-Marquardt technique.
   */
  template <class T> class ResidualFunction
  {
  public:
    virtual ~ResidualFunction() {}

    /**
     * Returns the number of functions to minimize. Typically, this
     * value is equal to the number of measurements against which a
     * model is to be fitted.
     *
     * @return number of optimized functions (M).
     */
    virtual int functionCount() const = 0;

    /**
     * Calculates residuals and stores them into @a residuals. Let us
     * denote the function to be optimized by @e f(x). We want to find
     * the parameters @e x such that @e f(x) = @e b. Suppose we are
     * given an approximation of @e x, denoted by @f$ x_0 @f$. Since
     * we don't know @e x, there is not way to calculate the real
     * error @f$x_0 - x@f$, but we can calculate @f$b-f(x_0)@f$,
     * which is called the residual. This function calculates the
     * residual value for all the functions to be optimized.
     *
     * @param params an N-element vector (@f$x_0@f$)
     *
     * @param residuals an M-element vector of residual values
     * calculated by the derived class as described above. M equals to
     * the number of functions to be minimized. This vector is filled
     * by the function.
     */
    virtual void residualValues(const T* params, T* residuals) const = 0;

    /**
     * Calculates the Jacobian matrix of the optimized functions at the
     * given position. This function does not always need to be
     * implemented as some optimizers are able to estimate the
     * Jacobian themselves. If the number of parameters is N and the
     * number of optimized functions M, the size of the Jacobian
     * matrix is N-by-M. If we denote the optimized functions by
     * @f$f_i(x)@f$, where @e x is a N-dimensional parameter vector,
     * then the Jacobian is defined as:
     *
     * @f[
     * J = - \left[ \begin{array}{ccc}
     * \frac{\partial f_1}{\partial x_1} & \cdots & \frac{\partial f_m}{\partial x_1} \\
     * \vdots & \ddots & \vdots \\
     * \frac{\partial f_1}{\partial x_n} & \cdots & \frac{\partial f_m}{\partial x_n}
     * \end{array} \right]
     * @f]
     *
     * @note The function must return a negated version of the
     * Jacobian for optimization purposes.
     *
     * @param params an N-element parameter vector
     *
     * @param jacobian a N-by-M Jacobian matrix. Must be filled by the
     * implementation.
     *
     * The default implementation does nothing.
     */
    virtual void jacobian(const T* params, PiiMatrix<T>& jacobian) const;

    /**
     * Returns @p true if the function is capable of calculating the
     * Jacobian, otherwise returns @p false. The default
     * implementation returns @p false.
     */
    virtual bool hasJacobian() const { return false; }
  };
  
  template <class T> void ResidualFunction<T>::jacobian(const T* /*params*/, PiiMatrix<T>& /*jacobian*/) const
  {}
  
  /**
   * The Broyden-Fletcher-Goldfarb-Shanno (BFGS) method is a method to
   * solve an unconstrained nonlinear optimization problem. This
   * function minimizes function F(X) of N arguments by using a quasi-
   * Newton method which is optimized to use a minimum amount of
   * memory.
   *
   * The algorithm generates the approximation of an inverse Hessian
   * matrix by using information about the last M steps of the
   * algorithm (M <= N). It lessens a required amount of memory from a
   * value of order N^2 to a value of order 2*N*M.
   *
   * @param function the function to be minimized
   *
   * @param intialParams an initial guess for the minimum point. A
   * 1-by-N matrix.
   *
   * @param epsG iteration break rule 1: @f$||G(X_t)|| <
   * \epsilon_G@f$, where @f$X_t@f$ is the value of the parameter
   * estimate at time instant t. The iteration will end once the norm
   * of the gradient vector goes below @p epsG.
   *
   * @param epsF iteration break rule 2: @f$|F(X_t) - F(X_{t-1})| \le
   * \epsilon_F \max(|F(X_t)|, |F(X_{t-1})|, 1)@f$.
   *
   * @param epsX iteration break rule 3: @f$|X_{t}-X_{t-1}| \le
   * \epsilon_X@f$.
   *
   * @param maxIterations iteration break rule 4: t > maxIterations.
   * This is the maximum number of iterations the algorithm will run
   * if none of the first three rules breaks it.
   *
   * @return the parameter values that result in the minimum value for
   * @p function.
   */
  PII_OPTIMIZATION_EXPORT PiiMatrix<double> bfgsMinimize(const GradientFunction<double>* function,
                                                         const PiiMatrix<double>& initialParams,
                                                         double epsG = 1e-8,
                                                         double epsF = 1e-8,
                                                         double epsX = 1e-8,
                                                         int maxIterations = 100);

  /**
   * The Levenberg-Marquardt is a method of non-linear optimization. 
   * It minimizes the sum of the squares of M nonlinear functions in N
   * arguments by using Jacobian and information about function
   * values. If the optimized function is not provided with Jacobian,
   * it is calculated automagically by a forward-difference
   * approximation.
   *
   * Such a minimization problem could be solved as a general
   * non-linear optimization problem (for example, using the @ref
   * lbfgsMinimize() "LBFGS" algorithm), but it is reasonable to use
   * the information about the function F structure to solve the
   * problem more effectively.
   *
   * An example of a suitable optimization problem is to fit a
   * mathematically defined geometric model to a set of measurements.
   * In such a case the M non-linear functions could be the distances
   * of each point to the model. The N optimized parameters could be
   * translation, rotation, and scaling of the model. Note that N
   * cannot exceed M.
   *
   * @param function the function to be minimized
   *
   * @param params is 1xN initial quess of parameters which are
   * optimized by the algorithm. Can't be all zeros.
   *
   * @param maxIterations maximum number of iterations
   *
   * @param ftol desired relative error in the sum of squared
   * residuals
   *
   * @param xtol desired relative error between last two
   * approximations
   *
   * @param gtol desired orthogonality between fvec and its
   * derivatives
   *
   * @param epsilon approximation step used to estimate the Jacobian
   * if it is not provided by the function.
   *
   * @param stepbound a factor that limits the size of initial
   * approximation steps. Acceptable values are about 0.1 - 100. The
   * default value seldom needs to be changed.
   */
  PII_OPTIMIZATION_EXPORT PiiMatrix<double> lmMinimize(const ResidualFunction<double>* function,
                                                       const PiiMatrix<double>& initialParams,
                                                       int maxIterations = 100,
                                                       double ftol = 1.e-14,
                                                       double xtol = 1.e-14,
                                                       double gtol = 1.e-14,
                                                       double epsilon = 1.e-14,
                                                       double stepbound = 100.0);

  /**
   * Solves the linear assignment problem. Wikipedia defines this
   * problem as follows: "There are a number of agents and a number of
   * tasks. Any agent can be assigned to perform any task, incurring
   * some cost that may vary depending on the agent-task assignment. 
   * It is required to perform all tasks by assigning exactly one
   * agent to each task in such a way that the total cost of the
   * assignment is minimized." The problem is presented as a cost
   * matrix that defines the cost of assigning agent i (row index) to
   * task j (column index).
   *
   * This implementation is an adaptation of Roy Jonker's original C++
   * implementation, which in turn is based on "A Shortest Augmenting
   * Path Algorithm for Dense and Sparse Linear Assignment Problems,"
   * Computing 38, 325-340, 1987 by R. Jonker and A. Volgenant. 
   * Specifically, the dense version of the algorithm is implemented.
   *
   * @param cost the cost matrix. This matrix must be square (the
   * number of tasks and agents must be the same). If there is a
   * different number of tasks and agents, dummy rows/columns can be
   * added with constant cost.
   *
   * @param solution an optional output value argument that will store
   * the optimal assignment upon return. The size of the matrix will
   * be 2-by-N, where N is the size of the input matrix. The first row
   * of the matrix stores the indices of columns assigned to rows in
   * the solution, and the second column the indices of rows assigned
   * to columns. In other words, solution(0,0) determines the task to
   * which agent 0 should be assigned, and solution(1,0) the agent to
   * which task 0 should be assigned. It follows that <tt>solution(0,
   * solution(1,N)) = N</tt>.
   *
   * @param duals an optional output value argument that will store
   * the dual variables of the solution (2-by-N). The first row stores
   * the row reduction numbers, and the second row the column
   * reduction numbers.
   */
  template <class Matrix>
  typename Matrix::value_type assign(const PiiRandomAccessMatrix& cost,
                                     PiiMatrix<int>* solution = 0,
                                     PiiMatrix<typename Matrix::value_type>* duals = 0);
}

#include "PiiOptimization-templates.h"

#endif //_PIIOPTIMIZATION_H
