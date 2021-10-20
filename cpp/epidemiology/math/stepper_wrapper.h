#ifndef STEPPER_WRAPPER_H_
#define STEPPER_WRAPPER_H_

#include "epidemiology/math/integrator.h"

// functions and operators neccessary for a Contolled Stepper to work with Eigen::VectorXd
// these have to be declared *before* the includes

namespace std {
Eigen::VectorXd abs(Eigen::VectorXd x) {
    // elementwise operations are defined on arrays within Eigen
    // casts to and from array supposedly cost no runtime when using compiler optimisation 
    return x.array().abs().matrix();
}
}

#include <boost/numeric/odeint/algebra/vector_space_algebra.hpp>
#include <boost/numeric/odeint/stepper/controlled_runge_kutta.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta4.hpp>
#include <boost/numeric/odeint/stepper/adams_bashforth_moulton.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta_fehlberg78.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta_cash_karp54.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta_dopri5.hpp>
#include <boost/numeric/odeint/stepper/bulirsch_stoer.hpp>
//#include <boost/numeric/odeint/stepper/base/explicit_stepper_base.hpp>
//#include <boost/boost/numeric/odeint/algebra/algebra_dispatcher.hpp>
//#include <boost/boost/numeric/odeint/algebra/operations_dispatcher.hpp>



Eigen::VectorXd operator+ (const double s, const Eigen::VectorXd& v) {
    return (v.array() + s).matrix();
}

Eigen::VectorXd operator/ (const Eigen::VectorXd& v, const Eigen::VectorXd& w) {
    return (v.array() / w.array()).matrix();
}

namespace boost { namespace numeric { namespace odeint {
// create struct specialization for Eigen::VectorXd of
// the l-infinity norm used by controlled_runge_kutter
template<>
struct vector_space_norm_inf<Eigen::VectorXd>
{
    typedef double result_type; // = Eigen::VectorXd::Scalar
    double operator() (Eigen::VectorXd x) const
    {
        return x.lpNorm<Eigen::Infinity>();
    }
};
}}} // namespace boost::numeric::odeint

// Wrappers implementing IntegratorCore for boost::numeric::odeint Steppers (Controlled, Explicit and adams_bashforth_moulton)

