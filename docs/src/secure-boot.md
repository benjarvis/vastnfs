# Signing kernel modules for secure boot

Some systems have Secure Boot enabled, meaning that unauthorized kernel code
cannot be loaded.
```
$ dmesg | grep secure
[    0.000000] secureboot: Secure boot enabled
```

Only trusted private keys can sign kernel modules that can be loaded.


## At package build time

If you are building the package yourself, likely that you already have a key
that that is used to sign its kernel modules.

For building a binary package where the module are signed, provide two
environment variables, `MODULE_SIGN_PRIV_KEY` and `MODULE_SIGN_PUB_KEY`
pointing to the key pair:

```
export MODULE_SIGN_PRIV_KEY=<path to private key>
export MODULE_SIGN_PUB_KEY=<path to public key>
./build.sh bin
```

The produced package will contain signed kernel modules.

Expect to see "Found Sign tool at" in the output to confirm that signing took place.


### If you don't have a key

If you are new to secure boot, it is possible to create your own key, and
enroll it in the BIOS. This is a general procedure is not specific to the
VAST NFS driver.


1. Create a key

```
openssl req -new -x509 -newkey rsa:2048 -keyout MOK.priv -outform DER -out MOK.der -nodes -days 3650 -subj "/CN=My Custom Key/"
```

2. Enroll the key

```
sudo mokutil --import MOK.der
```

You will be prompted to set a password. Make sure to remember this password, as
you'll need it during the next boot.


3. Reboot your system

After enrolling the key, reboot your system. During the boot process, you'll
see a blue screen with the title "MOK management." This is where you'll use the
password you set earlier.


4. Enroll the key in MOK

Follow these steps in the MOK management screen:

- Choose "Enroll MOK."
- Select "Continue."
- Choose "Yes" to confirm that you want to enroll the key.
- Enter the password you set earlier.
- Select "OK" to complete the enrollment process.
- Your system should now boot normally, and your custom key is enrolled in MOK.
