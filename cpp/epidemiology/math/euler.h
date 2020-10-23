#ifndef EULER_H
#define EULER_H

#include "epidemiology/secir/secir.h"
#include "epidemiology/math/integrator.h"

#include <vector>

namespace epi
{

using SecirModel1 = SecirModel<AgeGroup1>;

/**
 * @brief Simple explicit euler integration y(t+1) = y(t) + h*f(t,y) for ODE y'(t) = f(t,y)
 */
class EulerIntegratorCore : public IntegratorCore
{
public:
    /**
     * @brief Fixed step width of the integration
     *
     * @param[in] yt value of y at t, y(t)
     * @param[in,out] t current time step h=dt
     * @param[in,out] dt current time step h=dt
     * @param[out] ytp1 approximated value y(t+1)
     */
    bool step(const DerivFunction& f, Eigen::Ref<const Eigen::VectorXd> yt, double& t, double& dt,
              Eigen::Ref<Eigen::VectorXd> ytp1) const override;

    void set_abs_tolerance(double) override
    {
        log_warning("Setting absolute tolerance has no influence on an explicit Euler integrator");
    }
    void set_rel_tolerance(double) override
    {
        log_warning("Setting relative tolerance has no influence on an explicit Euler integrator");
    }
    /// sets the minimum step size (not used)
    void set_dt_min(double) override
    {
        log_warning("Setting minimum step size has no influence on an explict Euler integrator");
    }

    /// sets the minimum step size (not used)
    void set_dt_max(double) override
    {
        log_warning("Setting maximum step size has no influence on an explict Euler integrator");
    }
};

/**
 * @brief Implicit Euler integration (not generalized, adapted to SECIHURD-model)
 */
class ImplicitEulerIntegratorCore : public IntegratorCore
{
public:
    /**
     * @brief Setting up the implicit Euler integrator
     * @param params Paramters of the SECIR/SECIHURD model
     */
    ImplicitEulerIntegratorCore(SecirModel1 const& params);

    /**
     * @brief Fixed step width of the time implicit Euler time integration scheme
     *
     * @param[in] yt value of y at t, y(t)
     * @param[in,out] t current time step h=dt
     * @param[in,out] dt current time step h=dt
     * @param[out] ytp1 approximated value y(t+1)
     */
    bool step(const DerivFunction& f, Eigen::Ref<const Eigen::VectorXd> yt, double& t, double& dt,
              Eigen::Ref<Eigen::VectorXd> ytp1) const override;

    SecirModel1 const& get_secir_params() const
    {
        return m_model;
    }

    /// @param tol the required absolute tolerance for the comparison with the Fehlberg approximation (actually not really required but used in SecirSimulation constructor)
    void set_abs_tolerance(double tol) override
    {
        m_abs_tol = tol;
    }

    /// @param tol the required relative tolerance for the comparison with the Fehlberg approximation (actually not really required but used in SecirSimulation constructor)
    void set_rel_tolerance(double tol) override
    {
        m_rel_tol = tol;
    }

    /// sets the minimum step size (not used)
    void set_dt_min(double) override
    {
        log_warning("Setting minimum step size has no influence on an implicit Euler integrator");
    }

    /// sets the minimum step size (not used)
    void set_dt_max(double) override
    {
        log_warning("Setting maximum step size has no influence on an implicit Euler integrator");
    }

private:
    const SecirModel1& m_model;
    double m_abs_tol;
    double m_rel_tol;
};

} // namespace epi

#endif // EULER_H
