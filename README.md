# diskclone
**Diskclone is a valid data disk cloning library running on the linux platform.Support MBR, GPT and LVM disk formats. Support EXT4 and NTFS partition format.**

# Features
  * Running on the linux platform.
  * Support MBR, GPT and LVM disk formats.
  * Support EXT4 and NTFS partition format.
  * Only copy valid data.
  
# Documentation
  Not yet.

# Limitations
  * Can only run on the linux platform.
  * Does not support other disk or partition formats other than in Features.
  
## Repository contents

The public interface is in include/hdtd/*.h.Callers only needs to include the 
include/hdtd.h file to use the public interface.Callers should not include or 
rely on the details of any other header files in this package.  
Those internal APIs may be changed without warning.
Call the example see main.c.
