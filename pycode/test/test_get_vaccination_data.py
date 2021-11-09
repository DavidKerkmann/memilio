#############################################################################
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
#############################################################################
import unittest
from pyfakefs import fake_filesystem_unittest
from unittest.mock import patch, call

import os
import pandas as pd
import numpy as np

from epidemiology.epidata import getVaccinationData
from epidemiology.epidata import getPopulationData


class TestGetVaccinationData(fake_filesystem_unittest.TestCase):
    # construct fake directory for testing
    maxDiff = None

    path = '/home/VaccinationData'

    col_names = ['Unnamed: 0', 'Gesamtzahl bisher verabreichter Impfungen',
                 'Gesamtzahl einmalig geimpft', 'Gesamtzahl vollständig geimpft',
                 'Impfquote mit einer Impfung', 'Unnamed: 6', 'Unnamed: 7',
                 'Impfquote vollständig geimpft', 'Unnamed: 9', 'eine Impfung',
                 'Unnamed: 12', 'vollständig geimpft', 'Unnamed: 14']
    data = np.zeros((16,len(col_names)))
    data[:,0] = np.arange(1,17)
    data[:,1] = np.arange(16)*10
    data[:,2] = np.arange(16) *5
    data[:,3] = np.arange(16)*2
    data[:,4] = np.arange(16)/16

    data[:,5] = np.arange(16)/32
    data[:,6] = np.arange(16)/8

    data[5,5:7]= 'nan'

    test_vaccination_df =pd.DataFrame(data, columns= col_names)

    test_vaccination_df[col_names[5:7]][5] = '-'

    col_names = ['ID_County', 'Total', '<3 years', '3-5 years', '6-14 years', '15-17 years', '18-24 years',
               '25-29 years', '30-39 years', '40-49 years', '50-64 years',
               '65-74 years', '>74 years']

    data = np.zeros((17,len(col_names)))
    data[:,0] = np.arange(1,18)*1000
    data[-1,0] = 16001
    data[:, 1] = 110
    data[:,2:] = 10

    test_pop_df = pd.DataFrame(data, columns=col_names)

    col_names = ['ID_County', 'Administrated_Vaccinations', 'First_Shot', 'Full_Vaccination',
               'Ratio_All', 'Ratio_Young', 'Ratio_Old']
    data = np.zeros((17, len(col_names)))
    data[:,0] = test_pop_df['ID_County'].values
    data[:-1,1:4] = test_vaccination_df[test_vaccination_df.columns[1:4]]
    data[15:, 1:4] = data[15,1:4]/2
    data[:-1, 4:] = test_vaccination_df[test_vaccination_df.columns[4:7]]
    data[-1,4:] = data[15,4:]
    data[5, 5] = (np.sum(np.arange(16)) - 5)/(32*15)
    data[5, 6] = (110*data[5, 4] - (80 + (2/3)*10)*data[5,5])/(20 + (1/3)*10)
    test_result_df = pd.DataFrame(data, columns=col_names)
    test_result_df[test_result_df.columns[[0,1,3]]] = test_result_df[test_result_df.columns[[0,1,3]]].astype('int64')

    def setUp(self):
        self.setUpPyfakefs()

    @patch('epidemiology.epidata.getVaccinationData.download_vaccination_data', return_value=(test_vaccination_df, 'test_vaccination'))
    @patch('epidemiology.epidata.getVaccinationData.getPopulationData.get_age_population_data', return_value=test_pop_df)
    def test_get_vaccination_data(self, mock_vaccination, mock_pop):

        [read_data, file_format, out_folder, no_raw] = [False, 'json', self.path, False]
        getVaccinationData.get_vaccination_data(read_data, file_format, out_folder, no_raw)

        directory = os.path.join(out_folder, 'Germany/')

        filename = 'test_vaccination.json'

        test_df = pd.read_json(os.path.join(directory, filename))

        pd.testing.assert_frame_equal(test_df, self.test_result_df)


if __name__ == '__main__':
    unittest.main()
