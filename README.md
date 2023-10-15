# isfshax installer

This installer allows the Installation and Removal of the [isfshax](https://github.com/isfshax/isfshax) superblock to/from the SLC.

The installer searches for `superblock.img` [b]and[\b] `superblock.img.sha` on the SD. The install will fail if not both files are present.

## launching the installer

It can either be launched by fw_img loader or from minute. fw_img loader reguires the fw.img to be build encrypted. For launching it from minute it needs to be built without encryption (`no_crypto = False` in `castify.py`)
