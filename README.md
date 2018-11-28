# Crystal
Crystal is an interference-resilient ultra-low power data collection protocol for wireless sensor networks (WSN) especially efficient in applications generating sparse aperiodic traffic. It was shown to achieve over 99.999% packet delivery ratio in presence of Wi-Fi interference with per-mille radio duty cycle in some real-world applications. Crystal uses Glossy as underlying communication and time-synchronization primitive.

Crystal got the [2nd prize](https://iti-testbed.tugraz.at/blog/page/11/ewsn-18-dependability-competition-final-results/) at the EWSN'18 Dependability Competition!

## Publications

 * **Data Prediction + Synchronous Transmissions = Ultra-low Power Wireless Sensor Networks**, Timofei Istomin, Amy L. Murphy, Gian Pietro Picco, Usman Raza.  In Proceedings of the 14th ACM Conference on Embedded Networked Sensor Systems (SenSys 2016), Stanford (CA, USA), November 2016, [PDF](http://disi.unitn.it/~picco/papers/sensys16.pdf)
 * **Interference-Resilient Ultra-Low Power Aperiodic Data Collection**, Timofei Istomin, Matteo Trobinger, Amy L. Murphy, Gian Pietro Picco.  In Proceedings of the International Conference on Information Processing in Sensor Networks (IPSN 2018), [PDF](http://disi.unitn.it/~picco/papers/ipsn18.pdf).

## Status
Currently the protocol is implemented for the TMote Sky platform only. Porting it to more modern platforms is in our near future plans.

The **master** branch is the current stable branch of Crystal, while the **devel** branch has the most recent modifications. 

Please contact us in case you need the exact code we used in IPSN'18 experiments. The version we used for the EWSN'18 Dependability Competition is instead published here for reference (**depcomp18** branch). It contains several competition-specific tweaks and will not be updated in the future.

***Disclaimer:*** *Although we tested the code extensively, Crystal is a research prototype that likely contains bugs. We take no responsibility for and give no warranties in respect of using the code.*

## Testing in testbeds
To run experiments with different sets of parameters you can build a binary (or a set of binaries) using the `test_tools/simgen_ta.py` script. It reads the parameter set(s) from `params.py` file.

A good starting point for defining your parameter set can be found in the `exps/example/` directory. You may copy the whole directory and edit the parameters and the list of nodes present in the testbed. After that, run `../../test_tools/simgen_ta.py`. It will create one or several subdirectories (if they don't exist) named after the individual parameter sets defined in the `params.py` file. 

You can specify "cooja" in the testbed name to compile for Cooja, list the node IDs, and use MRM radio model for simulations (e.g., one defined in `apps/crystal-test-simple/mrm.csc`).
