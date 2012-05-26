#ifndef LMMIN_H
#define LMMIN_H

#ifdef __cplusplus
extern "C" {
#endif
// parameters for calling the high-level interface lmfit
//   ( lmfit.c provides lm_initialize_control which sets default values ):
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


// the subroutine that calculates fvec:
typedef void (lm_evaluate_ftype) (double *par, int m_dat, double *fvec,
                                  void *data, int *info);
typedef void (lm_jacobian_ftype) (double* par, int m_dat, double* fjac,
                                  void *data);
// default implementation therof, provided by lm_eval.c:
void lm_evaluate_default(double *par, int m_dat, double *fvec, void *data,
			 int *info);

// the subroutine that informs about fit progress:
typedef void (lm_print_ftype) (int n_par, double *par, int m_dat,
			       double *fvec, void *data, int iflag,
			       int iter, int nfev);
// default implementation therof, provided by lm_eval.c:
void lm_print_default(int n_par, double *par, int m_dat, double *fvec,
		      void *data, int iflag, int iter, int nfev);


// compact high-level interface:
void lm_initialize_control(lm_control_type * control);
void lm_minimize(int m_dat, int n_par, double *par,
                 lm_evaluate_ftype * evaluate, lm_print_ftype * printout,
                 lm_jacobian_ftype * jacobian,
                 void *data, lm_control_type * control,
                 double* jacobianMatrix);
                 

double lm_enorm(int, double *);

// low-level interface for full control:
void lm_lmdif(int m, int n, double *x, double *fvec, double ftol,
              double xtol, double gtol, int maxfev, double epsfcn,
              double *diag, int mode, double factor, int *info, int *nfev,
              double *fjac, int *ipvt, double *qtf, double *wa1,
              double *wa2, double *wa3, double *wa4,
              lm_evaluate_ftype * evaluate, lm_print_ftype * printout,
              lm_jacobian_ftype * jacobian,
              void *data);


#ifndef _LMDIF
extern const char *lm_infmsg[];
extern const char *lm_shortmsg[];
#endif

#ifdef __cplusplus
}
#endif

#endif // LMMIN_H
