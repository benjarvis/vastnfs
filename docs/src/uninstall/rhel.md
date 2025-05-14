# RHEL 8.x/9.0

Uninstalling the driver involves removal of RPM package and making sure the
boot image is regenerated:

```
sudo yum remove -y vastnfs
sudo dracut -f
```

To unload the driver, a reboot is recommended.
