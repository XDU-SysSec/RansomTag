# Introduction
Here is the hyperviosr-part of RansomTag project.
It consists of five models, i.e., the I/O Interceptor, Detector, Introspector, File System Monitor, and Tagger.
All these models are located in /drivers/usb/ransomtag.c.

Notes:
1. The I/O Interceptor model is called in /drivers/usb/usb_mscd.c.
2. The code of obtaining the CPU register for Introspector is located in /core/ransomtag_core.c.
3. The functionality of scanning the writing data for computing entropy is called in /drivers/usb/usb_mscd.c.


<br><br>

# Compile
1. Use make command in a Linux to compile the main elf file.
2. Go into /boot/login-simple and use make command to compile the login model.
3. Go into /boot/uefi-loader-login to compile UEFI loader.


<br><br>

# Usage
Put the compiled files, including bitvisor.elf, module1.bin, module2.bin, and loadvmm.efi, into the /EFI in the boot storage device.
Then reboot the host and select hypervisor to boot.
After typing the password, it go back to the window of choosing boot device.
Then, choose the Windows to boot.
Thus, the hypervisor runs under the OS.

   