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
        'Impfdatum',
        'LandkreisId_Impfort',
        'Altersgruppe',
        'Impfschutz',
        'Anzahl']
    df_vacc_data = pd.DataFrame(columns=col_names_vacc_data)
    counties = np.array(
        [1001, 1002, 1003, 1004, 1051, 1053, 1054, 1055, 1056, 1057, 1058,
         1059, 1060, 1061, 1062, 2000, 3101, 3102, 3103, 3151, 3153, 3154,
         3155, 3157, 3158, 3159, 3241, 3251, 3252, 3254, 3255, 3256, 3257,
         3351, 3352, 3353, 3354, 3355, 3356, 3357, 3358, 3359, 3360, 3361,
         3401, 3402, 3403, 3404, 3405, 3451, 3452, 3453, 3454, 3455, 3456,
         3457, 3458, 3459, 3460, 3461, 3462, 4011, 4012, 5111, 5112, 5113,
         5114, 5116, 5117, 5119, 5120, 5122, 5124, 5154, 5158, 5162, 5166,
         5170, 5314, 5315, 5316, 5334, 5358, 5362, 5366, 5370, 5374, 5378,
         5382, 5512, 5513, 5515, 5554, 5558, 5562, 5566, 5570, 5711, 5754,
         5758, 5762, 5766, 5770, 5774, 5911, 5913, 5914, 5915, 5916, 5954,
         5958, 5962, 5966, 5970, 5974, 5978, 6411, 6412, 6413, 6414, 6431,
         6432, 6433, 6434, 6435, 6436, 6437, 6438, 6439, 6440, 6531, 6532,
         6533, 6534, 6535, 6611, 6631, 6632, 6633, 6634, 6635, 6636, 7111,
         7131, 7132, 7133, 7134, 7135, 7137, 7138, 7140, 7141, 7143, 7211,
         7231, 7232, 7233, 7235, 7311, 7312, 7313, 7314, 7315, 7316, 7317,
         7318, 7319, 7320, 7331, 7332, 7333, 7334, 7335, 7336, 7337, 7338,
         7339, 7340, 8111, 8115, 8116, 8117, 8118, 8119, 8121, 8125, 8126,
         8127, 8128, 8135, 8136, 8211, 8212, 8215, 8216, 8221, 8222, 8225,
         8226, 8231, 8235, 8236, 8237, 8311, 8315, 8316, 8317, 8325, 8326,
         8327, 8335, 8336, 8337, 8415, 8416, 8417, 8421, 8425, 8426, 8435,
         8436, 8437, 9161, 9162, 9163, 9171, 9172, 9173, 9174, 9175, 9176,
         9177, 9178, 9179, 9180, 9181, 9182, 9183, 9184, 9185, 9186, 9187,
         9188, 9189, 9190, 9261, 9262, 9263, 9271, 9272, 9273, 9274, 9275,
         9276, 9277, 9278, 9279, 9361, 9362, 9363, 9371, 9372, 9373, 9374,
         9375, 9376, 9377, 9461, 9462, 9463, 9464, 9471, 9472, 9473, 9474,
         9475, 9476, 9477, 9478, 9479, 9561, 9562, 9563, 9564, 9565, 9571,
         9572, 9573, 9574, 9575, 9576, 9577, 9661, 9662, 9663, 9671, 9672,
         9673, 9674, 9675, 9676, 9677, 9678, 9679, 9761, 9762, 9763, 9764,
         9771, 9772, 9773, 9774, 9775, 9776, 9777, 9778, 9779, 9780, 10041,
         10042, 10043, 10044, 10045, 10046, 11000, 12051, 12052, 12053, 12054,
         12060, 12061, 12062, 12063, 12064, 12065, 12066, 12067, 12068, 12069,
         12070, 12071, 12072, 12073, 13003, 13004, 13071, 13072, 13073, 13074,
         13075, 13076, 14511, 14521, 14522, 14523, 14524, 14612, 14625, 14626,
         14627, 14628, 14713, 14729, 14730, 15001, 15002, 15003, 15081, 15082,
         15083, 15084, 15085, 15086, 15087, 15088, 15089, 15090, 15091, 16051,
         16052, 16053, 16054, 16055, 16056, 16061, 16062, 16063, 16064, 16065,
         16066, 16067, 16068, 16069, 16070, 16071, 16072, 16073, 16074, 16075,
         16076, 16077])

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
