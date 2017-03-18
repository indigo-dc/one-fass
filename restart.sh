#!/bin/sh

# Copyright © 2017 INFN Torino - INDIGO-DataCloud
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
#

rm -rf /var/lock/fass/fass
echo "Stopping fass..."
sudo systemctl stop fass
echo "Building..."
scons
echo "Installing..."
sudo ./install.sh
echo "Starting fass..."
sudo systemctl start fass
echo "And the result is..."
sudo systemctl status fass
