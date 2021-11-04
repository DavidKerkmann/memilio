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
from freezegun import freeze_time
from datetime import date, timedelta

import os
import pandas as pd

from epidemiology.epidata import getDIVIData as gdd
from epidemiology.epidata import getDataIntoPandasDataFrame as gd
from epidemiology.epidata import defaultDict as dd
from unittest.mock import patch, call


# The following lines are commented to remember a solution to write an output without using the function print()
# This is important, because the usage of print would alter the test results
# import sys
# sys.stdout.write(str())