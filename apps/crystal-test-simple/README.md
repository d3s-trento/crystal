This simple example application sends one or more packets per Crystal epoch from every node to the sink.

To compile for TMote Sky, use `./build_tmote.sh NODE_ID`. Node IDs are hard-coded in the binary and are 
16-bit unsigned integers not equal to zero or 0xFFFF. 
By default, the node with ID 1 is the sink. To change it, edit the `build_tmote.sh` script.
The script will produce a `crystal-test-*.sky` binary file with the specified node ID that can be flashed to a TMote Sky.

To compile for Cooja, use `./build_cooja.sh`. To launch, type `./cooja mrm.csc`, provided you have installed the Cooja submodules and 
compiled the simulator (see `../../README.md`).
