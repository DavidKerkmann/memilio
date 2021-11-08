#############################################################################
# Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
#
# Authors: Patrick Lenz, Sascha Korf 
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
from epidemiology.epidata import getDIVIData as gdd
from datetime import timedelta, date

# The following lines are commented to remember a solution to write an output without using the function print()
# This is important, because the usage of print would alter the test results
# import sys
# sys.stdout.write(str()) 

class Test_SanityChecks(unittest.TestCase):
    def test_header_names(self):

        # These strings need to be in the header 
        test_strings = {
            "date", "bundesland", "gemeindeschluessel", "faelle_covid_aktuell",
            "faelle_covid_aktuell_invasiv_beatmet"}

        # get current Header 
        today = date.today() - timedelta(1)
        last_number = 6072
        [_, df, _] = gdd.download_data_for_one_day(last_number, today)
        
        # get actual headers
        actual_strings_list = df.columns.tolist()

        #Compare
        for name in test_strings:
            if(name not in actual_strings_list):
                self.assertFalse("Not the same headers anymore!")
    
    def test_number_of_data(self):
        
        # These strings are the given data categories 
        test_strings = {
            "date", "bundesland", "gemeindeschluessel", "anzahl_standorte",
            "anzahl_meldebereiche", "faelle_covid_aktuell",
            "faelle_covid_aktuell_invasiv_beatmet", "betten_frei",
            "betten_belegt", "betten_belegt_nur_erwachsen",
            "betten_frei_nur_erwachsen"}

        # get current Header 
        today = date.today() - timedelta(1)
        last_number = 6072
        [_, df, _] = gdd.download_data_for_one_day(last_number, today)
        
        # get actual headers
        actual_strings_list = df.columns.tolist()

        # compare
        self.assertEqual(
            len(test_strings),
            len(actual_strings_list),
            "Number of data categories changed.")

    def test_number_of_rows(self):

        # get actual length of dataframe
        today = date.today() - timedelta(1)
        last_number = 6072
        [_, df, _] = gdd.download_data_for_one_day(last_number, today)
        actual_data_length = len(df.index)

        # calculate length of df
        last_length = 222145                # date = 2021-4-11
        average_plus = 396.3                # calculated from last 7 known
        days_difference = (today - date(2021, 11, 4)).days
        cal_exp_lgth = last_length + average_plus*(days_difference)
        variety = 0.05

        # compare
        self.assertAlmostEqual(
            variety + actual_data_length / cal_exp_lgth, 1.00, 1,
            "There's a huge difference in lenght of the given data")


if __name__ == '__main__':
    unittest.main()