AM2321
======

AM2321 temperature and humidity sensor library and example for Spark Core.

This device is manufactured by a China vendor called AoSong. It claims to use the I2C standard, however, the I2C protocol it is using is not entirely the same as what is expected from the I2C standard. Thus, custom I2C handling methods must be built to conform to the AM2321 I2C protocol in order to correctly read the information off the device.

After anything is read from the device, at least 2s must be waited before you can perform another read from the device again.

Ed Wios
Oct 2014

