# ConPTY 

On modern Windows, Kermit 95 can be used as a local terminal using ConPTY. The
version of ConPTY built-in to Windows tends to be fairly out of date and comes
with limitations that may impact which of Kermit 95s terminal features 
applications can use.

If conpty.dll and openconsole.exe are bundled with Kermit 95, then Kermit 95
will use those instead of the built-in version shipped with Windows.

Microsoft currently provides signed conpty.dll and openconsole.exe binaries via
Nuget which you can download using the `getconpty.bat` script. From inside the
root of the Kermit 95 source tree:

```bat
setenv.bat
cd conpty
getconpty.bat
```

This will fetch nuget.exe if required, then fetch a specific version (set near
the top of `getconpty.bat`) of the ConPTY package and update the environment 
variables so that ConPTY and OpenConsole are picked up by the `mkdist.bat` 
script.

You only need to run this once - `setenv.bat` will pick up the downloaded ConPTY
package automatically next time.