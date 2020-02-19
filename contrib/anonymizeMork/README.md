# A Script to remove personal data from a mork file

This script can be used to remove personal data from a mork file,
so it can be shared on the internet.

## ⚠ Disclaimer ⚠

There is no guarantee that this script removes **all** personal data.
You should always open the converted file and check that all the personal data was removed.

### Usage

You need [Python](https://www.python.org/) installed on your system.

Let's assume that `path/to/morkFile.msf` is the full path
to the mork file that you want to anonymize.

Either specify the path as a command line argument
```shell script
./anonymizeMork.py path/to/morkFile.msf
```
or start the script without arguments and you will be asked to input the path:
```shell script
./anonymizeMork.py
Path to the mork file: path/to/morkFile.msf
```
If you use the last method, you can drag and drop the mork file into the terminal window
if your terminal supports that.

If the Script run successfully, it will print `Success` and create the new mork file at
`path/to/morkFile.msf-anonymize.msf`.
