# slow-mode
App that allows for slow mode on the vehicle
It uses information about zoom level (from Payload Manager), knob value and weather 3m/s velocity limit is on (both from AMC)
#
To build locally:
```
mkdir build && cd build
```
Followed by:
```
cmake -S ../services/slowmode-app && make
```
To run in UDP mode:
```
./slowmode-app-udp
```
To run in TCP mode:
```
./slowmode-app-tcp
```
#
To install the app on the vehicle follow the auterion-cli guideline.
From root directory run:
```
 auterion-cli app build
 ```
 Followed by:
 ```
 auterion-cli app install build/com.auterion.slowmode-app.auterionos
 ```