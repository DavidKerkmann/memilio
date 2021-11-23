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
##############################################################################
import unittest
from pyfakefs import fake_filesystem_unittest
import os
import sys
from unittest.mock import patch
from epidemiology.epidata import transformWeatherData as twd

class Test_transformWeatherData(fake_filesystem_unittest.TestCase):

    def setUp(self):
        self.setUpPyfakefs()
        self.setup_dict = "read_data, file_format, out_folder, start_date, end_date, make_plot, moving_average, merge_berlin, merge_eisenach"
    @patch('builtins.print')
    def test_some_errors(self, mock_print):

        #test case where merge_berlin = True
        merge_berlin = True
        with self.assertRaises(SystemExit) as cm:
            twd.transformWeatherData(self.setup_dict)
            exit_string = "ERROR: County-IDs do not match with file"
        self.assertEqual(cm.exception.code, exit_string)
        
        
    #def test_transformWeatherData(self):
    #    (df_weather, directory, filename, file_format) = twd.transformWeatherData(read_data, file_format,
    #                                                                              out_folder, start_date, end_date, make_plot, moving_average, merge_berlin, merge_eisenach)
        

if __name__ == '__main__':
    unittest.main()