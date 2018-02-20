# Crystal
Crystal is an interference-resilient ultra-low power data collection protocol for wireless sensor networks (WSN) especially efficient in applications generating sparse aperiodic traffic. It was shown to achieve over 99.999% packet delivery ratio in presence of Wi-Fi interference with per-mille radio duty cycle in some real-world applications. Crystal uses Glossy as underlying communication and time-synchronization primitive.

Crystal got the [2nd prize](https://iti-testbed.tugraz.at/blog/page/11/ewsn-18-dependability-competition-final-results/) at the EWSN'18 Dependability Competition!

## Publications

 * **Data Prediction + Synchronous Transmissions = Ultra-low Power Wireless Sensor Networks**, Timofei Istomin, Amy L. Murphy, Gian Pietro Picco, Usman Raza.  In Proceedings of the 14th ACM Conference on Embedded Networked Sensor Systems (SenSys 2016), Stanford (CA, USA), November 2016, [PDF](http://disi.unitn.it/~picco/papers/sensys16.pdf)
 * **Interference-Resilient Ultra-Low Power Aperiodic Data Collection**, Timofei Istomin, Matteo Trobinger, Amy L. Murphy, Gian Pietro Picco.  In Proceedings of the International Conference on Information Processing in Sensor Networks (IPSN 2018), to appear.

## Status
Currently the protocol is implemented for the TMote Sky platform only. Porting it to more modern platforms is in our near future plans.

The **depcomp18** branch contains exactly the code that was used at the EWSN'18 Dependability Competition. It is published for reference only and will not be updated. For the more up-to-date code use the **master** branch instead. The configuration of the final competition binary can be found in `exps/z_/params.py`. 

To build a binary, go to an experiment directory and run `../../test_tools/simgen_depcomp.py`. It will create one or several subdirectories (if they don't exist) named after the individual parameter sets defined in the `params.py` file. To build for Cooja, go to `apps/crystal`, run `./make_cooja_depcomp.sh` and start Cooja with `depcomp.csc`.

***Disclaimer:*** *Although we tested the code extensively, Crystal is a research prototype that likely contains bugs. We take no responsibility for and give no warranties in respect of using the code.*