namespace epi {

template<
    size_t Steps,
    template<
        size_t InternalSteps = Steps,
        class State = Eigen::VectorXd,
        class Value = double,
        class Deriv = State,
        class Time = Value,
        class Algebra = boost::numeric::odeint::vector_space_algebra,
        class Operations = typename boost::numeric::odeint::operations_dispatcher<State>::operations_type,
        class Resizer = boost::numeric::odeint::never_resizer,
        class InitializingStepper = boost::numeric::odeint::runge_kutta4<State, Value, Deriv, Time, Algebra, Operations, Resizer>
    > class ABMStepper
>
class ABMStepperWrapper : public epi::IntegratorCore {
public:
    bool step(const epi::DerivFunction& f, Eigen::Ref<Eigen::VectorXd const> yt, double& t, double& dt,
              Eigen::Ref<Eigen::VectorXd> ytp1) const override {
        // reorder arguments of the DerivFunction f for the stepper
        std::function<void(const Eigen::VectorXd& x, Eigen::VectorXd& dxdt, double t)> sys =
            [&](const Eigen::VectorXd& x, Eigen::VectorXd& dxdt, double t){
                dxdt.resizeLike(x); // do_step calls sys with a vector of size 0 for some reason
                f(x, t, dxdt);
            };
        // copy y(t) to dxdt, since we use the scheme do_step(sys, inout, t, dt) with sys=f, inout=y(t) for
        // in-place computation - also, this form is shared by several (all?) steppers in boost
        Eigen::VectorXd dxdt = yt.eval();
        /* ABM has Steps internal states. according to the odeint guide on boost.org the stepper
         * should be called multiple times, but this loop gives incorrect results for the ode
        m_stepper.reset();
        for (size_t i = 0; i <= Steps; i++) {
            m_stepper.do_step(sys, dxdt, t, dt / Steps);
            //m_stepper.do_step(sys, dxdt, t, dt);
        }*/
        m_stepper.do_step(sys, dxdt, t, dt);
        ytp1 = dxdt;
        t += dt;
        return true; // no step size adaption
    }
private:
    mutable ABMStepper<> m_stepper;
};

template<
    template<
        class State = Eigen::VectorXd,
        class Value = double,
        class Deriv = State,
        class Time = double,
        class Algebra = boost::numeric::odeint::vector_space_algebra,
        class Operations = typename boost::numeric::odeint::operations_dispatcher<State>::operations_type,
        class Resizer = boost::numeric::odeint::never_resizer
    > class ExplicitStepper
>
class ExplicitStepperWrapper : public epi::IntegratorCore {
public:
    bool step(const epi::DerivFunction& f, Eigen::Ref<Eigen::VectorXd const> yt, double& t, double& dt,
              Eigen::Ref<Eigen::VectorXd> ytp1) const override {
        // reorder arguments of the DerivFunction f for the stepper
        std::function<void(const Eigen::VectorXd& x, Eigen::VectorXd& dxdt, double t)> sys =
            [&](const Eigen::VectorXd& x, Eigen::VectorXd& dxdt, double t){
                dxdt.resizeLike(x); // do_step calls sys with a vector of size 0 for some reason
                f(x, t, dxdt);
            };
        // copy y(t) to dxdt, since we use the scheme do_step(sys, inout, t, dt) with sys=f, inout=y(t) for
        // in-place computation - also, this form is shared by several (all?) steppers in boost
        Eigen::VectorXd dxdt = yt.eval();
        m_stepper.do_step(sys, dxdt, t, dt);
        ytp1 = dxdt;
        t += dt;
        return true; // no step size adaption
    }
private:
    mutable ExplicitStepper<> m_stepper;
};

template<
    template<
        class State = Eigen::VectorXd,
        class Value = double,
        class Deriv = State,
        class Time = double,
        class Algebra = boost::numeric::odeint::vector_space_algebra,
        class Operations = typename boost::numeric::odeint::operations_dispatcher<State>::operations_type,
        class Resizer = boost::numeric::odeint::never_resizer
    > class ControlledStepper
>
class ControlledStepperWrapper : public epi::IntegratorCore {
public:
    ControlledStepperWrapper(double abs_tol = 1e-6, double rel_tol = 1e-6) :
        m_dt_min(std::numeric_limits<double>::min()),
        m_stepper(boost::numeric::odeint::default_error_checker<
                typename ControlledStepper<>::value_type,
                typename ControlledStepper<>::algebra_type,
                typename ControlledStepper<>::operations_type
            > (abs_tol, rel_tol)
        )
        // for more options see: boost/boost/numeric/odeint/stepper/controlled_runge_kutta.hpp
    {}
    bool step(const epi::DerivFunction& f, Eigen::Ref<Eigen::VectorXd const> yt, double& t, double& dt,
              Eigen::Ref<Eigen::VectorXd> ytp1) const override {
        // reorder arguments of the DerivFunction f for the stepper
        std::function<void(const Eigen::VectorXd& x, Eigen::VectorXd& dxdt, double t)> sys =
            [&](const Eigen::VectorXd& x, Eigen::VectorXd& dxdt, double t){
                dxdt.resizeLike(x); // do_step calls sys with a vector of size 0 for some reason
                f(x, t, dxdt);
            };
        // copy y(t) to dxdt, since we use the scheme try_step(sys, inout, t, dt) with sys=f, inout=y(t) for
        // in-place computation. This is similiar to do_step, but it updates t and dt
        Eigen::VectorXd dxdt = yt.eval();
        const double t_old = t; // t is updated by try_step on a successfull step
        while (t == t_old && dt > m_dt_min) {
            m_stepper.try_step(sys, dxdt, t, dt);
        }
        ytp1 = dxdt;
        return dt > m_dt_min;
    }
private:
    const double m_dt_min;
    mutable boost::numeric::odeint::controlled_runge_kutta<ControlledStepper<>> m_stepper;
};

} // namespace epi

#endif