/*
 * This script updates pages in the Kermit 95 manual with the version numbers
 * and dates of the executing Kermit 95 instance, along with modified dates
 * and other things.
 *
 * It relies on external functions provided by Kermit 95, so can not be run
 * by a regular REXX interpreter - it can only be run from inside K95 with
 * its "rexx" command.
 *
 * This script also relies on the RexxRE and RegUtil external function
 * packages. RegUtil as standard with Regina REXX, while RexxRE must be built
 * from source separately.
 *
 * Parameters are passed to this script via Kermit macros:
 *  K-95> .file_in = "in\\input_file.html"
 *  K-95> .file_out = "out\\output_file.html"
 *  K-95> rexx call mkdocs.rex
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

/* 1 = use 'DEV' as version tag, 0 = don't do that */
dev_mode = CKermit("return \m(dev_mode)")

/* 1 = get file modified times from git, 0 = don't do that */
touch_files = CKermit("return \m(touch_files)")

/* 1 = use https://kermitproject.org, 0 = use http://kermitproject.org */
use_https = CKermit("return \m(use_https)")

/* 1 = generate .html files, 0 = generate .htm files and fix up links */
use_html = CKermit("return \m(web_mode)")

src_dir = CKermit("return \m(src)")
dest_dir = CKermit("return \m(dest)")

insert_banner = CKermit("return \m(insert_banner)")
banner = CKermit("return \m(banner)")

/* This is a list of all html files that we'll replacing references to within
 * files */
html_files. = ''
new_fn= CKermit("return \m(html_files)")
do #=1  until  new_fn==''
parse var new_fn html_files.# ',' new_fn
end

/* REXX doesn't allow returning stems, get_version_tags will create the tags
 * stem as a global, and populate it with information about the Kermit version
 * this script is running under. */
call get_version_tags

/*
say "Replacement Values"
say "------------------"
say "            Herald ($herald$):" tags.herald
say "               Version Number ($ver$):" tags.ver_num
say "                  Release ($ver-rel$):" tags.ver_rel
say "            Full version ($ver-full$):" tags.ver_full
say "            Version Date ($ver-date$):" tags.ver_date
say "Short Version date ($ver-short-date$):" tags.ver_short_date
say "    Version date ISO ($ver-date-iso$):" tags.ver_date_iso
say "-----"
say "     File last update ISO ($last_update_iso$):" tags.last_update_iso
say "             File last update ($last_update$):" tags.last_update
say " File last update short ($last_update_short$):" tags.last_update_short
say "-----"
*/

/* Flags are:
 *       'x': use extended regular expressions;
 *       'c': respect case
 *       'n': newlines denote line-ends
 *       's': match variables to sub-expressions
 *       'd': re is the value delimiter (default)
 *       'v': re matches a var
 *       't': first var is a stem for all the fields
 */
tagre = ReComp('<!--\$([a-z\-]*)\$-->([^<]*)<!--/\$([a-z\-]*)\$-->', 'x')
replre = ReComp('\$!([a-z\-]*)\$', 'x')
bodyre = ReComp('<body', 'x')

do j=1 for #
    filename = html_files.j
    call process_html_file filename
end

call ReFree tagre
call ReFree replre
call ReFree bodyre

Say "processing done."
return

process_html_file:
arg filename

/* Get input and output file names. Because the names can contain spaces,
 * its easier to just pluck these out of Kermit rather than try and take
 * them as arguments.
 */

filename = lower(filename)

input_file = src_dir"/"filename
output_file = dest_dir""filename

if use_html = 0 then do
    /* And update the name of *this* file we're processing */
    output_file = changestr(".html",output_file,".htm")
end

say "UPDATE " input_file "--->" output_file

/* Git prevers the forward slashes, so save a copy of the filename before
 * we convert it to a windows-style path */
git_fn = input_file

