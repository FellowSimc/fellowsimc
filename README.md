# [SimulationCraft](https://www.simulationcraft.org/) [![CI](https://github.com/simulationcraft/simc/workflows/CI/badge.svg)](https://github.com/simulationcraft/simc/actions?query=workflow%3ACI)
## What is This
Fork of the https://github.com/simulationcraft/simc project for the Game Fellowship.
Significant adaptions in progress to adapt the engine to better support Fellowship.

This is a personal project so there will be no support with using the software. There are numerous bugs. Updates will be sporadic as I find and fix things.

Heavy work in progress.

WoW code has yet to be pruned.


## Overview

SimulationCraft is a tool to explore combat mechanics in the popular MMO RPG ~~World of Warcraft~~ Fellowship (tm).

It is a multi-player event driven simulator written in C++ that models player character damage-per-second in various raiding and dungeon scenarios.

Increasing class synergy and the prevalence of proc-based combat modifiers have eroded the accuracy of traditional calculators that rely upon closed-form approximations to model very complex mechanics. The goal of this simulator is to close the accuracy gap while maintaining a performance level high enough to calculate relative stat weights to aid gear selection.

SimulationCraft allows raid/party creation of arbitrary size, generating detailed charts and reports for both individual and raid performance.

Graphical interface is slated for removal to remove dependencies due to it being based on the World of Warcraft code. It will not be supported.

## How Can I Contribute?
See [Contributing](CONTRIBUTING.md).

## Original Source
SimulationCraft (https://github.com/simulationcraft/simc). GNU GPL v3 License (see License for more information)

## External Libraries

This program uses the following external libraries.

RapidJSON (http://rapidjson.org)

Copyright (c) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
MIT License (see LICENSE.MIT for more information).

RapidXML (http://rapidxml.sourceforge.net/index.htm)

Copyright (c) 2006, 2007 Marcin Kalicinski. All rights reserved.
MIT License (see LICENSE.MIT for more information).

The MSInttypes r29 (https://code.google.com/p/msinttypes/)

Copyright (c) Alexander Chemeris. All rights reserved.
BSD 3-Clause License (see LICENSE.BSD for more information).

The Qt Toolkit (https://www.qt.io/)

Copyright (c) 2016 The Qt Company Ltd. and other contributors. All rights reserved.
GNU Lesser General Public License, version 3 (see LICENSE.LGPL for more information).

UTF-8 CPP (https://github.com/nemtrif/utfcpp)

Copyright (c) 2006 Nemanja Trifunovic. All rights reserved.
Boost Software License, Version 1.0 (see LICENSE.BOOST for more information).

{fmt} (https://github.com/fmtlib/fmt)

Copyright (c) 2012 - 2016, Victor Zverovich. All rights reserved.
BSD 2-Clause "Simplified" License (see LICENSE.BSD2 for more information).

cpp-semver (https://github.com/easz/cpp-semver)

Copyright (c) 2018 Cas Perl. All rights reserved.
MIT License (see LICENSE.MIT for more information).

utf8.h (https://github.com/sheredom/utf8.h)

Unlicense License (see LICENSE.UNLICENSE for more information).
