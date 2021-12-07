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
from epidemiology.epidata import getCommuterMobility as gcm
from epidemiology.epidata import defaultDict as dd
from datetime import datetime, timedelta, date
from epidemiology.epidata import getPopulationData as gpd

# The following lines are commented to remember a solution to write an output without using the function print()
# This is important, because the usage of print would alter the test results
# import sys
# sys.stdout.write(str()) 
# Data comes from  "https://www.divi.de/divi-intensivregister-tagesreport-archiv-csv"

class Test_Divi_SanityChecks(unittest.TestCase):

    #def setUp(self):
        # get current Header 
    (df, df_counties, df_states, df_ger) = gdd.get_divi_data(read_data=False)   
    today = date.today()

    def test_header_names(self):
        # These strings need to be in the header 
        test_strings = {
            "date", "bundesland", "gemeindeschluessel", "faelle_covid_aktuell",
            "faelle_covid_aktuell_invasiv_beatmet"}

        # get actual headers
        actual_strings_list = self.df.columns.tolist()

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

    
        # get actual headers
        actual_strings_list = self.df.columns.tolist()

        # compare
        self.assertEqual(
            len(test_strings),
            len(actual_strings_list),
            "Number of data categories changed.")

    def test_number_of_rows(self):
        actual_data_length = len(self.df.index)

        # calculate length of df
        last_length = 222145                # date = 2021-4-11
        average_plus = 396.3                # calculated from last 7 known
        days_difference = (self.today - date(2021, 11, 4)).days
        cal_exp_lgth = last_length + average_plus*(days_difference)
        variety = 0.05

        # compare
        self.assertAlmostEqual(
            actual_data_length / cal_exp_lgth, 1.00, None,
            "There's a huge difference in lenght of the given data",variety)

    def test_number_of_rows_rudimentary(self):
        actual_data_length = len(self.df.index)
        self.assertGreaterEqual(
            actual_data_length, 222145,
            "The probably data changed in size, e.g. data for just one day instead of every day in one file.")
    
class Test_Commuter_SanityChecks(unittest.TestCase):
    
    def setUp(self):
        # is there any way to do this faster?
        self.df_commuter_migration = gcm.get_commuter_data(
            setup_dict='', read_data=dd.defaultDict['read_data'],
            file_format=dd.defaultDict['file_format'],
            out_folder=dd.defaultDict['out_folder'],
            make_plot=dd.defaultDict['make_plot'],
            no_raw=dd.defaultDict['no_raw'])

    def test_data_size(self):
        self.assertEqual(len(self.df_commuter_migration.index),
                         len(self.df_commuter_migration.columns))
        assert len(self.df_commuter_migration.index) > 40
        assert len(self.df_commuter_migration.index) < 500

    # check if diagonal elements =0 ?
    '''
    class Test_PopulationData_SanityChecks(unittest.TestCase):
    
    def setUp(self):
        self.df = gpd.get_population_data()

    def test_header_names(self):
        test_strings = {
            "FID", "LAN_ew_RS", "LAN_ew_AGS", "LAN_ew_SDV_RS", "LAN_ew_GEN",
            "LAN_ew_BEZ", "LAN_ew_IBZ", "LAN_ew_FK_S3", "LAN_ew_NUTS",
            "LAN_ew_WSK", "LAN_ew_EWZ", "LAN_ew_KFL", "SHAPE_Length",
            "SHAPE_Area", "LAN_ew_SN-G", "LAN_ew_SN-R", "LAN_ew_SN-L",
            "LAN_ew_SN-K", "LAN_ew_SN-V1", "LAN_ew_SN-V2", "LAN_ew_BEM"}
        actual_strings_list = self.df.columns.tolist()

        # compare
        for name in test_strings:
            if(name not in actual_strings_list):
                self.assertFalse("Not the same headers anymore!")
    '''
if __name__ == '__main__':
    unittest.main()