input_file = changestr("/",input_file,"\")
output_file = changestr("/",output_file,"\")

/* This is the date from git, if we're asked to do that */
git_date = ''

if touch_files = 1 then do
    /* The usual REXX commands for running other programs or commands don't work
     * here as any system commands are actually handed off to Kermit 95 for
     * execution as it is in effect our shell. So we've got to ask K95 to run
     * git
     */
     git_date = CKermit('return \Fcommand(git log -1 --pretty="format:%ci" "'git_fn'" 2> nul)')
end

/*
say "input_file =" input_file
say "output_file =" output_file
say "git date =" git_date
*/

/* If Kermit can't supply gits date for this file, then use the existing
 * file date:
 */
if length(git_date) = 0 then do
    tags.last_update_iso = SysGetFileDateTime(input_file, 'm')
    parse var tags.last_update_iso f_date" "f_time
end
else do
    tags.last_update_iso = git_date
    parse var tags.last_update_iso f_date" "f_time" "f_tz
end

parse var f_date f_date_y"-"f_date_m"-"f_date_d

tags.last_update=left(date('W',f_date,'I'),3)" "left(date('M',f_date,'I'),3)" "f_date_d" "f_time" "f_date_y

tags.last_update_short = f_date_d" "date('M',f_date,'I')" "f_date_y

tags.file = filename

/* Ensure output file doesn't exist */
rcc = SysFileDelete(output_file)
/* rcc=0: success, rcc=2: doesn't exist */
if rcc > 2 then say "error deleting output file:" output_file "("rcc")"
if rcc = 1 then say "error deleting output file:" output_file "("rcc")"

do_banner = 0

do while lines(input_file) = 1

    if do_banner = 1 then do
        input_string = banner
        do_banner = 0
    end
    else do
        input_string = linein(input_file)
    end

    /*input_string = linein(input_file)*/
    output_string = input_string

    /* Normalise links to kermitproject.org to use a specific scheme */
    if use_https = 2 then do
        output_string = changestr("http://kermitproject.org",output_string,"//kermitproject.org")
        output_string = changestr("http://www.kermitproject.org",output_string,"//www.kermitproject.org")
        output_string = changestr("https://kermitproject.org",output_string,"//kermitproject.org")
        output_string = changestr("https://www.kermitproject.org",output_string,"//www.kermitproject.org")
    end
    else if use_https = 1 then do
        output_string = changestr("http://kermitproject.org",output_string,"https://kermitproject.org")
        output_string = changestr("http://www.kermitproject.org",output_string,"https://www.kermitproject.org")
    end
    else if use_https = 0 then do
        output_string = changestr("https://kermitproject.org",output_string,"http://kermitproject.org")
        output_string = changestr("https://www.kermitproject.org",output_string,"http://www.kermitproject.org")
    end

    if use_html = 0 then do
        /* index.html becomes kermit95.htm */
        output_string = changestr('="index.html',output_string,'="kermit95.htm')

        /* replace any references to .html files with .htm files */
        do k=1 for #
            old_fn = html_files.k
            new_fn = substr(html_files.k,1,length(html_files.k)-1)
            /*say old_fn "-->" new_fn*/
            output_string = changestr('="'old_fn,output_string,'="'new_fn)
        end
    end

    do forever
        matched = ReExec(tagre, output_string, 'FIELDS')

        if matched < 1 then leave

        /* For debugging
        say fields.0 'fields'
        do i = 1 to fields.0
        say i '»'fields.i'«'
        end*/


        tag_name = fields.1
        fulltag="<!--$"tag_name"$-->"fields.2"<!--/$"fields.3"$-->"

        if tag_name <> fields.3 then do
            /* The begin and end tags don't match - just eliminate them. */
            say "Error: Mismatched tag:" fulltag
            tag_value = ""
        end
        else do
            tag_name = changestr("-",tag_name,"_")
            interpret "tag_value=tags."tag_name
        end

        output_string = changestr(fulltag,output_string,tag_value)
    end

    do forever
        matched = ReExec(replre, output_string, 'FIELDS')
        if matched < 1 then leave

        /* For debugging
        say fields.0 'fields'
        do i = 1 to fields.0
        say i '»'fields.i'«'
        end
        */

        tag_name = fields.1
        fulltag="$!"tag_name"$"

        tag_name = changestr("-",tag_name,"_")
        interpret "tag_value=tags."tag_name

        /*
        say " ReTag name:" tag_name
        say "ReTag value:" tag_value
        say "-----"
        say "  Input String:" output_string
        say "Tag to Replace:" fulltag
        */
        output_string = changestr(fulltag,output_string,tag_value)
        /*
        say "        Result:" output_string
        */

    end

    rcc = lineout(output_file, output_string)

    /* Insert the banner if one has been specified. This facility is mostly used
     * to warn about preliminary (or out of date) documentation. It is inserted
     * right after the <body> tag.
     */
    if insert_banner = 1 then do
        found = ReExec(bodyre, output_string)
        if found = 1 then do
            /*Say "Insert banner" banner "after line" output_string
            rcc = lineout(output_file, banner) */
            do_banner = 1
        end
    end

end

/* It is important we close the input and output files, otherwise Regina
 * will crash when returning to Kermit 95 as it tries to clean up any unclosed
 * files. This appears to be a bug in Regina REXX 3.9.6 at least.
 */
rc = close(input_file)
rc = close(output_file)

/* Set file timestamp */
/*
say "Output File:" output_file
say "     f_date:" f_date
say "     f_time:" f_time
*/
rcc = SysSetFileDateTime(output_file, f_date, f_time)

return

get_version_tags: procedure expose tags.
    /* This sets tags. as a global as procedures aren't allowed to return stems.
     */

    input_file = CKermit("return \m(file_in)")
    git_date = CKermit("return \m(git_date)")
    dev_mode = CKermit("return \m(dev_mode)")

    tags.herald=CKermit("return \v(herald)")

    /* Accepted herald formats:
        1: Kermit 95 3.0.0 Pre-Beta.8, Sep  8 2025, for Windows
        2: Kermit 95 3.0.0, 8 Sep 2025, for Windows
        3: Kermit 95 3.0.0, Sep  8 2025, for Windows
        4: Kermit 95 3.0.0, 2025/03/22, for Windows

        Format 1 is standard for development-test and pre-stable release builds.
        The other formats are all for stable release builds with varying date
        formats. Format 2 was used by 2.1.3 and earlier, Format 3 is the same as
        Beta/RC/etc builds (the format __DATE__ gives), while Format 4 is what
        C-Kermit has used recently.
    */

    /* This regex should work for all development test and "unstable"
     * (alpha/beta/RC) release builds which have a herald of the form:
     *           Kermit 95 3.0.0 Pre-Beta.8, Sep  8 2025, for Windows
     * Fields:             --1-- -----2----  -3- -4- -5-
     */
    heraldre = ReComp('([[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+) ([[:alpha:][:digit:].-]+), ([[:alpha:]]+) +([[:digit:]]+) ([[:digit:]]+),', 'x')
    matched = ReExec(heraldre, tags.herald, 'FIELDS')
    call ReFree heraldre

    herald_format = 1

    if length(fields.1) = 0 then do
        /* Failed to parse herald. Maybe its a stable release build? Those
         * traditionally have a herald of the form:
         *           Kermit 95 2.1.3, 1 Jan 2003, for 32-bit Windows
         * Fields:             --1-- -2 -3- --4-
         */
        Say "Herald format 1 failed, trying format 2"
        heraldre = ReComp('([[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+), ([[:digit:]]+) ([[:alpha:]]+) ([[:digit:]]+),', 'x')
        matched = ReExec(heraldre, tags.herald, 'FIELDS')
        call ReFree heraldre

        herald_format = 2
    end

    if length(fields.1) = 0 then do
        /* Failed to parse herald. Maybe its a stable release build with a
         * beta-style date format?
         *           Kermit 95 2.1.3, Sep  8 2025, for 32-bit Windows
         * Fields:             --1--  -2- -3 --4-
         */
        Say "Herald format 2 failed, trying format 3"
        heraldre = ReComp('([[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+), ([[:alpha:]]+) +([[:digit:]]+) ([[:digit:]]+),', 'x')
        matched = ReExec(heraldre, tags.herald, 'FIELDS')
        call ReFree heraldre

        herald_format = 3
    end

    if length(fields.1) = 0 then do
        /* Failed to parse herald. Maybe its a stable release build with a
         * C-Kermit style date format?
         *           Kermit 95 2.1.3, 2025/03/22, for 32-bit Windows
         * Fields:             --1--  -2-- -3 -4
         */
        Say "Herald format 2 failed, trying format 3"
        heraldre = ReComp('([[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+), ([[:digit:]]+)/([[:digit:]]+)/([[:digit:]]+),', 'x')
        matched = ReExec(heraldre, tags.herald, 'FIELDS')
        call ReFree heraldre

        herald_format = 4
    end

    /* For debugging: output each of the things found in the herald
    say fields.0 'fields'
    do i = 1 to fields.0
    say i '?'fields.i'?'
    end*/

    if length(fields.1) = 0 then do
        say "Failed to parse herald!"
        exit 0
    end

    tags.ver_num = fields.1                             /* $ver$ */

    if herald_format = 1 then do
        /*
         *           Kermit 95 3.0.0 Pre-Beta.8, Sep  8 2025, for Windows
         * Fields:             --1-- -----2----  -3- -4- -5-
         */

        /* Development builds add the "Pre-" prefix to the release tag. That isn't
         * really part of the actual release tag though. This value primarily
         * appears in preliminary banner, and it doesn't really make sense to say
         * "this is for an upcoming version of K95 (3.0 Pre-Beta 8)", so strip it
         * off. */
        tags.ver_rel_actual = changestr("Pre-",changestr(".0",fields.2,""),"")

        tags.ver_rel = fields.2                             /* $ver_rel$ */

        ver_month_short = fields.3
        ver_day = fields.4
        ver_year = fields.5
    end
    else if herald_format = 2 then do
        /*           Kermit 95 2.1.3, 1 Jan 2003, for 32-bit Windows
         * Fields:             --1-- -2 -3- --4-
         */

         tags.ver_rel_actual = ""
         tags.ver_rel = ""

        ver_day = fields.2
         ver_month_short = fields.3
         ver_year = fields.4
    end
    else if herald_format = 3 then do
         /*           Kermit 95 2.1.3, Sep  8 2025, for 32-bit Windows
          * Fields:             --1--  -2- -3 --4-
          */

         tags.ver_rel_actual = ""
         tags.ver_rel = ""

         ver_month_short = fields.1
         ver_day = fields.2
         ver_year = fields.4
    end
    else if herald_format = 4 then do
         /*           Kermit 95 2.1.3, 2025/03/22, for 32-bit Windows
          * Fields:             --1--  -2-- -3 -4
          */

         tags.ver_rel_actual = ""
         tags.ver_rel = ""

         ver_year = fields.2
         ver_month_short = mnum_to_mshort(fields.3)
         ver_day = fields.4
    end

    if dev_mode = 1 then do
        tags.ver_rel = 'DEV'
    end

    tags.ver_full = tags.ver_num tags.ver_rel           /* $ver-full$ */

    if tags.ver_rel_actual = "" then tags.ver_full_actual = tags.ver_num
    else tags.ver_full_actual = tags.ver_num tags.ver_rel_actual

    ver_month_num = mshort_to_num(ver_month_short)
    ver_month = mnum_to_word(ver_month_num)

    tags.ver_short_date = ver_day ver_month_short ver_year       /* $ver-short-date$ */
    tags.ver_date = ver_day ver_month ver_year                   /* $ver-date$ */
    if ver_day < 10 then
        tags.ver_date_iso = ver_year"-"ver_month_num"-0"ver_day  /* $ver-date-iso$ */
    else
        tags.ver_date_iso = ver_year"-"ver_month_num"-"ver_day   /* $ver-date-iso$ */

    /* $ver_major$, $ver_minor$, $ver_rev$ */
    parse var tags.ver_num tags.ver_major"."tags.ver_minor"."tags.ver_rev

    tags.ver = tags.ver_major"."tags.ver_minor" "tags.ver_rel

    return

mshort_to_num: procedure
  mw.jan = '01'
  mw.feb = '02'
  mw.mar = '03'
  mw.apr = '04'
  mw.may = '05'
  mw.jun = '06'
  mw.jul = '07'
  mw.aug = '08'
  mw.sep = '09'
  mw.oct = '10'
  mw.nov = '11'
  mw.dec = '12'

  arg mon
  return mw.mon

mnum_to_word: procedure
  mw.01 = 'January'
  mw.02 = 'February'
  mw.03 = 'March'
  mw.04 = 'April'
  mw.05 = 'May'
  mw.06 = 'June'
  mw.07 = 'July'
  mw.08 = 'August'
  mw.09 = 'September'
  mw.10 = 'October'
  mw.11 = 'November'
  mw.12 = 'December'

  arg mon
  return mw.mon

mnum_to_mshort: procedure
  mw.01 = 'Jan'
  mw.02 = 'Feb'
  mw.03 = 'Mar'
  mw.04 = 'Apr'
  mw.05 = 'May'
  mw.06 = 'Jun'
  mw.07 = 'Jul'
  mw.08 = 'Aug'
  mw.09 = 'Sep'
  mw.10 = 'Oct'
  mw.11 = 'Nov'
  mw.12 = 'Dec'

  arg mon
  return mw.mon