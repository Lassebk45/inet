How to run a simulation on your local pc (requires ubuntu. ~30 minutes to setup):
create "projects" directory in your home directory by running e.g. "mkdir ~/projects"
Clone this repository: git@github.com:Lassebk45/inet.git into ~/projects
Clone our update component: git@github.com:knaD123/p10.git into ~/projects

Add the following two lines at the bottom of the file ~/.profile:
[ -f "$HOME/projects/inet/omnetpp/setenv" ] && source "$HOME/projects/inet/omnetpp/setenv"

[ -f "$HOME/projects/inet/setenv" ] && source "$HOME/projects/inet/setenv"

Log out and in to your pc to make the changes take effect.

Change to the omnet directory: "cd ~/projects/omnetpp"

Try to run "./configure WITH_QTENV=no WITH_OSG=no". This might require some python packages to be installed. In that case run:

"sudo apt-get install build-essential clang lld gdb bison flex perl python3 python3-pip qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5opengl5-dev libxml2-dev zlib1g-dev doxygen graphviz libwebkit2gtk-4.0-37"

and

"python3 -m pip install --user --upgrade numpy pandas matplotlib scipy seaborn posix_ipc"

Run "./configure WITH_QTENV=no WITH_OSG=no" again and it should work. Otherwise consult: https://doc.omnetpp.org/omnetpp/InstallGuide.pdf

Run "make -j $(nproc)" to build omnet using all the available cores in your system (5-10 min). 

cd to INET: "cd ~/projects/inet/"

Clean the makefiles: "make cleanall"

Generate new makefiles: "make makefiles"

Build INET: "make -j $(nproc)" (10-20 minutes)

Change dir to our update component: "cd ~/projects/p10"

Setup a virtual enviroment with the necessary python packages:

Make sure you can create a Python venv: "sudo apt install python3.10-venv
"
Then run:

"python3 -m venv venv"

"source venv/bin/activate"

"python3 -m pip install -r requirements.txt"

Now the necessary packets are installed.

We run an example simulation using the Bics topology, which is visualized in p10/utilities/figures/zoo_Bics.json.png.

We first run Spungeet (2 minutes):

"python3 main.py --topology pruned_topologies/zoo_Bics.json --demands pruned_temporal_demands/Bics_0000.yml --algorithm SPUNGEET --generate_package --update_interval 10 --write_interval 5 --scaler 25 --time_scale 0.01 --short_experiment --demand_scaler 1"

We then run the same experiment but with All Shortest Paths (1 minute):

"python3 main.py --topology pruned_topologies/zoo_Bics.json --demands pruned_temporal_demands/Bics_0000.yml --algorithm split_shortest_path --generate_package --update_interval 10 --write_interval 5 --scaler 25 --time_scale 0.01 --short_experiment --demand_scaler 1"

This outputs two json results files in the directory p10/results/Bics. By comparing these files we observe that:

Spungeet achieves 85.4% delivered packet rate by dropping 43969 out of 302628 packets due to congestion.

ASP achieves 82.1% delivered packet rate by dropping 54145 out of 302582 packets due to congestion.

Both algorithms reached a max link utilization of 100%.



[![badge 1][badge-1]][1] [![badge 2][badge-2]][2]

INET Framework for OMNEST/OMNeT++
=================================

The [INET framework](https://inet.omnetpp.org) is an open-source communication networks
simulation package, written for the OMNEST/OMNeT++ simulation system. The INET
framework contains models for numerous wired and wireless protocols, a detailed
physical layer model, application models and more. See the CREDITS file for the
names of people who have contributed to the INET Framework.

IMPORTANT: The INET Framework is continuously being improved: new parts
are added, bugs are corrected, and so on. We cannot assert that any protocol
implemented here will work fully according to the specifications. YOU ARE
RESPONSIBLE YOURSELF FOR MAKING SURE THAT THE MODELS YOU USE IN YOUR SIMULATIONS
WORK CORRECTLY, AND YOU'RE GETTING VALID RESULTS.

Contributions are highly welcome. You can make a difference!

See the WHATSNEW file for recent changes.


GETTING STARTED
---------------
You may start by downloading and installing the INET framework. Read the INSTALL
file for further information.

Then you can gather initial experience by starting some examples or following a
tutorial or showcase (see the /examples, /showcases or /tutorials folder).
After that, you can learn the NED language from the OMNeT++ manual & sample
simulations.

After that, you may write your own topologies using the NED language. You may
assign some of the submodule parameters in NED files. You may leave some of
them unassigned.

Then, you may assign unassigned module parameters in omnetpp.ini of your
simulation. (You can refer to sample simulations & manual for the content of
omnetpp.ini)

Finally, you will be ready to run your simulation. As you see, you may use
the INET framework without writing any C++ code, as long as you use the
available modules.

To implement new protocols or modify existing ones, you'll need to add your
code somewhere under the src directory. If you add new files under the 'src'
directory you will need to regenerate the makefiles (using the 'make makefiles'
command).

If you want to use external interfaces in INET, enable the "Emulation" feature
either in the IDE or using the inet_featuretool then regenerate the INET makefile
using 'make makefiles'.


[badge-1]: https://github.com/inet-framework/inet/workflows/Build%20and%20tests/badge.svg?branch=master
[badge-2]: https://github.com/inet-framework/inet/workflows/Feature%20tests/badge.svg?branch=master

[1]: https://github.com/inet-framework/inet/actions?query=workflow%3A%22Build+and+tests%22
[2]: https://github.com/inet-framework/inet/actions?query=workflow%3A%22Feature+tests%22
