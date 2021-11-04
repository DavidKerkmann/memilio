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
from epidemiology.epidata import getDIVIData as gD

# The following lines are commented to remember a solution to write an output without using the function print()
# This is important, because the usage of print would alter the test results
# import sys
# sys.stdout.write(str()) 

class Test_SanityChecks(unittest.TestCase):
    def test_header_names(self):
        today = date.today()
        last_number = 6072
        [_, df, _] = gD.download_data_for_one_day(last_number, today)
        row_1=df.iloc[0]
        

        actual_name = "date"              
        given_name = "date"          
        self.assertEqual(actual_name, given_name)

if __name__ == '__main__':
    unittest.main()