# Distributed Agents for Mobility Generation

## Description

This is a basic toolkit for mobility generation using distributed agents.

## Usage

Start by running the manager:

```
./manager conf.json
```

A manager starts accepting incoming connections from the agents.
Then, run the client(s):

```
./agent name
```

Each agent connects to the manager and sends JSON payloads containing: current position, nearby agents, or a command to be executed by the manager.

## In other projects

- Traffic simulation on [StarBED](http://starbed.nict.go.jp/en/index.html) 

<p align="center">
	<img src="https://github.com/raviq/Agent_mobility/blob/master/arch.png" width="400">
</p>

## Licence & Copyright
This software was developed in the hope that it would be of some use to the AI research community, and is freely available for redistribution and/or modification under the terms of the GNU General Public Licence. It is distributed WITHOUT WARRANTY; without even the implied warranty of merchantability or fitness for a particular purpose. See the [GNU General Public License] for more details. 

If you find this code to be of any use, please let me know. I would also welcome any feedback.

Copyright (c) 2015--2018 Rafik Hadfi, rafik.hadfi@gmail.com

