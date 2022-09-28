# B4W1P3_INSTALL
Install program for Bible4W

See : https://github.com/dad98253/B4W1P3

Note that the anti-virus checksum feature can be disabled in this version (see README for B4W1P3).

Also note that this installer expects to have the Microsoft C Run-Time Library for DOS functions available (specifically, _dos_getftime). _dos_getftime is a cover routine for system call 0x57 to get the date and time that the specified file was last written. The time tag was a 16 bit number and the install program hid the checksum there. I remember having to change this later to a Windows call that only took a valid HH:MM:SS format. If I run into a copy of that change, I'll upload it later.
