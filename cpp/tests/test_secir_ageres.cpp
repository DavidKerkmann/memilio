#include "load_test_data.h"
#include "epidemiology/secir/secir.h"
#include "epidemiology/model/simulation.h"
#include <gtest/gtest.h>

TEST(TestSecir, compareAgeResWithSingleRun)
{
    double t0   = 0;
    double tmax = 50;
    double dt   = 0.1;

    double tinc = 5.2, tinfmild = 6, tserint = 4.2, thosp2home = 12, thome2hosp = 5, thosp2icu = 2, ticu2home = 8,
           ticu2death = 5;

    double cont_freq = 0.5, alpha = 0.09, beta = 0.25, delta = 0.3, rho = 0.2, theta = 0.25;

    double nb_total_t0 = 10000, nb_exp_t0 = 100, nb_inf_t0 = 50, nb_car_t0 = 50, nb_hosp_t0 = 20, nb_icu_t0 = 10,
           nb_rec_t0 = 10, nb_dead_t0 = 0;

    epi::SecirModel model(3);
    size_t nb_groups = model.parameters.get_num_groups();
    double fact      = 1.0 / (double)nb_groups;

    auto& params = model.parameters;
    for (size_t i = 0; i < nb_groups; i++) {
        params.times[i].set_incubation(tinc);
        params.times[i].set_infectious_mild(tinfmild);
        params.times[i].set_serialinterval(tserint);
        params.times[i].set_hospitalized_to_home(thosp2home);
        params.times[i].set_home_to_hospitalized(thome2hosp);
        params.times[i].set_hospitalized_to_icu(thosp2icu);
        params.times[i].set_icu_to_home(ticu2home);
        params.times[i].set_icu_to_death(ticu2death);

        model.populations[{epi::AgeGroup(i), epi::InfectionState::Exposed}] = fact * nb_exp_t0;
        model.populations[{epi::AgeGroup(i), epi::InfectionState::Carrier}] = fact * nb_car_t0;
        model.populations[{epi::AgeGroup(i), epi::InfectionState::Infected}] = fact * nb_inf_t0;
        model.populations[{epi::AgeGroup(i), epi::InfectionState::Hospitalized}] = fact * nb_hosp_t0;
        model.populations[{epi::AgeGroup(i), epi::InfectionState::ICU}] = fact * nb_icu_t0;
        model.populations[{epi::AgeGroup(i), epi::InfectionState::Recovered}] = fact * nb_rec_t0;
        model.populations[{epi::AgeGroup(i), epi::InfectionState::Dead}] = fact * nb_dead_t0;
        model.populations.set_difference_from_group_total<epi::AgeGroup>({epi::AgeGroup(i), epi::InfectionState::Susceptible},
                                                                         fact * nb_total_t0);

        params.probabilities[i].set_infection_from_contact(1.0);
        params.probabilities[i].set_carrier_infectability(1.0);
        params.probabilities[i].set_asymp_per_infectious(alpha);
        params.probabilities[i].set_risk_from_symptomatic(beta);
        params.probabilities[i].set_hospitalized_per_infectious(rho);
        params.probabilities[i].set_icu_per_hospitalized(theta);
        params.probabilities[i].set_dead_per_icu(delta);
    }

    params.apply_constraints();

    epi::ContactMatrixGroup& contact_matrix = params.get_contact_patterns();
    contact_matrix[0] = epi::ContactMatrix(Eigen::MatrixXd::Constant(nb_groups, nb_groups, fact * cont_freq));
    contact_matrix[0].add_damping(0.7, epi::SimulationTime(30.));

    auto integrator = std::make_shared<epi::RKIntegratorCore>();
    integrator->set_dt_min(0.3);
    integrator->set_dt_max(1.0);
    integrator->set_rel_tolerance(1e-4);
    integrator->set_abs_tolerance(1e-1);
    epi::TimeSeries<double> secihurd = simulate(t0, tmax, dt, model, integrator);

    // char vars[] = {'S', 'E', 'C', 'I', 'H', 'U', 'R', 'D'};
    // printf("People in\n");
    // for (size_t k = 0; k < epi::InfectionState::SecirCount; k++) {
    //     double dummy = 0;

    //     for (size_t i = 0; i < params.get_num_groups(); i++) {
    //         printf("\t %c[%d]: %.0f", vars[k], (int)i, secir[secir.size() - 1][k]);
    //         dummy += secir[secir.size() - 1][k];
    //     }

    //     printf("\t %c_otal: %.0f\n", vars[k], dummy);
    // }

    auto compare = load_test_data_csv<double>("secihurd-compare.csv");

    ASSERT_EQ(compare.size(), static_cast<size_t>(secihurd.get_num_time_points()));
    for (size_t i = 0; i < compare.size(); i++) {
        ASSERT_EQ(compare[i].size() - 1, secihurd.get_num_elements() / nb_groups) << "at row " << i;
        ASSERT_NEAR(secihurd.get_time(i), compare[i][0], 1e-10) << "at row " << i;
        for (size_t j = 1; j < compare[i].size(); j++) {
            double dummy = 0;
            for (size_t k = 0; k < nb_groups; k++) {
                dummy += secihurd.get_value(i)[j - 1 + k * (size_t)epi::InfectionState::Count];
            }
            EXPECT_NEAR(dummy, compare[i][j], 1e-10) << " at row " << i;
        }
    }
}
