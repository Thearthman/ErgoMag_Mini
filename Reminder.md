# Terminal Commands
To build uf2 firmwares, first make sure libraries are updated and compiled by running `cmake -G "MinGW Makefiles" ..` under `.\ErgoMag\ErgoMag_Mini\Code\ErgoMag\build`. Then, run `make` to compile main.c, also in the above directory. Last, run `cp .\ErgoMag.uf2 G:/` for fast deployment.
