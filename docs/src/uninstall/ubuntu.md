## Ubuntu

Uninstalling the driver involves removal of the Deb package and making sure the
boot image is regenerated:

```
sudo apt remove -y vastnfs-modules
sudo update-initramfs -u -k `uname -r`
```

To unload the driver, a reboot is recommended.
