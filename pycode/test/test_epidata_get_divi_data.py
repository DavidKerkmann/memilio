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
from pandas.core.indexes.base import Index
from pyfakefs import fake_filesystem_unittest
from freezegun import freeze_time
from datetime import date, datetime, time, timedelta

import os
import io
import pandas as pd

from epidemiology.epidata import getDIVIData as gdd
from epidemiology.epidata import getDataIntoPandasDataFrame as gd
from epidemiology.epidata import defaultDict as dd
from unittest.mock import patch, call


class TestGetDiviData(fake_filesystem_unittest.TestCase):

    maxDiff = None

    path = '/home/DiviData'

    test_df = pd.DataFrame(
        {
            'Date':
            ['2021-09-08', '2021-09-08', '2021-09-08', '2021-09-08', '2021-09-08',
             '2021-09-08', '2021-09-08', '2021-09-08', '2021-09-08', '2021-09-08',
             '2021-09-08', '2021-09-08', '2021-09-08', '2021-09-08', '2021-09-08',
             '2021-09-08'],
            'ICU':
            [16, 52, 111, 7, 432, 126, 74, 175, 208, 33, 79, 16, 11, 27, 5, 15],
            'ICU_ventilated':
            [13, 34, 63, 5, 220, 53, 38, 79, 111, 15, 53, 8, 7, 13, 2, 9],
            'ID_State': [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
            'State':
            ['Schleswig-Holstein', 'Hamburg', 'Niedersachsen', 'Bremen',
                'Nordrhein-Westfalen', 'Hessen', 'Rheinland-Pfalz',
                'Baden-Württemberg', 'Bayern', 'Saarland', 'Berlin', 'Brandenburg',
                'Mecklenburg-Vorpommern', 'Sachsen', 'Sachsen-Anhalt', 'Thüringen']})
    (df_raw, df_counties, df_states, df_ger) = gdd.get_divi_data()

    def setUp(self):
        self.setUpPyfakefs()

    def test_cut_of_dates(self):
        df_state_testdate = gdd.cut_of_dates(
            self.df_states, date(2021, 9, 8),
            date(2021, 9, 8))
        pd.testing.assert_frame_equal(self.test_df, df_state_testdate)

    
    @unittest.mock.patch('sys.stdout', new_callable=io.StringIO)
    def test_det_divi_data(self, mock_print):

        # case with start_date before 2020-04-24 
        [read_data, file_format, out_folder, no_raw, end_date, start_date,
         impute_dates, moving_average] = [False, "json", self.path, False,
                                          date.today(),
                                          date(2020, 1, 1),
                                          dd.defaultDict['impute_dates'],
                                          dd.defaultDict['moving_average']]

        gdd.get_divi_data(
            read_data, file_format, out_folder, no_raw, end_date, start_date,
            impute_dates, moving_average)

        expected_output = 'Warning: First data available on 2020-04-24. You asked for 2020-01-01.'
        expected_in_actual_print = mock_print.getvalue()
        self.assertTrue(expected_output.__contains__(expected_output))

        


if __name__ == '__main__':
    unittest.main()
