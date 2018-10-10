

# Distributed Agents for Mobility Generation

## Description

This is a basic toolkit for mobility generation using distributed agents.
Useful for vehicle simulations once equiped with a visualiser on OSM.


## How does it work?

A manager starts accepting incoming connections from the agents.
Each agent connects to the manager and sends JSON payloads containing: current position, nearby agents, or a command to be executed by the manager.


		agent
		  ^
		  |
		  |
		 json - coords, bids, behaviors, etc.
		  | 
		  |
		  v
		agent(s) <------json-----> manager <-----json?-----> visualiser
						 |
					 commands, coords


## Usage

Start by running the manager:

```
./manager conf.json
```


Then, trun the client(s):

```
./agent name
```





## Licence & Copyright
This software was developed in the hope that it would be of some use to the AI research community, and is freely available for redistribution and/or modification under the terms of the GNU General Public Licence. It is distributed WITHOUT WARRANTY; without even the implied warranty of merchantability or fitness for a particular purpose. See the [GNU General Public License] for more details. 

If you find this code to be of any use, please let me know. I would also welcome any feedback.

Copyright (c) 2015--2018 Rafik Hadfi, rafik.hadfi@gmail.com
