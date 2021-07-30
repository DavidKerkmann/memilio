#include "epidemiology/abm/location.h"
#include "epidemiology/abm/person.h"
#include "epidemiology/utils/random_number_generator.h"
#include "epidemiology/abm/random_events.h"

#include <numeric>

namespace epi
{

Location::Location(LocationType type, uint32_t index)
    : m_type(type)
    , m_index(index)
    , m_subpopulations{}
    , m_cached_exposure_rate({AbmAgeGroup::Count})
{
}

InfectionState Location::interact(const Person& person, TimeSpan dt, const GlobalInfectionParameters& global_params) const
{
    auto state = person.get_infection_state();
    auto age = person.get_age();
    switch (state) {
    case InfectionState::Susceptible:
        return random_transition(state, dt, {{InfectionState::Exposed, m_cached_exposure_rate[age]}});
    case InfectionState::Carrier:
        return random_transition(
            state, dt,
            {{InfectionState::Infected_Detected,
              global_params.get<DetectInfection>()[age] * global_params.get<CarrierToInfected>()[age]},
             {InfectionState::Infected_Undetected, (1 - global_params.get<DetectInfection>()[age]) *
                                                       global_params.get<CarrierToInfected>()[age]},
             {InfectionState::Recovered_Carrier, global_params.get<CarrierToRecovered>()[age]}});
    case InfectionState::Infected_Detected: //fallthrough!
    case InfectionState::Infected_Undetected:
        return random_transition(
            state, dt,
            {{InfectionState::Recovered_Infected, global_params.get<InfectedToRecovered>()[age]},
             {InfectionState::Dead, global_params.get<InfectedToDead>()[age] * m_parameters.get<DeathFactor>()}});
    case InfectionState::Recovered_Carrier: //fallthrough!
    case InfectionState::Recovered_Infected:
        return random_transition(
            state, dt, {{InfectionState::Susceptible, global_params.get<RecoveredToSusceptible>()[age]}});
    default:
        return state; //some states don't transition
    }
}

void Location::begin_step(TimeSpan /*dt*/, const GlobalInfectionParameters& global_params)
{
    //cache for next step so it stays constant during the step while subpopulations change
    //otherwise we would have to cache all state changes during a step which uses more memory
    auto num_carriers = get_subpopulation(InfectionState::Carrier);
    auto num_infected = get_subpopulation(InfectionState::Infected_Detected) + get_subpopulation(InfectionState::Infected_Undetected);
    if (m_num_persons == 0){
        m_cached_exposure_rate = {{epi::AbmAgeGroup::Count}, 0.};
    } 
    else{
        m_cached_exposure_rate.array()
            = std::min(m_parameters.get<EffectiveContacts>(), double(m_num_persons)) / m_num_persons *
                             (global_params.get<SusceptibleToExposedByCarrier>().array() * num_carriers +
                              global_params.get<SusceptibleToExposedByInfected>().array() * num_infected);
    }
    
}


void Location::add_person(const Person& p)
{
    ++m_num_persons;
    InfectionState s = p.get_infection_state();
    change_subpopulation(s, +1);
}

void Location::remove_person(const Person& p)
{
    --m_num_persons;
    InfectionState s = p.get_infection_state();
    change_subpopulation(s, -1);
}

void Location::changed_state(const Person& p, InfectionState old_state)
{
    change_subpopulation(old_state, -1);
    change_subpopulation(p.get_infection_state(), +1);
}

void Location::change_subpopulation(InfectionState s, int delta)
{
    m_subpopulations[size_t(s)] += delta;
    assert(m_subpopulations[size_t(s)]>=0 && "subpopulations must be non-negative");
}

int Location::get_subpopulation(InfectionState s) const
{
    return m_subpopulations[size_t(s)];
}

Eigen::Ref<const Eigen::VectorXi> Location::get_subpopulations() const
{
    return Eigen::Map<const Eigen::VectorXi>(m_subpopulations.data(), m_subpopulations.size());
}


} // namespace epi
