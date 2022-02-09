#############################################################################
# Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
#
# Authors: Kathrin Rack, Wadim Koslow
#
# Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################
"""
@file getSimulationData.py

@brief Executes all data downloads which belong to the epidata package and downloads external data

The functions which are called are:
- getRKIData.get_rki_data
- getPopulationData.get_population_data
- getVacccinationData.get_vaccination_data
- getDIVIData.get_divi_data
"""


from memilio.epidata import getDataIntoPandasDataFrame as gd
from memilio.epidata import defaultDict as dd
from memilio.epidata import getVaccinationData
from memilio.epidata import getPopulationData
from memilio.epidata import getRKIData
from memilio.epidata import getDIVIData


def print_error(text):
    print('Error: Something went wrong while getting ' + text +
          ' data. This was likely caused by a changed file format'
          ' of the source material. Please report this as an issue. ' + text +
          ' data could not be stored correctly.')


def get_simulation_data(split_berlin=dd.defaultDict['split_berlin'],
                        read_data=dd.defaultDict['read_data'],
                        file_format=dd.defaultDict['file_format'],
                        out_folder=dd.defaultDict['out_folder'],
                        no_raw=dd.defaultDict['no_raw'],
                        start_date=dd.defaultDict['start_date'],
                        end_date=dd.defaultDict['end_date'],
                        impute_dates=dd.defaultDict['impute_dates'],
                        make_plot=dd.defaultDict['make_plot'],
                        moving_average=dd.defaultDict['moving_average']
                        ):
    """! Downloads all data from external sources

    The functions which are called are:
    - getRKIData.get_rki_data
    - getPopulationData.get_population_data
    - getVaccinationData.get_vaccination_data
    - getDIVIData.get_divi_data

    Keyword arguments:
    @param split_berlin True or False. Defines if Berlin's disctricts are kept separated or get merged. Default defined in defaultDict.
    @param read_data True or False. Defines if data is read from file or downloaded.  Default defined in defaultDict.
    @param file_format File format which is used for writing the data. Default defined in defaultDict.
    @param out_folder Folder where data is written to. Default defined in defaultDict.
    @param no_raw True or False. Defines if unchanged raw data is saved or not. Default defined in defaultDict.
    @param start_date Date of first date in dataframe. Default 2020-01-01.
    @param end_date Date of last date in dataframe. Default defined in defaultDict.
    @param impute_dates True or False. Defines if values for dates without new information are imputed. Default defined in defaultDict.
    @param moving_average Integers >=0. Applies an 'moving_average'-days moving average on all time series
        to smooth out weekend effects.  Default defined in defaultDict.
    @param make_plot True or False. Defines if plots are generated with matplotlib. Default defined in defaultDict.
    """

    arg_dict_all = {
        "read_data": read_data, "file_format": file_format,
        "out_folder": out_folder, "no_raw": no_raw}

    arg_dict_rki = {**arg_dict_all, "make_plot": make_plot,
                    "impute_dates": impute_dates,
                    "moving_average": moving_average,
                    "split_berlin": split_berlin}

    arg_dict_divi = {**arg_dict_all, "end_date": end_date,
                     "start_date": start_date, "moving_average": moving_average}

    arg_dict_vacc = {**arg_dict_all, "make_plot": make_plot,
                     "moving_average": moving_average}

    try:
        getRKIData.get_rki_data(**arg_dict_rki)
    except Exception as exp:
        print(str(type(exp).__name__) + ": " + str(exp))
        print_error('RKI')

    try:
        getPopulationData.get_population_data(**arg_dict_all)
    except Exception as exp:
        print(str(type(exp).__name__) + ": " + str(exp))
        print_error('population')

    try:
        getDIVIData.get_divi_data(**arg_dict_divi)
    except Exception as exp:
        print(str(type(exp).__name__) + ": " + str(exp))
        print_error('DIVI')

    try:
        getVaccinationData.get_vaccination_data(**arg_dict_vacc)
    except Exception as exp:
        print(str(type(exp).__name__) + ": " + str(exp))
        print_error('vaccination')


def main():
    """! Main program entry."""

    arg_dict = gd.cli("sim")
    get_simulation_data(**arg_dict)


if __name__ == "__main__":
    main()
