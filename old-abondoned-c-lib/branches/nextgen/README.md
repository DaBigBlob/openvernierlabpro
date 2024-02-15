liblabpro
---------

* NOTE: If you are reading this on GitHub, be aware that this repository may not be up-to-date.
Use SVN to get the latest version from [SourceForge](https://sourceforge.net/p/liblabpro/code).

liblabpro will be (it isn't yet) a library for collecting data from [Vernier](https://www.vernier.com)
[LabPro](https://www.vernier.com/products/interfaces/labpro/) sensor interfaces. It is written
in C and uses [libusb](http://libusb.info) for maximum portability.

liblabpro does not do a lot of abstraction, but it does provide a somewhat more object-oriented
and easy-to-use interface than sending commands to the interface "by hand."
