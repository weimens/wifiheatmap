
# wifiheatmap
![screenshot](https://weimens.github.io/wifiheatmap/screenshot.png)

Overview
--------
Wifiheatmap is an WiFi survey tool to visualize the WiFi signal strength in a building or area.
Basic usage: Walk around and take measurements.
Take a look at the resulting heat map to decide whether the coverage is good enough.
If the coverage is not satisfying, use the heat map information to determine if you need more access points
or moving one or several APs to another location could be sufficient.

Installation
----------
A precompiled rpm package is available for Fedora
[Releases page](https://github.com/weimens/wifiheatmap/releases).

Basic Usage
-----------
1. Open a floor plan (png or jpg).
2. Decide which measurement you want:
	* connected or scan: measure only the currently connected wifi or scan for all available wifis
	* iperf: measure the bandwidth with iperf3
		* As usual for iperf you need to start `iperf3 -s` on another pc to act as a server.
		  This pc should have an ethernet connection which is faster than the wifi that you want to measure,
          otherwise this becomes an bottleneck.
3. Walk to the physical location where you want to measure and click on the corresponding location on the floor plan to place a measurement.
  Wait until the measurement is finished.
  - Repeat this step for the area where you want to collect information on the available WiFi APs.
    Four measurements per room placed in the corners are a good starting point.
    If something goes wrong, you can remove a point by double click or move it with drag and drop.
4. To display the resulting heat map, select an AP from the list on the left side.
  When multiple APs are selected,
  the best signal is used to generate the heat map.

Compiling
---------
Please note that wifiheatmap is currently only tested on Fedora. If you want to compile wifiheatmap for android take a look at [wifiheatmap-superbuild](https://github.com/weimens/wifiheatmap-superbuild)

Install dependencies,
```
sudo dnf install gcc-c++ cmake make qt5-qtbase-devel qt5-qtdeclarative-devel qt5-qtquickcontrols2 \
  CGAL-devel libnl3-devel quazip-qt5-devel iperf3-devel
```
get the source code,
```
git clone https://github.com/weimens/wifiheatmap.git wifiheatmap-source
```
build it.
```
mkdir wifiheatmap-build && cd wifiheatmap-build
cmake ../wifiheatmap-source
make -j$(nproc)
```

