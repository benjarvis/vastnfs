# Development build

The following command should build the driver for the current kernel, assuming
that a compiler and the kernel headers are installed for the local machine.
Further development package of the relevant Linux distro may be needed.

```
make
```
When this command is issued, no package is being generated and instead the
kernel modules reside within the built source tree.

To load the newly built development version, the following command can be
issued (`sudo` is being used by the script):

```
./scripts/control.sh dev-reload
```
