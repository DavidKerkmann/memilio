/* 
* Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
*
* Authors: Elisabeth Kluth
*
* Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "abm/migration_data.h"
#include "abm/location.h"
#include "abm/random_events.h"

#include <numeric>

namespace mio
{

MigrationData::MigrationData()
    : m_trips({})
    , m_current_index(0)
{
}

Trip& MigrationData::get_next_trip()
{
    return m_trips[m_current_index];
}

TimePoint MigrationData::get_next_trip_time()
{
    return m_trips[m_current_index].migration_time;
}

void MigrationData::sort_trips()
{
}

void MigrationData::add_trip(Trip trip)
{
    m_trips.push_back(trip);
}

} // namespace mio
