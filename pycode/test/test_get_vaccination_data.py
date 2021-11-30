######################################################################
# Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
#
# Authors:
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
######################################################################
from sys import path

import unittest
from unittest.mock import patch, call

import os
import pandas as pd
import numpy as np

from epidemiology.epidata import getVaccinationData
from epidemiology.epidata import getPopulationData
from epidemiology.epidata import defaultDict as dd


class TestGetVaccinationData(unittest.TestCase):

    col_names_vacc_data = [
        'Impfdatum', 'LandkreisId_Impfort', 'Altersgruppe', 'Impfschutz',
        'Anzahl']
    df_vacc_data = pd.DataFrame(columns=col_names_vacc_data)
    CountyMerging = {
    # Different districts to Berlin; reporting differs according to source
    11000: [11001, 11002, 11003, 11004, 11005, 11006, 11007, 11008, 11009, 
            11010, 11011, 11012],
    # Wartburgkreis and Eisenach to Wartburgkreis (decision from July 1, 2021)
    16063 : [16063, 16056]
}
    counties = sorted(set(dd.County.keys()))
    for i in CountyMerging[11000]:
        counties.remove(i)

    for county in counties:
        vacc_data = [
            ('2020-12-27', str(county), '12-17', 1, 10),
            ('2020-12-27', str(county), '12-17', 2, 15),
            ('2020-12-27', str(county), '12-17', 3, 72),
            ('2020-12-27', str(county), '18-59', 1, 2),
            ('2020-12-27', str(county), '18-59', 2, 3),
            ('2020-12-27', str(county), '18-59', 3, 222),
            ('2020-12-27', str(county), '60+', 1, 22),
            ('2020-12-27', str(county), '60+', 2, 332),
            ('2020-12-27', str(county), '60+', 3, 76)
        ]
        df_to_append = pd.DataFrame(
            vacc_data, columns=col_names_vacc_data)
        df_vacc_data = df_vacc_data.append(
            df_to_append, ignore_index=True)

    df_vacc_data.astype({'LandkreisId_Impfort': 'string'}).dtypes
    df_vacc_data.astype({'Altersgruppe': 'string'}).dtypes
    df_vacc_data.astype({'Impfschutz': int}).dtypes
    df_vacc_data.astype({'Anzahl': int}).dtypes

    df_empty = pd.DataFrame()
    @patch('epidemiology.epidata.getVaccinationData.download_vaccination_data',
           return_value=df_vacc_data)
    def test_get_vaccination_data(self, mock_vaccination):

        [read_data, file_format, no_raw] = [False, 'json', False]
        out_folder = dd.default_file_path
        getVaccinationData.get_vaccination_data(
            read_data, file_format, out_folder, no_raw)

    def test_download_vaccination_data(self):
        df = getVaccinationData.download_vaccination_data()
        # Normally this should'nt be empty.
        self.assertFalse(
            df.empty,
            "Vaccination Data is empty. Should'nt be.")

    @patch('epidemiology.epidata.getVaccinationData.download_vaccination_data',
           return_value=df_empty)
    def test_download_not_working(self, mock_vaccination):
        df = getVaccinationData.download_vaccination_data()
        self.assertTrue(df.empty, "Vaccination Data is empty.")

    def test_intervall_mapping(self):
        print("HI")


if __name__ == '__main__':
    unittest.main()
