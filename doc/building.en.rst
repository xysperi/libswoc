.. Licensed to the Apache Software Foundation (ASF) under one or more contributor license
   agreements. See the NOTICE file distributed with this work for
   additional information regarding copyright ownership. The ASF licenses this file to you under the
   Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
   the License. You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software distributed under the License
   is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
   or implied. See the License for the specific language governing permissions and limitations under
   the License.

.. include:: common-defs.rst

.. _building:

Building
********

The library is usually built using CMake, but is also intended to be buildable as a part for
`SCons <http://scons.org>`__ using the `Parts <https://pypi.org/project/scons-parts/>` extensions.
For this reason the overall codebase is split into the library code and the test code. As a library
only the library code needs to be built, which is the part "swoc++/swoc++.part". For example, to
get the 1.0.8 release as a library in a larger project, the top level "Sconstruct" would have ::

   Part("swoc++/swoc++.part"
     , vcs_type=VcsGit(server="github.com"
       , repository="SolidWallOfCode/libswoc"
       , tag="1.0.8"))

The object that depends on this library would then have ::

   DependsOn([
      # ... other dependencies
      , Component("swoc++")
      , # ... more dependencies
   ])
