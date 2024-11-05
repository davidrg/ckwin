/* This applies the file timestamps in modtimes.csv if the current file
 * timestamp is within a certain range
 */

/* Load RexxRE. Because the Rexx interpreter lives as long as Kermit 95 does,
 * RexxRE will stay loaded after the end of this script so if we try to load
 * it every time we'll get errors. So before we load it, check to see if its
 * already loaded.
 */
if rxFuncQuery("reloadfuncs") = 1 then do
    say "RexxRE not loaded. loading..."
    rcc = rxfuncadd('reloadfuncs', 'rexxre', 'reloadfuncs')
    if rcc > 0 then do
      msg=""
      if rcc = 40 then msg="(not found)"
      say "Failed to load RexxRE, error:" rcc msg
      return 1
    end
    call reloadfuncs
    say "RexxRE loaded successfully"
end

/* And do the same for regutil */
if rxFuncQuery("sysloadfuncs") = 1 then do
    say "RegUtil not loaded. loading..."
    rcc = rxfuncadd('sysloadfuncs', 'regutil', 'sysloadfuncs')
    if rcc > 0 then do
      msg=""
      if rcc = 40 then msg="(not found)"
      say "Failed to load RegUtil, error:" rcc msg
      return 1
    end
    call sysloadfuncs
    say "RegUtil loaded successfully"
end

directory = CKermit("return \m(manual_dir)")
modtime_file = CKermit("return \m(modtime_file)")

say "   Directory:" directory
say "Modtime file:" modtime_file

csvre = ReComp('([[:alpha:][:digit:]\._]*),([0-9T\:\-]*)','x')

say "Processing files..."
do while lines(modtime_file) = 1
    line = linein(modtime_file)
    matched = ReExec(csvre, line, 'FIELDS')

    /* For debugging
    say fields.0 'fields'
    do i = 1 to fields.0
    say i '»'fields.i'«'
    end*/

    if matched < 1 then do
        say "Regex match failed:" line
        leave
    end

    if fields.0 = 0 then do
       say "Regex match failed:" line
       leave
    end

    filename = directory""fields.1
    timestamp = fields.2

    /* HTML files in modtime.csv all use the '.html' file extension. Depending
     * on how the documentation has been built, they may instead end up with a
     * '.htm' file extension. This will deal with that. */
    rcc = state(filename)
    if rcc <> 0 then do
      say "File not found:" filename
      filename = substr(filename,1,length(filename)-1)
      say "Trying instead:" filename
      rcc = state(filename)
      if rcc <> 0 then do
        say "File still not found:" filename
        say "skipping."
        iterate
      end
    end

    file_ts = SysGetFileDateTime(filename, 'm')
    parse var file_ts file_date file_time

    /* For Debugging
    say "=========="
    say " Filename:" filename
    say "Timestamp:" timestamp
    say "  File TS:" file_ts
    say "File Date:" file_date
    */

    /* The commits that added the Kermit 95 manual to git were all dated
     * 2024-07-24. So any file that has that date doesn't have it because it was
     * modified on that date, but because it was added to git on that date. The
     * modtime.csv file contains the best known actual modified time for these
     * files. */
    if file_date = "2024-07-24" then do
        say "Reset modified time to" timestamp "for" fields.1
        parse var timestamp date"T"time
        rcc = SysSetFileDateTime(filename, date, time)
        if rcc <> 0 then do
            say "Failed to set time, rcc="rcc
        end
    end
end
say "Done"
