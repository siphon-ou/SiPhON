# SiPhON

SiPhON (Si-Photonics-Based In-Vehicle Optical Network) simulator is based on CoRE4INET simulation extension over [OMNEST/OMNeT++](https://omnetpp.org/) simulation system. 
SiPhON is created by Advanced Network Architecture Research Laboratory at Osaka University.
CoRE4INET is created by the CoRE (Communication over Realtime Ethernet) research group at the HAW-Hamburg (Hamburg University of Applied Sciences).


## Installation

The simulator is tested only on Windows.

1. Download OMNeT++ 5.5.1
    * [https://omnetpp.org/download/](https://omnetpp.org/download/)
2. Install OMNeT++
    * [https://doc.omnetpp.org/omnetpp/InstallGuide.pdf](https://doc.omnetpp.org/omnetpp/InstallGuide.pdf)
3. Download INET framework 3.6.6
    * [https://github.com/inet-framework/inet/releases/download/v3.6.6/inet-3.6.6-src.tgz](https://github.com/inet-framework/inet/releases/download/v3.6.6/inet-3.6.6-src.tgz)
4. Install INET framework 3.6.6
	Run mingwenv.cmd
	Enter the command omnetpp
	From the File menu click File -> General -> Existing Projects into Workspace. Press "Select archive file". Choose the inet-3.6.6-src.tgz file.
5. Download SiPhON
	Download (clone) as a zip file from [https://github.com/siphon-ou/SiPhON](https://github.com/siphon-ou/SiPhON)
5. Install SiPhON
	From the File menu click File -> General -> Existing Projects into Workspace. Press "Select archive file". Choose the downloaded SiPhON zip file.
	
	
## Simulation

Two types of SiPhON scheduling 	algorithms are implemented in the simulator. 
Dynamic scheduling is implemented under dynamicintracarinfotainmentatmaster folder and the static scheduling is implemented under staticintracarinfotainmentatmaster folder.
Both folders are under examples folder of SiPhON.
Please run the omnetpp.ini file in large_network folder under dynamicintracarinfotainmentatmaster or staticintracarinfotainmentatmaster.
Ignore the warnings.
Choose the "Default" config name when asked.
	
	
	
## References

O. Alparslan, S. Arakawa, and M. Murata, "A Zone-based Optical Intra-Vehicle Backbone Network Architecture with Dynamic Slot Scheduling," Optical Switching and Networking, vol. 50, August 2023.