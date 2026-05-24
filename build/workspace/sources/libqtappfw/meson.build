#
# Copyright (C) 2022 Konsulko Group
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

project (
  'libqtappfw',
  ['cpp'],
  version : '2.0.1',
  license : 'Apache-2.0',
  meson_version : '>= 0.59.0',
  default_options :
    [
      'warning_level=1',
      'buildtype=debugoptimized',
      'c_std=c17',
      'cpp_std=c++17'
    ],
)

qt = import('qt6')

#subdir('docs')

# These need to be first as they declare dependencies for use in some
# of the other libraries
subdir('bluetooth')
subdir('vehicle-signals')

subdir('applauncher')
subdir('hvac')
subdir('map')
subdir('mediaplayer')
subdir('navigation')
subdir('network')
subdir('pbap')
subdir('radio')
subdir('telephony')
subdir('weather')
