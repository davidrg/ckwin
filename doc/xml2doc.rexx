/*
 * (C) Copyright 2025, David Goodwin
 * Portions (C) 2009-2020, Andrew J. Armstrong

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in
        the documentation and/or other materials provided with the
        distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


 ******************************************************************************
 * This program converts the Kermit 95 Control Sequences documentation that
 * lives in ctlseqs.xml to a number of (currently HTML formatted) documents.
 * This program and the XML schema it consumes were developed specially for the
 * needs of representing the content of these particular documents..
 *
 * It has four optional positional parameters:
 *    - If outputs should be produced in development mode (1)
 *      or release mode (0). This affects the version number inserted into
 *      documents. Default is 1 (development mode)
 *    - The filename for a banner to be inserted at the top of each file if
 *      desired. The file should contain only a single line - all other lines
 *      are ignored. Default is '' (no banner)
 *    - show warnings (1) or don't show warnings (0). Default is 1
 *    - XML file to process. If not specified, "ctlseqs.xml" in the current
 *      directory is assumed
 * Some additional settings not available via command line options are below.
 *
 * This program relies on certain Kermit 95 integration features, and as such
 * can not be run from a standalone Rexx interpreter. It must be run from
 * within Kermit 95, for example:
 *    [C:\kermit\] K-95> rexx call xml2doc 1, 'preliminary.html', 0
 * If you want to do all of this from the shell, the method of passing
 * parameters to rexx (with the ',' character) is incompatbile with Kermit 95s
 * way of receiving multiple commands to execute on the command line (also with
 * the ',' character). For this purpose, a simple wrapper script is provided:
 *    C:\kermit> k95.exe -Y -# 94 -C "take xml2doc.ksc 1 {preliminary.html}, exit"
 *
 *
 * The REXX XML Parser used to read the XML document is appended to this program
 * to allow it to be self-contained. The XML parser comes from
 * Andrew J. Armstrong and is licensed under the simplified BSD license:
 *      https://github.com/abend0c1/rexxxmlparser
 */

parse arg param_devmode, param_bannerfile, param_verbose, param_inputfile


settings.output_file = ""            /* Comes from the XML file */

/* Which terminals should be included in the term-ctlseqs comparison tables */
settings.compare_terminals = 'vt100 vt102 vt132 vt220 vt420 vt510 vt520 vt525 xterm tt'

/* Filtering for the to-do output. The following terminals will be eligible to
 * be included in the output. Additional filtering can be done by the
 * "todoOutputFilter" procedure further down. */
settings.todo_terminals = 'vt102 vt220 vt420 xterm tt'

/* Settings set via command line arguments */
settings.dev_mode = param_devmode    /* Use 'DEV' as the release tag */
settings.verbose = param_verbose     /* Log warnings about warnable things */
settings.input_file = strip(param_inputfile)  /* XML file to read */

if settings.dev_mode <> '0' then settings.dev_mode = 1
if settings.verbose <> '1' then settings.verbose = 0
if settings.input_file = '' then settings.input_file = "ctlseqs.xml"

/* Optional preliminary information banner, compatible with the K95 mkdocs
 * script: */
settings.preliminary_banner_file = strip(param_bannerfile)

/* TODO: - It would be nice to issue a warning on conflicting control sequences
 *       - Hide not-implemented things by default
 *       - Javascript to show/hide not-implemented things
 *         - This will be difficult were the not-implemented section is just noting
 *           that some part of the overall feature is not implemented: those should
 *         remain.
 *       - Javascript to show only unbadged or with-specific-badge elements
 *         (show only escapes in effect for particular term type)
 *         For terminals with multiple personalities, this should probably include
 *         badges for all their personalities too! Basically all the stuff you
 *         could use without having to do a "set term type"
 *       - Javascript should ideally work on vintage browsers
 *       - HTML should ideally work on vintage browsers
 *       - For filtering by badge, going to need some way of dealing with things
 *         like "ESC [" which is by default CSI so unbadged, but for h19 its hold
 *         screen (badged)
 *       - Provide a better way of representing all the set/reset/query mode
 *         parameters. Ideally all the modes could be described in one place
 *         with get/set/request descriptions/references/implementation flags,
 *         and a mode group identifier. Then SM/RM can just reference the mode
 *         group and whether the set/reset/query content is desired. This would
 *         make keeping the content up-to-date and in-sync *much* easier. Eg:
 *         <modes>
 *            <mode-group id="mode-ansi">
 *               <mode identifier="60" mnemonic=DECHCCM" title="Horizontal cursor coupling">
 *                  <set-mode title="Enable horizontal cursor coupling" not-implemented=true>
 *                    <p>aragraphs describing the implementation, etc.
 *                  </set-mode>
 *               </mode>
 *            </mode-group>
 *         </modes>
 *         the mode and set-mode together would behave like a section, with child
 *         attributes overriding the parent where specified, and together they'd
 *         be rendered like a parameter section. Then in SetMode, you could just
 *         do:  <mode-group-ref group-id="mode-ansi" operation="set"/>
 *         and it all gets included at that point.
 */

/*****************************************************************************
 * Begin Program *************************************************************
 *****************************************************************************/

/* Load RegUtil. Because the Rexx interpreter lives as long as Kermit 95 does,
 * RegUtil will stay loaded after the end of this script so if we try to load
 * it every time we'll get errors. So before we load it, check to see if its
 * already loaded.
 */
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

if settings.dev_mode = 1 then say "Building in Development mode"
if settings.verbose = 1 then say "Warnings are enabled"

k95info. = ''
call getK95Info

call initParser

say "Parsing "settings.input_file"..."
rc = parseFile(settings.input_file)
say "Done parsing."

say "===================="

doc = getDocumentElement()

/* And do the same to find badge definitions */
Say "Finding Badge Definitions..."
badgeSet. =''
badgeSet.badgeNames = ''
badgeSet.groupNames = ''
badgeSet.badges. = ''
badgeSet.groups. = ''
call buildBadgeSet doc
Say "Found" words(badgeSet.badgeNames) "badges and" words(badgeSet.groupNames) "groups"

Say "Finding reference source definitions..."
refSet. =''
refSet.refKeys = ''
refSet.refs. = ''
call buildRefSet doc
Say "Found" words(refSet.refKeys) "reference sources"

/* Find all ctlseq sections that share a ctlseq string with another ctlseq
 * section. This is only valid if the sections all specify a different set of
 * allowed terminals */
Say "Finding control sequence definitions..."
ctlseqStrs. = ''
ctlseqStrs.0 = 0
ctlseqEls. = ''
ctlseqEls.0 = 0
ctlseqDups. = ''
ctlseqDups.0 = 0
call buildCtlSeqArrays doc
say "Found" ctlseqStrs.0 "control sequences"

/* TODO: Check for any ctlseqs that have overlapping allowed terminals. Eg two
 * sections with the ctlseq string and no badges specified, or two ctlseqs
 * that both specify the same terminal as being allowed */

/* Add "exclude-badges" attributes to any ctlseq sections that don't have a
 * list of badges and share a ctlseq string with another ctlseq section */
say "Computing exclusions..."
call setExcludedBadges
say "done."

/* Walk the document to find any sections that belong in the TOC */
Say "Building TOC..."
toc.0 = 0
call buildToc doc, 0, 1
Say "TOC has" toc.0 "items"

call produceOutputs doc

call destroyParser
exit 0

/* This function allows whats included in the to-do output to be filtered.
 * The function is called for each ctlseq or parameter, and if it returns 0
 * then it will be excluded from output.
 */
todoOutputFilter: procedure expose g.
    parse arg el, supportedTerminals, badges, mnemonic, ctlseqs

    /* TODO: Require VT420 in supportedTerminals, or xterm+tt in supported
     * TODO: terminals, or any Sun control sequences and other easy things.
     */

    is_vt102 = 0
    is_vt220 = 0
    is_vt420 = 0
    is_xterm = 0
    is_tt = 0

    if wordpos('vt102', supportedTerminals) <> 0 then is_vt102 = 1
    if wordpos('vt220', supportedTerminals) <> 0 then is_vt220 = 1
    if wordpos('vt420', supportedTerminals) <> 0 then is_vt420 = 1
    if wordpos('xterm', supportedTerminals) <> 0 then is_xterm = 1
    if wordpos('tt', supportedTerminals) <> 0 then is_tt = 1

    if is_vt102 = 1 | is_vt220 = 1 | is_vt420 = 1 | (is_xterm = 1 & is_tt = 1) then do
        return 1
    end

    if wordpos(mnemonic, 'SUNBOW SUNWOB SUNSCRL CHA REP SCORC SCOSC') <> 0 then return 1

    id = getAttribute(el, "id")
    if wordpos(id, 'rxvt-stbt rxvt-stbk sgr-29-co cursor-steady-block steady-underline xt-urgency xt-raise-window') <> 0 then return 1

    return 0

buildToc: procedure expose g. toc.
    parse arg el, level

    name = getName(el)
    if name = 'section' then do
        if hasAttribute(el, 'id') then do
            addToToc = 0
            if hasAttribute(el, 'toc') then do
                x = getAttribute(el, 'toc')
                if x = 'true' then addToToc = 1
            end

            if addToToc == 1 then do
                /* Add to toc */
                ind = toc.0 + 1
                toc.0 = ind
                toc.ind.id = getAttribute(el, 'id')
                toc.ind.title = getAttribute(el,'title')
                toc.ind.lvl = level
            end
        end
    end

    if name = 'document' | name = 'section' then do
        /* Then do each of its children */
        child = getFirstChild(el)
        do while child <> ''
            call buildToc child, level+1
            child = getNextSibling(child)
        end
    end

    return

buildRefSet: procedure expose g. refSet.
    parse arg docEl

    /* Loop over elements in the Document element */
    docChild = getFirstChild(docEl)
    do while docChild <> ''
        if \ isElementNode(docChild) then do
            docChild = getNextSibling(docChild)
            iterate
        end

        name = getName(docChild)
        if name = 'references' then do
            refChild = getFirstChild(docChild)
            do while refChild <> ''
                if getName(refChild) = 'refsrc' then do
                    refName = getAttribute(refChild, "name")
                    refKey = getAttribute(refChild, "key")
                    refUri = getAttribute(refChild, "uri")
                    refUriType = getAttribute(refChild, "uri-type")
                    pageOff = 0
                    if hasAttribute(refChild, "pdf-page-offset") then do
                        pageOff = getAttribute(refChild, "pdf-page-offset")
                    end
                    refSort = getAttribute(refChild, "sort-order")
                    ttbadge = getAttribute(refChild, "badge")
                    refSet.refKeys = strip(refSet.refKeys" "refKey)
                    refSet.refs.refKey.title = refName
                    refSet.refs.refKey.uri = refUri
                    refSet.refs.refKey.uriType = refUriType
                    refSet.refs.refKey.sortOrder = refSort
                    refSet.refs.refKey.pageOffset = pageOff
                    refSet.refs.refKey.badge = ttbadge
                end
                refChild = getNextSibling(refChild)
            end
        end

        docChild = getNextSibling(docChild)
    end
    return

buildBadgeSet: procedure expose g. badgeSet.
    parse arg docEl

    badgesTmp. = ''
    badgesTmp.0 = 0

    /* Loop over elements in the Document element */
    docChild = getFirstChild(docEl)
    do while docChild <> ''
        if \ isElementNode(docChild) then do
            docChild = getNextSibling(docChild)
            iterate
        end

        name = getName(docChild)
        if name = 'badges' then do
            badgeChild = getFirstChild(docChild)
            do while badgeChild <> ''
                if getName(badgeChild) = 'badge-def' then do
                    badgeName = getAttribute(badgeChild, "name")
                    colourAttr = getAttribute(badgeChild, "colour")
                    fgColorAttr = getAttribute(badgeChild, "fg-color")
                    commentAttr = getAttribute(badgeChild, "comment")
                    sortOrder = 0
                    if hasAttribute(badgeChild, "sort-order") then do
                        sortOrder = getAttribute(badgeChild, "sort-order")
                    end


                    idx = badgesTmp.0 + 1
                    badgesTmp.idx.bname = badgeName
                    badgesTmp.idx.colour = colourAttr
                    badgesTmp.idx.fgColor = fgColorAttr
                    badgesTmp.idx.comment = commentAttr
                    badgesTmp.idx.order = sortOrder
                    badgesTmp.0 = idx
                end
                badgeChild = getNextSibling(badgeChild)
            end
        end
        if name = 'badgegroups' then do
            badgeGroupEl  = getFirstChild(docChild)
            do while badgeGroupEl <> ''
                if isElementNode(badgeGroupEl) then do
                    groupName = getAttribute(badgeGroupEl, "name")
                    badgeSet.groupNames = strip(badgeSet.groupNames' 'groupName)
                    refEl = getFirstChild(badgeGroupEl)
                    do while refEl <> ''
                        if isElementNode(refEl) then do
                            refElName = getName(refEl)
                            if refElName = 'badge' then do
                                badgeName = getAttribute(refEl, "name")
                                badgeSet.groups.groupName = strip(badgeSet.groups.groupName' 'badgeName)
                            end
                            else if refElName = 'badgegroup-ref' then do
                                /* Its a reference to another badge group. For this to
                                 * be valid it must occur earlier in the XML doc */
                                otherGroupName = getAttribute(refEl, "name")
                                badgeSet.groups.groupName = strip(badgeSet.groups.groupName' 'badgeSet.groups.otherGroupName)
                            end
                        end
                        refEl = getNextSibling(refEl)
                    end
                end
                badgeGroupEl = getNextSibling(badgeGroupEl)
            end
        end

        docChild = getNextSibling(docChild)
    end

    /* Sort the badges */

    if badgesTmp.0 > 0 then do
        do i = 1 to badgesTmp.0 - 1
            do j = i + 1 to badgesTmp.0

                order_i = badgesTmp.i.order
                order_j = badgesTmp.j.order

                name_i = badgesTmp.i.bname
                name_j = badgesTmp.j.bname

                if order_i = order_j then do
                    order_i = name_i
                    order_j = name_J
                end

                if order_i > order_j then do /* Swap entries */

                    colour_i = badgesTmp.i.colour
                    fgColour_i = badgesTmp.i.fgColor
                    comments_i = badgesTmp.i.comments

                    badgesTmp.i.bname = badgesTmp.j.bname
                    badgesTmp.i.colour = badgesTmp.j.colour
                    badgesTmp.i.fgColor = badgesTmp.j.fgColor
                    badgesTmp.i.comments = badgesTmp.j.comments

                    badgesTmp.j.bname = name_i
                    badgesTmp.j.colour = colour_i
                    badgesTmp.j.fgColor = fgColour_i
                    badgesTmp.j.comments = comments_i
                end
            end
        end
    end

    do i = 1 to badgesTmp.0
        badgeName = badgesTmp.i.bname
        badgeSet.badgeNames = strip(badgeSet.badgeNames" "badgeName)
        badgeSet.badges.badgeName.colour = badgesTmp.i.colour
        badgeSet.badges.badgeName.fgColor = badgesTmp.i.fgColor
        badgeSet.badges.badgeName.comment = badgesTmp.i.comment
    end

    return


/* Finds all the control sequences in the document and puts them in the
 * ctlseqStrs array, and puts the associated section it was found in at the
 * same index in the ctlseqEls array */
buildCtlSeqArrays: procedure expose g. ctlseqEls. ctlseqStrs. ctlseqDups.
    parse arg el

    name = getName(el)

    if name = 'section' then do
        if hasAttribute(el, "role") & ((getAttribute(el, "role") == "ctlseq") | (getAttribute(el, "role") == "ctlchar")) then do
            ctlseqs = getSectionCtlSeqs(el,0)
            do I = 1 by 1 until ctlseqs = ""
                parse var ctlseqs ctlseq"<br>"ctlseqs
                if ctlseq <> '' then do
                    /* Strip any line breaks */
                    ctlseq = changestr("0D 0A"x,ctlseq," ")
                    ctlseq = changestr("0D"x,ctlseq," ")

                    /* Strip excess spaces between elements in the ctlseq */
                    ctlseq = space(ctlseq)

                    /* And trip any leading and trailing spaces */
                    ctlseq = strip(ctlseq)

                    /* Append to the arrays */
                    ind = ctlseqStrs.0 + 1
                    ctlseqStrs.ind = ctlseq
                    ctlseqEls.ind = el
                    ctlseqStrs.0 = ind
                    ctlseqEls.0 = ind

                    /* Now check for duplicates */
                    do j = 1 to ind - 1
                        if ctlseqStrs.j = ctlseq then do
                            /*say "Ctlseq #"ind "duplicates ctlseq #"j": "ctlseq*/

                            new = 1
                            do k = 1 to ctlseqDups.0
                                if ctlseqDups.k = ctlseq then do
                                    /*say "Duplicate is already known"*/
                                    new = 0
                                end
                            end

                            if new = 1 then do
                                k = ctlseqDups.0 + 1
                                ctlseqDups.k = ctlseq
                                ctlseqDups.0 = k
                            end
                        end
                    end
                end
            end
        end
    end

    child = getFirstChild(el)
    do while child <> ''
        call buildCtlSeqArrays child
        child = getNextSibling(child)
    end

    return

setExcludedBadges: procedure expose g. badgeSet. ctlseqStrs. ctlseqEls. ctlseqDups. settings.

/*
For each duplicate:
  badge_list = ''
  global_match = 0
  loop over ctlseqStrs finding matches
    If the match has no groups or badges, store it as the global_match
    If the match has groups or badges, expand groups and append all badges to badge_list
  if global_match !=
    set global_match.excludes_badges = badge_list
*/

    do i = 1 to ctlseqDups.0
        badge_list = ''
        global_match = 0
        dup_ctlseq = ctlseqDups.i

        do j = 1 to ctlseqStrs.0
            ctlseq = ctlseqStrs.j

            if dup_ctlseq = ctlseq then do
                ctlseq_el = ctlseqEls.j

                badges = getSectionBadges(ctlseq_el)

                if badges = '' then do
                    /*say "found dup instance with no badges for ctlseq: "ctlseq*/
                    global_match = ctlseq_el
                end
                else do
                    /*say "For dup ctlseq '"ctlseq"' found badges:" badges*/
                    badge_list = badge_list' 'badges
                end
            end
        end

        if (global_match <> 0) & (badge_list <> '') then do
            badge_list = space(badge_list)
            badge_list = strip(badge_list)

            /*say "for generic ctlseq" dup_ctlseq "found exclusions:" badge_list*/

            exclusions = ''
            if hasAttribute(global_match, 'exclude-badges') then do
                exclusions = getAttribute(global_match, 'exclude-badges')
            end

            do j = 1 by 1 until badge_list = ""
                parse var badge_list badge" "badge_list
                if badge <> '' then do
                    if wordpos(badge, exclusions) == 0 then do
                        exclusions = exclusions' 'badge
                    end
                end
            end

            if exclusions <> '' then do
                call setAttribute global_match,'exclude-badges',exclusions
            end
        end
        /*else do
            say "for ctlseq" dup_ctlseq "there were no unguarded variants"
        end*/
    end

    return

update_banner: procedure expose settings. k95info.
    if settings.dev_mode = 1 & settings.preliminary_banner_file <> '' then do
        Say "Reading banner from" settings.preliminary_banner_file
        settings.preliminary_banner = ''
        settings.preliminary_banner = linein(settings.preliminary_banner_file)
        call close settings.preliminary_banner_file
        if settings.preliminary_banner <> '' then do
            banner = settings.preliminary_banner
            banner = changestr("$!herald$",banner,k95info.herald)
            banner = changestr("$!ver$",banner,k95info.version_number)
            banner = changestr("$!ver-rel$",banner,k95info.version_release)
            banner = changestr("$!ver-rel-actual$",banner,k95info.version_release_actual)
            banner = changestr("$!ver-full$",banner,k95info.version_full)
            banner = changestr("$!ver-date$",banner,k95info.version_date)
            banner = changestr("$!ver-short-date$",banner,k95info.version_short_date)
            banner = changestr("$!ver-date-iso$",banner,k95info.version_date_iso)
            banner = changestr("$!file$",banner,settings.output_file)
            banner = changestr("$!ver-major$",banner,k95info.version_major)
            banner = changestr("$!ver-minor$",banner,k95info.version_minor)
            banner = changestr("$!ver-rev$",banner,k95info.version_rev)
            /*banner = changestr("$!$",banner,k95info.)*/
            settings.preliminary_banner = banner
        end
    end
    return

/* Parameters:
 *      docEl       Document element
 */
produceOutputs: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg docEl

    say "Producing outputs..."

    docTitle = getAttribute(docEl,"title")

    /* Find the outputs element */
    docChild = getFirstChild(docEl)
    do while docChild <> ''
        if \ isElementNode(docChild) then do
            docChild = getNextSibling(docChild)
            iterate
        end
        name = getName(docChild)
        if name = 'outputs' then do
            outChild = getFirstChild(docChild)
            do while outChild <> ''
                if getName(outChild) = 'html' then do
                    /* Produce an HTML output */
                    style = ""
                    script = ""
                    settings.title = docTitle
                    if hasAttribute(outChild, "title") then settings.title = getAttribute(outChild, "title")

                    htmlChild = getFirstChild(outChild)
                    do while htmlChild <> ''
                        if getName(htmlChild) = 'style' then do
                            styleChild = getFirstChild(htmlChild)

                            do while styleChild <> ''
                                if isTextNode(styleChild) then do
                                    style = " "getText(styleChild)
                                end
                                styleChild = getNextSibling(styleChild)
                            end
                        end
                        if getName(htmlChild) = 'script' then do
                            scriptChild = getFirstChild(scriptChild)
                            do while scriptChild <> ''
                                if isTextNode(scriptChild) then do
                                    script = " "getText(scriptChild)
                                end
                                scriptChild = getNextSibling(scriptChild)
                            end
                        end
                        htmlChild = getNextSibling(htmlChild)
                    end

                    settings.output_file = getAttribute(outChild, "filename")

                    call update_banner

                    /* Ensure output file doesn't exist */

                    say "Removing output file" settings.output_file "if it exists"
                    rcc = SysFileDelete(settings.output_file)
                    /* rcc=0: success, rcc=2: doesn't exist */
                    if rcc > 2 then say "error deleting output file:" settings.output_file "("rcc")"
                    if rcc = 1 then say "error deleting output file:" settings.output_file "("rcc")"

                    type = getAttribute(outChild, "type")
                    if type = 'document' then do
                        say "Building HTML document..."
                        call doDocumentHTML docEl, 1, 0, style, script
                        say "done."
                    end
                    else if type = 'outline' then do
                        say "Building HTML Outline document..."
                        call doOutlineHTML docEl, 1, 0, style, script
                        say "done."
                    end
                    else if type = 'ctlseq-table' then do
                        say "Building HTML Ctlseq-Table document..."
                        call doCtlseqTableHTML docEl, 1, 0, style, script
                        say "done."
                    end
                    else do
                        say "Building HTML "type" document..."
                        call doHTML docEl, 1, 0, style, script, '', type
                        say "done."
                    end

                    call close settings.output_file
                end
                outChild = getNextSibling(outChild)
            end
            /* Done! */
            return
        end
        docChild = getNextSibling(docChild)
    end

    return


outputElBadges: procedure expose g. badgeSet. settings.
    parse arg el,indentLevel,hiddenBadges

    excludeBadges = hiddenBadges
    outputBadges = ""

    if hasAttribute(el, "exclude-badges") then do
        excludeBadges = strip(hiddenBadges' 'getAttribute(el, "exclude-badges"))
    end

    have_badges = 0
    if hasAttribute(el, "groups") | hasAttribute(el, "badges") then do
        call outputHtml indentLevel,'<span class="el_badges">'
        indentLevel = indentLevel + 1
        have_badges = 1
    end

    if hasAttribute(el, "groups") then do
        groupNames = getAttribute(el, "groups")
        do I = 1 by 1 until groupNames = ""
            parse var groupNames groupName" "groupNames
            if groupName <> '' then do
                call outputBadgeGroup groupName,indentLevel,excludeBadges
            end
        end
    end

    if hasAttribute(el, "badges") then do
        badgeNames = getAttribute(el, "badges")
        do I = 1 by 1 until badgeNames = ""
            parse var badgeNames badgeName" "badgeNames
            if badgeName <> '' then do
                if wordpos(badgeName, excludeBadges) == 0 then do
                    call outputBadge badgeName,indentLevel
                end
            end
        end
    end

    if have_badges == 1 then do
        indentLevel = indentLevel - 1
        call outputHtml indentLevel,'</span>'
    end


    return

/* Gets all badges applied to a section */
getSectionBadges: procedure expose g. badgeSet.
    parse arg el

    /* TODO: Add on getHiddenSectionBadges() */

    exclusions = ''
    if hasAttribute(global_match, 'exclude-badges') then do
        exclusions = getAttribute(global_match, 'exclude-badges')
    end

    badges = ''
    if hasAttribute(el, "badges") then badges = getAttribute(el, "badges")

    if hasAttribute(el, "groups") then do
        group_names = getAttribute(el, "groups")
        do i = 1 by 1 until group_names = ""
            parse var group_names group_name" "group_names
            if group_name <> '' then do
                /* see outputBadgeGroup to get badges */
                group_badges = badgeSet.groups.group_name

                do j = 1 by 1 until group_badges = ""
                    parse var group_badges badge" "group_badges
                    if badge <> '' then do
                        if wordpos(badge, badges) == 0 then do
                            if wordpos(badge, exclusions) == 0 then do
                                badges = badges' 'badge
                            end
                        end
                    end
                end
            end
        end
    end

    badges = strip(badges)

    return badges


/* Gets all badges that should be hidden within a section and its children */
getHiddenSectionBadges: procedure expose g. badgeSet.
    parse arg el

    badges = ''
    if hasAttribute(el, 'hide-badges') then do
        badges = getAttribute(el, 'hide-badges')
    end

    if hasAttribute(el, "hide-groups") then do
        group_names = getAttribute(el, "hide-groups")
        do i = 1 by 1 until group_names = ""
            parse var group_names group_name" "group_names
            if group_name <> '' then do
                /* see outputBadgeGroup to get badges */
                group_badges = badgeSet.groups.group_name

                do j = 1 by 1 until group_badges = ""
                    parse var group_badges badge" "group_badges
                    if badge <> '' then do
                        if wordpos(badge, badges) == 0 then do
                            badges = badges' 'badge
                        end
                    end
                end
            end
        end
    end

    badges = strip(badges)

    return badges


/* Gets the control sequences associated with a section element. If multiple
 * control sequences are found, they will be separated with '<br>' whether the
 * output is HTML or not ("<br>" is unlikely to ever appear in a real control
 * sequence, so its as good a separator for plain text as any)
 */
getSectionCtlSeqs: procedure expose g. settings.
    parse arg el,html

    line = ''
    if hasAttribute(el, "ctlseq") then line = getAttribute(el, "ctlseq")

    sectionChild = getFirstChild(el)
    do while sectionChild <> ''
        if isElementNode(sectionChild) then do
            if getName(sectionChild) = "ctlseq" then do
                if line <> '' then line = line'<br>'

                ctlseqChild = getFirstChild(sectionChild)
                do while ctlseqChild <> ''
                    if isTextNode(ctlseqChild) then do
                        t = getText(ctlseqChild)
                        line = line''t
                    end
                    else do
                      n = getName(ctlseqChild)
                      if (n = 'param-char') | (n = 'param-single') | (n = 'param-multi') | (n = 'param-text') then do
                        param = parameterToString(ctlseqChild,html)
                        line = line ' ' param
                      end
                    end

                    ctlseqChild = getNextSibling(ctlseqChild)
                end
            end
        end
        sectionChild = getNextSibling(sectionChild)
    end

    return line

/* Gets all badges applied to a section */
getSectionSupportedTerminals: procedure expose g. refSet.
    parse arg el

    terminals = ''
    tt = ''
    excl = ''

    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)
        if name = 'ref' then do
            src = getAttribute(child, 'src')
            parse var src key":"src

            term = refSet.refs.key.badge

            tt = tt' 'term
        end
        else if name = 'termsupp' then do
            first = getAttribute(child, "first")
            addit = getAttribute(child, "additional")
            ex = getAttribute(child, "excluded")

            tt = tt' 'first' 'addit
            excl = excl' 'ex
        end

        child = getNextSibling(child)
    end

    tt = strip(tt)

     do i = 1 by 1 until tt = ""
         parse var tt term' 'tt
         /* Handle series of terminals:
          *   vt50 implies vt50 and vt50h and vt52
          *   VT50H implies vt50h vt52
          *   VT100 implies vt100 and vt102
          *   VT510 implies vt510 and vt520 and vt525
          *   VT520 implies vt520 and vt525
          */

         if term = 'vt50' then tt = tt' vt50h vt52'
         else if term = 'vt50h' then tt = tt' vt52'
         else if term = 'vt100' then tt = tt' vt102'
         else if term = 'vt102' then tt = tt' vt132'
         else if term = 'vt510' then tt = tt' vt520 vt525'
         else if term = 'vt520' then tt = tt' vt525'

         if wordpos(term, terminals) == 0 then do
            if wordpos(term, excl) == 0 then do
                terminals = terminals' 'term
            end
         end
     end

    return terminals

parameterToString: procedure expose g. settings.
    parse arg el,html
    c = ''

    elName = getName(el)
    if elName = 'param-char'
    then c = 'C'
    else if elName = 'param-single'
    then c = 'Ps'
    else if elName = 'param-multi'
    then c = 'Pm'
    else if elName = 'param-text'
    then c = 'Pt'
    else return

    if hasAttribute(el, "symbol")
    then c = getAttribute(el, "symbol")

    /* Can't output without a parameter symbol */
    if c = '' then return

    if html = 1 then line = '<span class="param">'c'</span>'
    else line = c

    if hasAttribute(el, "value")
    then line = line '=' getAttribute(el, "value")

    if hasAttribute(el, "description")
    then do
        if html = 1 then line = line '=>' getAttribute(el, "description")
        else line = line '&#8658;' getAttribute(el, "description")
    end

    return line

outputRef: procedure expose settings. refSet.
    parse arg indentLevel,ref,comments

    parse var ref key":"src

    title = refSet.refs.key.title
    uri = refSet.refs.key.uri
    uriType = refSet.refs.key.uriType
    pageOffset = refSet.refs.key.pageOffset

    if uri <> '' then do
        if uriType == 'anchor' then src = uri"#"src
        else if uriType == 'prefix' then src = uri""src
        else if uriType == 'pdf-page'then do
            pageNum = src
            if pageOffset <> 0 then do
                pageNum = src - pageOffset
                pageNum = pageNum'/'src
            end
            if comments = '' then title = title' (p'pageNum')'
            else title = title' (p'pageNum', 'comments')'
            src = uri"#page="src
        end
        else if uriType == 'direct' then src = src

        if (comments <> '') & (uriType <> 'pdf-page') then do
            title = title' ('comments')'
        end
    end

    call outputHtml indentLevel,'<a class="ref" data-ref="'ref'" href="'src'">'title'</a>'
    return

outputBadge: procedure expose settings.
    parse arg badgeName,indentLevel
    call outputHtml indentLevel,'<span class="badge 'badgeName'">'badgeName'</span>'
    return

outputBadgeGroup: procedure expose badgeSet. settings.
    parse arg groupName,indentLevel,excludeBadges

    doneBadges = ""

    badges = badgeSet.groups.groupName
    do I = 1 by 1 until badges = ""
        parse var badges badgeName" "badges
        if badgeName <> '' then do
            /* Don't output any badges we've been asked to exclude */
            if wordpos(badgeName, excludeBadges) == 0 then do
                /* And don't output any badges we've been asked not to */
                if wordpos(badgeName, doneBadges) == 0 then do
                    doneBadges = doneBadges" "badgeName
                    call outputBadge badgeName, indentLevel
                end
            end
        end
    end
    return

getK95InfoValue: procedure expose k95info. settings.
    parse arg val

    if val = 'herald' then return strip(k95info.herald)
    else if val = 'version-number' then return strip(k95info.version_number)
    else if val = 'version-full' then return strip(k95info.version_full)
    else if val = 'version-major' then return strip(k95info.version_major)
    else if val = 'version-minor' then return strip(k95info.version_minor)
    else if val = 'version-rev' then return strip(k95info.version_rev)
    else if val = 'release' then return strip(k95info.version_release)
    else if val = 'release-actual' then return strip(95info.version_release_actual)
    else if val = 'version-date' then return strip(k95info.version_date)
    else if val = 'version-date-short' then return strip(k95info.version_short_date)
    else if val = 'version-date-iso' then return strip(k95info.version_date_iso)
    else if val = 'processed-date' then return strip(date())
    else if val = 'processed-time' then return strip(time())
    return '-k95info_unknown-'


outputSectionReferences: procedure expose g. refSet. settings.
    parse arg el, indentLevel, contanerElement, docType, checkReferencesOnly

    role = "text"
    if hasAttribute(el, "role") then role = getAttribute(el, "role")

    sectionChild = getFirstChild(el)
    i = 0
    refComments. = ''
    refComments.0 = 0
    refEls. = ''
    refEls.0 = 0
    do while sectionChild <> ''
        if isElementNode(sectionChild) then do
            if getName(sectionChild) = "ref" then do
                i = i + 1
                refEls.i = getAttribute(sectionChild, "src")
                refComments.i = ''
                if hasAttribute(sectionChild, "comment") then do
                    refComments.i = getAttribute(sectionChild, "comment")
                end
            end
        end
        sectionChild = getNextSibling(sectionChild)
    end
    refEls.0 = i
    refComments.0 = i

    if refEls.0 > 0 then do
        /* Sort the references */
        do i = 1 to refEls.0 - 1
            do j = i + 1 to refEls.0
                src_i = refEls.i
                parse var src_i key_i":"src_i
                sort_i = refSet.refs.key_i.sortOrder

                src_j = refEls.j
                parse var src_j key_j":"src_j
                sort_j = refSet.refs.key_j.sortOrder

                if sort_i > sort_j then do /* Swap entries */
                    temp = refEls.i
                    temp_c = refComments.i
                    refEls.i = refEls.j
                    refComments.i = refComments.j
                    refEls.j = temp
                    refComments.j = temp_c
                end
            end /* j = i + 1 to refEls.0 */
        end /* i = 1 to refEls.0 - 1 */

        /* Output the references */
        if checkReferencesOnly = 0 then do
            call outputHtml indentLevel,'<'contanerElement' class="sec_references">'
            indentLevel = indentLevel + 1
            do i = 1 to refEls.0
                call outputRef indentLevel,refEls.i,refComments.i
            end
            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</'contanerElement'>'
        end
    end
    else if role = 'ctlseq' | role = 'ctlchar' | role = 'parameter' & docType = 'outline' then do
        /* Output whenever we encounter a contol sequence/character or a
         * parameter for a control sequence/character that doesn't have
         * any references. Ideally *all* control sequences should have
         * references so we know why K95 implements it, and how it *should*
         * be implemented.
         *
         * We only output this for the outline document just so its not all
         * being output twice.
         */
        if settings.verbose = 1 then do
            title = ''
            if hasAttribute(el, "title") then title = strip(getAttribute(el, "title"))

            mnemonic = ''
            if hasAttribute(el, "mnemonic") then mnemonic = strip(getAttribute(el, "mnemonic"))

            ctlseqs = getSectionCtlSeqs(el,1)

            say "NOTE:" role "section has no references! Mnemonic: '"mnemonic"'   Title: '"title"'   Ctlseqs: '"ctlseqs"'"
        end
    end

    return



isElementFiltered: procedure expose g. badgeSet.
    parse arg el, hiddenbadges

    badges = getSectionBadges(el)

    /* No badges? No filtered */
    if badges = '' then return 0

    /* If all badges are in hiddenBadges, then return 1 */
    do i = 1 by 1 until badges = ''
        parse var badges badge" "badges
        if badge <> '' then do
            /* the element has a badge that isn't being hidden, then the
             * element shouldn't be filtered out. */
            if wordpos(badge, hiddenBadges) == 0 then return 0
        end
    end

    /* There was at least one badge, all of them were in hiddenBadges, so the
        element itself should be hidden */

    return 1


doDocumentElementHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, indentLevel, style, script
    title=settings.title

    call outputHtml indentLevel,'<html lang="en" xmlns="http://www.w3.org/1999/html" xmlns="http://www.w3.org/1999/html">'
    call outputHtml indentLevel,'<!-- ----------------------------------------------------------'
    indentLevel = indentLevel + 1
    call outputHtml indentLevel,' This document was generated by a tool on' DATE() 'at' TIME()
    call outputHtml indentLevel,' Do not edit it by hand; any changes made will be lost!'
    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'---------------------------------------------------------- -->'
    call outputHtml indentLevel,'<head>'
    indentLevel = indentLevel + 1
    call outputHtml indentLevel,'<title>'title'</title>'
    call outputHtml indentLevel,'<style>'
    indentLevel = indentLevel + 1

    badgeNames = badgeSet.badgeNames
    do I = 1 by 1 until badgeNames = ""
        parse var badgeNames badge" "badgeNames
        if badge <> '' then do
            call outputHtml indentLevel,'.'badge '{ /*' badgeSet.badges.badge.comment '*/'
            indentLevel = indentLevel + 1
            if badgeSet.badges.badge.colour <> ''
            then call outputHtml indentLevel,'background-color:' badgeSet.badges.badge.colour ';'
            else call outputHtml indentLevel,'display: none;'
            if badgeSet.badges.badge.fgColor <> '' then
                call outputHtml indentLevel,'color:' badgeSet.badges.badge.fgColor ';'
            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'}'
        end
    end
    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</style>'

    if style <> '' then do
        call outputHtml indentLevel,'<style>'
        indentLevel = indentLevel + 1
        call outputHtml indentLevel, style
        indentLevel = indentLevel - 1
        call outputHtml indentLevel,'</style>'
    end

    if script <> '' then do
        call outputHtml indentLevel,'<script>'
        indentLevel = indentLevel + 1
        call outputHtml indentLevel, script
        indentLevel = indentLevel - 1
        call outputHtml indentLevel,'</script>'
    end

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</head>'
    call outputHtml indentLevel,'<body>'

    if settings.dev_mode = 1 then call outputHtml indentLevel, settings.preliminary_banner

    call outputHtml indentLevel,'<h1>'title'</h1>'
    call outputHtml indentLevel,'<hr>'

    return indentLevel

getTextHtml: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el

    /* Valid contents:
    #PCDATA|br|a|em|badge|tt|b|u|param-char|param-single|param-multi|param-text|k95info
    * White space is preserved.
    */

    /* el is just text (#PCDATA) - nothing to do but return it*/
    if isTextNode(el) then return getText(el)
    else if isElementNode(el) then do
        name = getName(el)
        if name = 'br' then return "<br>"
        else if name = 'k95info' then return getK95InfoValue(getAttribute(el,'val'))
        else if name = 'param-char' then return parameterToString(el,1)
        else if name = 'param-single' then return parameterToString(el,1)
        else if name = 'param-multi' then return parameterToString(el,1)
        else if name = 'param-text' then return parameterToString(el,1)
        /*else if name = 'arrow' then return "&#8658;"*/
        else if name = 'badge' then do
            if hasAttribute(el, "name") then do
                badgeName = getAttribute(el, 'name')
                return '<span class="badge 'badgeName'">'badgeName'</span>'
            end
            else if hasAttribute(el, "group") then do
                groupName = getAttribute(el, 'group')
                result = ''
                doneBadges = ""

                badges = badgeSet.groups.groupName
                do I = 1 by 1 until badges = ""
                    parse var badges badgeName" "badges
                    if badgeName <> '' then do
                        /* And don't output any badges we've already done */
                        if wordpos(badgeName, doneBadges) == 0 then do
                            doneBadges = doneBadges" "badgeName
                            result = result'<span class="badge 'badgeName'">'badgeName'</span>'
                        end
                    end
                end

                return result
            end
        end
        else if name = 'b' then return doTextHtmlWithChildren(el, '<b>', '</b>')
        else if name = 'u' then return doTextHtmlWithChildren(el, '<u>', '</u>')
        else if name = 'tt' then return doTextHtmlWithChildren(el, '<tt>', '</tt>')
        else if name = 'em' then return doTextHtmlWithChildren(el, '<em>', '</em>')
        else if name = 'a' then do
            if hasAttribute(el, "href") then do
                openTag = '<a href="'getAttribute(el, "href")'">'
                return doTextHtmlWithChildren(el, openTag, '</a>')
            end
        end
    end

    return ''

doTextHtmlWithChildren: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, openTag, closeTag

    result = openTag

    child = getFirstChild(el)
    do while child <> ''
        result = result''getTextHtml(child)

        child = getNextSibling(child)
    end

    result = result''closeTag

    return result

/* Parameters:
 *   el             - the element to handle text for
 *   indentLevel    - The level of indentation to insert after any line breaks.
 *                    If the value is -1, no indentation will be inserted after
 *                    line breaks.
 *   openTag        - If supplied, the first line of output will be prefixed
 *                    with this value
 *   outputTag      - If supplied, the last line of output will be suffixed with
 *                    this value
 */
doTextHtml: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, indentLevel, openTag, closeTag

    result = openTag

    child = getFirstChild(el)
    do while child <> ''
        result = result''getTextHtml(child)

        child = getNextSibling(child)
    end

    result = result''closeTag

    /* Output the result in one go. The outputHtml function will handle
     * indenting after any linebreaks to make it tidy if necessary. */
    call outputHtml indentLevel,result

    return

doParagraphHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, indentLevel, hiddenBadges

    classes=''
    if hasAttribute(el, 'role') then do
        role = getAttribute(el, 'role')
        classes='classes="'role'"'
    end

    call outputHtml indentLevel,'<p'classes'>'
    indentLevel = indentLevel + 1

    call outputElBadges el,indentLevel, hiddenBadges
    call doTextHtml el, indentLevel, '', ''

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</p>'

    return

doListHtml: procedure expose g. toc. badgeSet. settings. refSet. k95info.
             parse arg el, indentLevel, orderded

    /* ordered = 1 ? ol : ul */

    if ordered = 1 then call outputHtml indentLevel,'<ol>'
    else call outputHtml indentLevel,'<ul>'
    indentLevel = indentLevel + 1

    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)

        /* Valid children: li */
        if name = 'li' then do
            call outputHtml indentLevel,'<li>'
            indentLevel = indentLevel + 1

            call doTextHtml child, indentLevel, '', ''

            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</li>'
        end

        child = getNextSibling(child)
    end

    indentLevel = indentLevel - 1
    if ordered = 1 then call outputHtml indentLevel,'</ol>'
    else call outputHtml indentLevel,'</ul>'

    return

doDLHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, indentLevel

    call outputHtml indentLevel,'<dl>'
    indentLevel = indentLevel + 1

    ni = 0

    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)

        /* Valid children: dt, dd */
        if name = 'dt' then do

            if hasAttribute(child, "not-implemented") then do
                if getAttribute(child, 'not-implemented') = 'true' then ni = 1
            end

            if ni = 1 then call outputHtml indentLevel,'<dt class="not-implemented">'
            else call outputHtml indentLevel,'<dt>'
            indentLevel = indentLevel + 1

            call doTextHtml child, indentLevel, '', ''

            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</dt>'
        end
        else if name = 'dd' then do
            if ni = 1 then call outputHtml indentLevel,'<dd class="not-implemented">'
            else call outputHtml indentLevel,'<dd>'
            indentLevel = indentLevel + 1

            call doTextHtml child, indentLevel, '', ''

            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</dd>'
            ni = 0
        end

        child = getNextSibling(child)
    end

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</dl>'

    return

doPreHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, indentLevel

    LF = "0A"x

    /* Pre is a block element, so it gets indented on its line */
    call doTextHtml el, -1, copies(' ', indentLevel * 4)'<pre>', '</pre>'LF

    return

doTableHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, indentLevel

    call outputHtml indentLevel,'<table>'
    indentLevel = indentLevel + 1

    tr = getFirstChild(el)
    do while tr <> ''
        name = getName(tr)

        /* Valid children: tr */
        if name = 'tr' then do

            if hasAttribute(tr, 'not-implemented') then do
                ni = getAttribute(tr, 'not-implemented')
                if ni = 'true' then call outputHtml indentLevel,'<tr class="not-implemented">'
                else call outputHtml indentLevel,'<tr>'
            end
            else call outputHtml indentLevel,'<tr>'
            indentLevel = indentLevel + 1

            trChild = getFirstChild(tr)
            do while trChild <> ''
                name = getName(trChild)

                if name = 'th' then do
                    call outputHtml indentLevel,'<th>'
                    indentLevel = indentLevel + 1

                    call doTextHtml trChild, indentLevel, '', ''

                    indentLevel = indentLevel - 1
                    call outputHtml indentLevel,'</th>'
                end
                else if name = 'td' then do
                    if hasAttribute(trChild, 'not-implemented') then do
                        ni = getAttribute(trChild, 'not-implemented')
                        if ni = 'true' then call outputHtml indentLevel,'<td class="not-implemented">'
                        else call outputHtml indentLevel,'<td>'
                    end
                    else call outputHtml indentLevel,'<td>'
                    indentLevel = indentLevel + 1

                    call doTextHtml trChild, indentLevel, '', ''

                    indentLevel = indentLevel - 1
                    call outputHtml indentLevel,'</td>'
                end

                trChild = getNextSibling(trChild)
            end

            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</tr>'
        end

        tr = getNextSibling(tr)
    end

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</table>'

    return


ctlsecTableRowHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info. column_totals.
    parse arg el, sectionLevel, indentLevel, linkFile, hiddenBadges, docType, allowedBadges

    is_implemented = 1
    if hasAttribute(el, "not-implemented") then is_implemented = 0

    if is_implemented = 0 & docType <> 'term-ctlseqs' & docType <> 'todo' then return

    todo = 0
    if hasAttribute(el, "todo") then todo = 1

    title = ''
    if hasAttribute(el, "title") then title = strip(getAttribute(el, "title"))

    mnemonic = ''
    if hasAttribute(el, "mnemonic") then mnemonic = strip(getAttribute(el, "mnemonic"))

    id = ''
    if hasAttribute(el, "id") then id = strip(getAttribute(el, "id"))

    role = ''
    if hasAttribute(el, "role") then role = strip(getAttribute(el, "role"))

    ctlseqs = getSectionCtlSeqs(el,1)

    exclusions = ''
    if hasAttribute(el, 'exclude-badges') then do
        exclusions = getAttribute(el, 'exclude-badges')
    end

    if docType = 'term-ctlseqs' | docType = 'todo' then do
        /* Output a generic title if none is present for a parameter */
        if title = '' & role = 'parameter' then title = 'Parameter'

        badges = getSectionSupportedTerminals(el)

        /* Check to see that at least one of the badges is in the list of
           allowed badges */

        found = 0
        tt = badges
        do I = 1 by 1 until tt = ""
            parse var tt badge' 'tt
            if badge <> '' & wordpos(badge, allowedBadges) <> 0 then do
                found = 1
                leave
            end
        end

        /* If none of the sections supported terminals are in the list of
           terminals being included in the table, then don't output it */
        if found = 0 then return

        sectionBadges = getSectionBadges(el)
        /* TODO: If there are no specific badges applied, then perhaps we
                    should apply *all* badges (as thats what no badges
                    implicitly means in the absence of any exclusions) */
    end
    else do
        badges = getSectionBadges(el)
        /* TODO: If there are no specific badges applied, then perhaps we
                    should apply *all* badges (as thats what no badges
                    implicitly means in the absence of any exclusions) */
    end

    if docType = 'todo' then do
        if todoOutputFilter(el, badges, sectionBadges, mnemonic, ctlseqs) = 0 then return
    end

    if is_implemented = 1 & docType = 'todo' then do
        if sectionHasUnimplementedParameterChildren(el, allowedBadges) = 0 then return
    end

    row_classes = ''
    if is_implemented = 0 then row_classes = 'not-implemented'
    if role = 'parameter' then row_classes = row_classes' parameter'
    row_classes = strip(row_classes)

    call outputHtml indentLevel,'<tr id="'id'" class="'row_classes'">'
    indentLevel = indentLevel + 1

    call outputHtml indentlevel,'<th class="ctlseq_title 'row_classes'">'title'</th>'
    call outputHtml indentLevel,'<th class="mnemonic">'mnemonic'</th>'
    call outputHtml indentLevel,'<th class="ctlseq">'
    indentLevel = indentLevel + 1
    if id <> '' & todo = 0 then do
        call outputHtml indentLevel,'<a href="'linkFile'#'id'">'
        indentLevel = indentLevel + 1
    end
    call outputHtml indentLevel,ctlseqs
    if id <> '' & todo = 0 then do
        indentLevel = indentLevel - 1
        call outputHtml indentLevel,'</a>'
    end
    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</th>'

    /* TODO: Ensure exclusions are being handled correctly */

    if docType = 'term-ctlseqs' | docType = 'todo' then badgeNames = allowedBadges
    else badgeNames = badgeSet.badgeNames
    do I = 1 by 1 until badgeNames = ""
        parse var badgeNames badge" "badgeNames
        if badge <> '' then do
            if wordpos(badge, allowedBadges) <> 0 then do
                if wordpos(badge, hiddenBadges) == 0 then do
                    td_classes = badge
                    if docType = 'term-ctlseqs' | docType = 'todo' then do
                        td_classes = td_classes' show-badge'
                    end

                    td = '<td class="'td_classes'">'

                    if docType = 'term-ctlseqs' | docType = 'todo' then do
                        if wordpos(badge, badges) <> 0 then do
                            if sectionBadges <> '' & wordpos(badge, sectionBadges) = 0 then td = td'O'
                            else td = td'X'

                            totl = column_totals.badge.total
                            impl = column_totals.badge.implemented

                            column_totals.badge.total = totl + 1
                            if is_implemented = 1 then do
                                column_totals.badge.implemented = impl + 1
                            end
                        end
                    end
                    else do
                        if wordpos(badge,badges) <> 0 | badges = '' then do
                            if wordpos(badge,exclusions) = 0 then do
                                td=td'X'
                            end
                        end
                    end
                    td = td'</td>'
                    call outputHtml indentLevel,td
                end
            end
        end
    end

    /* References column */
    if docType = 'todo' then call outputSectionReferences el, indentLevel, 'td', docType, 0

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</tr>'

    /* Any child parameter sections as additional sub-rows */
    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)

        if name = 'section' & hasAttribute(child, 'role') then do
            role = getAttribute(child, 'role')
            if role = 'parameter' then do
                call ctlsecTableRowHTML child, sectionLevel, indentLevel, linkFile, hiddenBadges, docType, allowedBadges
            end
            else if role = 'group' then do
                group_child = getFirstChild(child)
                do while group_child <> ''
                    name = getName(group_child)

                    if name = 'section' & hasAttribute(group_child, 'role') then do
                        role = getAttribute(group_child, 'role')
                        if role = 'parameter' then do
                            call ctlsecTableRowHTML group_child, sectionLevel, indentLevel, linkFile, hiddenBadges, docType, allowedBadges
                        end
                    end

                    group_child = getNextSibling(group_child)
                end
            end
        end

        child = getNextSibling(child)
    end

    return

ctlseqTableSectionHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, sectionLevel, indentLevel, linkFile, hiddenBadges, docType

    call outputHtml indentLevel,'<table class="ctlseq-table">'
    indentLevel = indentLevel + 1

    /* Output the heading row of terminal types */
    call outputHtml indentLevel,'<thead>'
    indentLevel = indentLevel + 1

    call outputHtml indentLevel,'<tr>'
    indentLevel = indentLevel + 1

    call outputHtml indentlevel,'<th>Title</th>'
    call outputHtml indentLevel,'<th>Mnemonic</th>'
    call outputHtml indentLevel,'<th>Control Sequence</th>'

    if docType = 'term-ctlseqs' then badgeNames = settings.compare_terminals
    else if docType = 'todo' then badgeNames = settings.todo_terminals
    else badgeNames = badgeSet.badgeNames

    allowedBadges = badgeNames

    column_totals. = ''

    do I = 1 by 1 until badgeNames = ""
        parse var badgeNames badge" "badgeNames
        if badge <> '' then do
            if wordpos(badge, hiddenBadges) == 0 then do
                if docType = 'term-ctlseqs' | docType = 'todo' then do
                    call outputHtml indentLevel,'<th class="'badge' show-badge">'badge'</th>'
                end
                else call outputHtml indentLevel,'<th class="'badge'">'badge'</th>'
                column_totals.badge.total = 0
                column_totals.badge.implemented = 0
            end
        end
    end

    /* References column */
    if docType = 'todo' then call outputHtml indentlevel,'<th>Documentation References</th>'

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</tr>'

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</thead>'

    call outputHtml indentLevel,'<tbody>'
    indentLevel = indentLevel + 1

    /* Output one child row per ctlseq */
    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)

        if name = 'section' & hasAttribute(child, 'role') then do
            role = getAttribute(child, 'role')
            if role = 'ctlseq' | role = 'ctlchar' then do
                call ctlsecTableRowHTML child, sectionLevel, indentLevel, linkFile, hiddenBadges, docType, allowedBadges
            end
        end

        child = getNextSibling(child)
    end

    /* totals at the bottom of each column for term-ctlseqs output */
    if docType = 'term-ctlseqs' then do
        call outputHtml indentLevel,'<th colspan=3 class="totals">Totals:</th>'
        badgeNames = allowedBadges
        do I = 1 by 1 until badgeNames = ""
            parse var badgeNames badge" "badgeNames
            if badge <> '' then do
                if wordpos(badge, hiddenBadges) == 0 then do
                    impl = column_totals.badge.implemented
                    totl = column_totals.badge.total
                    perct = 0
                    if totl > 0 then perct = (impl/totl)*100
                    perct = strip(format(perct, 3, 1))
                    val = impl'/'totl'<br><span class="percentage">('perct'%)</span>'
                    call outputHtml indentLevel,'<td class="'badge' show-badge">'val'</td>'
                end
            end
        end
    end

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</tbody>'

    indentLevel = indentLevel - 1
    call outputHtml indentLevel,'</table>'

    return

sectionHasCtlseqChildren: procedure expose g.
    parse arg el

    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)

        if name = 'section' & hasAttribute(child, 'role') then do
            role = getAttribute(child, 'role')
            if role = 'ctlseq' | role = 'ctlchar' then do
                return 1
            end
        end

        child = getNextSibling(child)
    end

    return 0


/* For the TO-DO output, its not good enough to just check if a particular
 * control sequence is unimplemented and passes the selection criteria, we
 * also need to check against all of its parameter children too. If one or
 * more parameter children are eligible for inclusion in the document, then
 * the parent must also be included even if it is marked as implemented.
 */
sectionHasUnimplementedParameterChildren: procedure expose g.
    parse arg el, allowedBadges

    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)

        if name = 'section' & hasAttribute(child, 'role') then do
            role = getAttribute(child, 'role')

            is_implemented = 1
            if hasAttribute(child, "not-implemented") then is_implemented = 0

            if role = 'parameter' & is_implemented = 0 then do
                badges = getSectionSupportedTerminals(child)

                /* Check to see that at least one of the badges is in the list of
                   allowed badges */

                found = 0
                tt = badges
                do I = 1 by 1 until tt = ""
                    parse var tt badge' 'tt
                    if badge <> '' & wordpos(badge, allowedBadges) <> 0 then do
                        found = 1
                        leave
                    end
                end

                if found = 1 then do
                    sectionBadges = getSectionBadges(child)
                    mnemonic = ''
                    if hasAttribute(el, "mnemonic") then mnemonic = strip(getAttribute(child, "mnemonic"))
                    ctlseqs = getSectionCtlSeqs(child,1)
                    if todoOutputFilter(child, badges, sectionBadges, mnemonic, ctlseqs) = 1 then do
                        return 1
                    end
                end
            end
        end

        child = getNextSibling(child)
    end

    return 0

/* Outputs a section - this is kind of equivalent to a <div> in HTML. Its a
 * block element which contains other block elements. In particular, it can
 * contain (that we render): section, p, ul, ol, dl, pre, and table
 * It also has two elements that are used to configure the display of the
 * section which are not themselves rendered: ref and ctlseq.
 *
 * Attributes are:
 *   id             - used as an id in the generated HTML
 *   title          - used for the sections heading
 *   ctlseq         - specifies a control sequence associated with the section.
 *                    Multiple control sequences, or control sequences requiring
 *                    limited formatting (eg, bold) can be specified using the
 *                    <ctlseq> element.
 *   mnemonic       - The mnemonic for the control sequence if there is one.
 *   toc            - If this section should appear in the Table of Contents
 *   groups         - Badge groups that apply to this section
 *   badges         - Individual badges that apply to this section
 *   exclude-badges - Badges that specifically do not apply to this section,
 *                    even if those badges appear in a referenced badge group.
 *   style          - What "style" of section this is. Options are:
 *                      section - a normal section
 *                      group   - a section with no top and bottom margin. Used
 *                                for grouping multiple headings together.
 *                      box     - Rendered as a box. Deprecated.
 *   not-implemented -The details discussed in the section have not been
 *                    implemented in K95 yet. Valid options are true or false.
 *                    If not specified, false is assumed.
 *   role           - The sections "role" in the document. Options are:
 *                      text      - General text content
 *                      ctlseq    - Section describing a control seqeunce
 *                      ctlchar   - Section describing a control character
 *                      parameter - Section describing a parameter to a control
 *                                  sequence.
 *                      group     - A group of other sections with no content of
 *                                  its own.
 *   formats        - Which document formats and types the section should be
 *                    included in. Options are currently:
 *                      html:document  - The main document.
 *                      html:outline   - The outline document
 *                    If no format is specfied, html:document is assumed. The
 *                    outline document includes the headings for all sections
 *                    regardless of their format setting - a format of
 *                    html:document only prevents the text content from being
 *                    output.
 *
 * Parameters are:
 *   el            - The section element to output
 *   sectionLevel  - The heading level of the parent section
 *   indentLevel   - The indent level of the parent section
 *   docType       - Type of document that is being generated:
 *                   'document': Output the full document excluding any sections
 *                               marked for other document types
 *                   'outline': Only sections with their format set to
 *                              "html:outline" will be output in full,
 *                              everything else will be collapsed to headings
 *                              only, and those headings will be links to the
 *                              regular document.
 *                   'ctlseq-table': Only sections with their format set to
 *                                   "html:ctlseq-table" will be output in full.
 *                                   Sections containing ctlseq subsections will
 *                                   be converted to a table.
 *   linkFile      - Name of the regular document the outline document should
 *                   link to.
 *   parentFormats - The format attribute of the parent section (or its parent).
 *                   This is used if no format attribute is specified on this
 *                   section.
 */
doSectionHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, sectionLevel, indentLevel, docType, linkFile, parentFormats, requiredFormat, hiddenBadges

    output_content = 1
    checkReferencesOnly = 0
    output_badges = 1
    ctlseq_table_output = 0

    /* If isOutline = 0 then we're outputting the full document.
       If isOutline = 1 then we're outputting some subset of the document
            (eg, the outline document or the ctlseq-table)
     */
    isOutline = 0
    if docType = 'outline' then do
        isOutline = 1
        checkReferencesOnly = 1
        output_badges = 0
    end

    sectionLevel = sectionLevel + 1
    indentLevel = indentLevel + 1

    required_format = "html:document"
    if requiredFormat <> "" then required_format = requiredFormat
    if docType = 'outline' then required_format = "html:outline"
    else if docType = 'ctlseq-table' then required_format = "html:ctlseq-table"
    else if docType = 'term-ctlseqs' then required_format = "html:term-ctlseqs"
    else if docType = 'todo' then required_format = 'html:todo'

    if docType = 'ctlseq-table' | docType = 'term-ctlseqs' | docType = 'todo' then do
        ctlseq_table_layout = 1
    end

    /* Section can have:
        id | title | ctlseq | toc | groups | badges | exclude-badges |
        style | not-implemented | role | formats | mnemonic */

    role = "text"
    if hasAttribute(el, "role") then role = getAttribute(el, "role")

    formats = parentFormats
    if formats = '' then formats = "html:document"
    if hasAttribute(el, "formats") then formats = getAttribute(el, "formats")

    /* for the outline format, we'll only output the content of sections (that
     * is their paragraphs, tables, etc) html:outline is explicitly mentioned
     * in the formats attribute. For all other sections we'll only output
     * headings. */
    if wordpos(required_format,formats) == 0 then output_content = 0

    /* For the ctlseq-table and term-ctlseqs, we're really only interested in
     * sections with the ctlseq or ctlchar role, and their parent sections (for
     * their headings). So any section that isn't explicitly marked for the
     * html:ctlseq-table or html:term-ctlseqs format and has no ctlseq sections
     * in it can be ignored entirely */
    if ctlseq_table_layout = 1 & output_content = 0 then do
        if sectionHasCtlseqChildren(el) = 0 then return
    end
    else if docType = 'document' & output_content = 0 then return

    style = "section"
    if hasAttribute(el, "style") then style = getAttribute(el, "style")

    implemented = 1
    if hasAttribute(el, "not-implemented") then implemented = 0

    todo = 0
    if hasAttribute(el, "todo") & getAttribute(el, "todo") = "true" then todo = 1

    if todo = 1 then do
        /* TODO sections are not to be rendered currently */
        return
    end

    title = ''
    if hasAttribute(el, "title") then title = strip(getAttribute(el, "title"))

    mnemonic = ''
    if hasAttribute(el, "mnemonic") then mnemonic = strip(getAttribute(el, "mnemonic"))

    id = ''
    if hasAttribute(el, "id") then id = strip(getAttribute(el, "id"))


    ctlseqs = getSectionCtlSeqs(el,1)

    if title == '' & ctlseqs == '' & mnemonic == '' & id='' then do
        if isOutline = 1 & role <> 'group' & output_content=0  then do
            /* In outline mode we're mostly interested in outputting section titles,
             * not their content. So any sections that don't have any title
             * components can likely be skipped */
            return
        end

        /* No title? Don't increment the section level */
        sectionLevel = sectionLevel - 1
    end

    if ctlseq_table_layout = 1 & (role = 'ctlseq' | role = 'ctlchar') then do
        /* For the table-layout doc types these are output as
           table rows rather than sections */
        return
    end

    /*say "Mnemonic: '"mnemonic"'   Title: '"title"'   Ctlseqs: '"ctlseqs"'"*/
    if id = '' & ctlseqs <> '' & implemented = 1 & (title <> '' |  | mnemonic <> '') then do
        if settings.verbose = 1 then do
            say "WARN: Titled Section Missing ID - Mnemonic: '"mnemonic"'   Title: '"title"'   Ctlseqs: '"ctlseqs"'"
        end
    end

    exclusions = ''
    if hasAttribute(el, 'exclude-badges') then do
        exclusions = getAttribute(el, 'exclude-badges')
    end
    badges = getSectionBadges(el)
    /* TODO: If there are no specific badges applied, then perhaps we
            should apply *all* badges (as thats what no badges
            implicitly means in the absence of any exclusions) */

    if docType <> 'term-ctlseqs' & docType <> 'todo' then do
        hb = getHiddenSectionBadges(el)
        if hb <> '' then hiddenBadges = hiddenBadges' 'hb

        if isElementFiltered(el, hiddenBadges) then return
    end

    badge_classes = ''

    do i = 1 by 1 until badges = ""
        parse var badges badge" "badges
        if badge <> '' then do
            if wordpos(badge, exclusions) == 0 then do
                badge_classes = badge_classes' for-'badge
            end
        end
    end
    do i = 1 by 1 until exclusions = ""
        parse var exclusions excl_badge" "exclusions
        if excl_badge <> '' then badge_classes = badge_classes' no-'excl_badge
    end

    /* We have all the section details we care about. Now output. */

    /* TODO: should we output box sections at all for outline? */
    if style = 'box' then do /* A box section */
        if title = '' then title = "Terminal Type Specific Behaviour"

        call outputHtml indentLevel,'<div class="section box">' /* OPEN SECTION */
        indentLevel = indentLevel + 1
        call outputHtml indentLevel,'<div class="title">'title'</div>'
        call outputHtml indentLevel,'<div class="badges">'
        call outputElBadges el,indentLevel + 1, hiddenBadges
        call outputHtml indentLevel,'</div>'
    end
    else do  /* a regular section */

        /* Open the section */
        classes = 'section'
        if style = 'group' then do
            classes = 'sec_group'
        end
        if badge_classes <> '' then classes = classes badge_classes


        call outputHtml indentLevel,'<div class="'classes'">' /* OPEN SECTION */
        indentLevel = indentLevel + 1

        /* ------------ Title Block ------------ */
        if ctlseqs <> "" | mnemonic <> "" | title <> "" then do
            h = '<h'sectionLevel
            if id <> "" then h = h' id="'id'"'
            if implemented = 0 then h = h' class="not-implemented"'
            h = h'>'
            call outputHtml indentLevel,h   /* OPEN <h> */
            indentLevel = indentLevel + 1
            call outputHtml indentLevel,'<span class="sec-title-main">'
            indentLevel = indentLevel + 1
        end

        if docType = 'outline' & id <> '' & implemented = 1 then do
            call outputHtml indentLevel,'<a href="'linkFile'#'id'">'
            indentLevel = indentLevel + 1
        end

        /* Control sequence if we have one */
        if ctlseqs <> "" then call outputHtml indentLevel,'<span class="sec_ctlseq">'ctlseqs'</span>'

        /* For h3 or above, if there is a ctlseq and a title then output
           as: ctlseq => title */
        if (sectionLevel > 3) & (ctlseqs <> "") & (title <> "" | mnemonic <> "") then do
            call outputHtml indentLevel,'<span class="sec_arrow">&#8658;</span>'
        end

        /* Mnemonic if we have one */
        if mnemonic <> "" then do
            dash = '&mdash;'
            if title = "" then dash = ''
            call outputHtml indentLevel,'<span class="sec_mnemonic">'mnemonic'</span> 'dash
        end

        /* Title if we have one */
        if title <> "" then do
            call outputHtml indentLevel,'<span class="sec_title">'title'</span>'
        end

        if docType = 'outline' & id <> '' & implemented = 1 then do
            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</a>'
        end

        if ctlseqs <> "" | mnemonic <> "" | title <> "" then do
            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</span>'
        end

        /* References if we have any */
        call outputSectionReferences el, indentLevel, 'span', docType, checkReferencesOnly

        /* For h4 and above, or outline mode, output the section badges in
         * the title */
        if output_badges = 1 then do
            if (sectionLevel > 3) | docType = 'outline' then do
                call outputElBadges el,indentLevel,hiddenBadges
            end
        end

        if ctlseqs <> "" | mnemonic <> "" | title <> "" then do
            indentLevel = indentLevel - 1
            call outputHtml indentLevel,'</h'sectionLevel'>' /* CLOSE <h> */
        end

        /* ------------ END Title Block ------------ */

        if output_badges = 1 then do
            /* For h3 and below, output the section badges under the title */
            if (sectionLevel < 4) & docType <> 'outline' then do
                call outputElBadges el,indentLevel + 1, hiddenBadges
            end
        end
    end

    if implemented = 0 then do
        call outputHtml indentLevel,'<div class="not-implemented">'
        indentLevel = indentLevel + 1
    end

    call outputHtml indentLevel,'<div class="content">'

    /* Now process children. Section can contain:
        ctlseq|ref|p|ul|ol|dl|section|pre|table */

    has_ctlseq_children = 0

    child = getFirstChild(el)
    do while child <> ''
        name = getName(child)

        /* TODO: Can we output the content tags only if we're going to output
            something here? */

        if name = 'section' then do
            if has_ctlseq_children = 0 & hasAttribute(child, 'role') then do
                role = getAttribute(child, 'role')
                if role = 'ctlseq' | role = 'ctlchar' then do
                    has_ctlseq_children =  1
                end
            end
            call doSectionHTML child, sectionLevel, indentLevel, docType, linkFile, formats, required_format, hiddenBadges
        end
        else if name = 'p' & output_content = 1 & isElementFiltered(child, hiddenBadges) = 0 then do
            call doParagraphHTML child, indentLevel, hiddenBadges
        end
        else if name = 'ul' & output_content = 1 then call doListHtml child, indentLevel, 0
        else if name = 'ol' & output_content = 1 then call doListHtml child, indentLevel, 1
        else if name = 'dl' & output_content = 1 then call doDLHTML child, indentLevel
        else if name = 'pre' & output_content = 1 then call doPreHTML child, indentLevel
        else if name = 'table' & output_content = 1 then call doTableHTML child, indentLevel
        /* The rest we ignore: ref, ctlseq */

        child = getNextSibling(child)
    end

    /* Lastly, produce a table of control sequences for any in this section */
    if ctlseq_table_layout = 1 & has_ctlseq_children = 1 then do
        call ctlseqTableSectionHTML el, sectionLevel, indentLevel, linkFile, hiddenBadges, docType
    end

    call outputHtml indentLevel,'</div> <!-- /content -->'
    indentLevel = indentLevel - 1

    if implemented = 0 then do
        indentLevel = indentLevel - 1
        call outputHtml indentLevel,'</div> <!-- /not-implemented -->'
    end

    /* And then close out the section */
    if style = 'box' then do /* A box section */
        call outputHtml indentLevel,'</div> <!-- /section box -->' /* CLOSE SECTION */
    end
    else do  /* a regular section */
        call outputHtml indentLevel,'</div> <!-- /section -->' /* CLOSE SECTION */
    end

    return

doHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, sectionLevel, indentLevel, style, script, docFile, docType

    name = getName(el)

    if name = 'document' then do
        indentLevel = doDocumentElementHTML(el, indentLevel, style, script)

        /* Figure out the non-outline document name */
        linkFile = ''
        if docType <> 'document' then do
            docChild = getFirstChild(el)
            do while docChild <> '' & linkFile = ''
                if \ isElementNode(docChild) then do
                    docChild = getNextSibling(docChild)
                    iterate
                end
                name = getName(docChild)
                if name = 'outputs' then do
                    outChild = getFirstChild(docChild)
                    do while outChild <> '' & linkFile = ''
                        if getName(outChild) = 'html' then do
                            /* Produce an HTML output */

                            file = getAttribute(outChild, "filename")

                            type = getAttribute(outChild, "type")
                            if type = 'document' then do
                                linkFile = file
                                say "Found link file:" linkFile
                            end
                        end
                        outChild = getNextSibling(outChild)
                    end
                end
                docChild = getNextSibling(docChild)
            end
        end

        /* Handle children */
        child = getFirstChild(el)
        do while child <> ''
            name = getName(child)

            /* document can contain:
                references | badges | badgegroups | outputs | section | index */

            /* the only child we render for the outline document is section */
            if name = 'section' then call doSectionHTML child, 1, indentLevel, docType, linkFile, ''
            else if name = 'index' & docType = 'document' then do
                call outputHtml indentLevel,'<h'sectionLevel'>'Contents'</h'sectionLevel'>'
                call outputHtml indentLevel,'<ul>'
                indentLevel = indentLevel + 1
                I = 1
                do while I < toc.0 + 1
                  call outputHtml indentLevel,'<li><a href="#'toc.I.id'">'toc.I.title'</a></li>'
                  I = I + 1
                end
                indentLevel = indentLevel - 1
                call outputHtml indentLevel,'</ul>'
            end
            /* The rest we ignore: references, badges, badgegroups, outputs, index */

            child = getNextSibling(child)
        end
    end

    call outputHtml indentLevel,'</body>'
    call outputHtml indentLevel,'</html>'
    return


doCtlseqTableHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, sectionLevel, indentLevel, style, script, docFile

    call doHTML el, sectionLevel, indentLevel, style, script, docFile, 'ctlseq-table'
    return

doOutlineHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, sectionLevel, indentLevel, style, script, docFile

    call doHTML el, sectionLevel, indentLevel, style, script, docFile, 'outline'
    return

doDocumentHTML: procedure expose g. toc. badgeSet. settings. refSet. k95info.
    parse arg el, sectionLevel, indentLevel, style, script, docFile

    call doHTML el, sectionLevel, indentLevel, style, script, docFile, 'document'
    return

apLine: procedure
    parse arg existing, new
    LF = "0A"x
    return existing""new""LF

writeHtml: procedure expose settings.
    parse arg html
    call charout settings.output_file,html
    return


outputHtml: procedure expose settings.
    parse arg indentLevel,html

    /* Indent level less than 0 means don't tidy or otherwise with white space,
       just output verbatim. */
    if indentLevel < 0 then do
        call charout settings.output_file, html
        return
    end

    /* Tidy up the input html - strip any leading and trailing blanks from
       each line, then indent it properly */

    /* Convert CRLF to LF */
    LF = "0A"x
    lines = changestr("0D 0A"x,html,LF)
    lines = changestr("0D"x,lines,LF)

    do I = 1 by 1 until lines = ""
        parse var lines line"0A"x lines

        line = strip(line)

        if length(line) > 0 then do
            line = copies(' ', indentLevel * 4)line
            if settings.output_file = ''
            then say line
            else call lineout settings.output_file, line
        end
    end
    return


getK95Info: procedure expose k95info. settings.

    say "Getting K95 Info..."

    k95info.herald = CKermit("return \v(herald)")

    hh = k95info.herald

    parse var hh "Kermit 95 "full_version", "ver_month_short" "ver_day" "ver_year","hh
    parse var full_version k95info.version_number" "k95info.version_release_actual


    if settings.dev_mode = 0 then do
        k95info.version_release = k95info.version_release_actual
    end
    else
    do
        k95info.version_release = 'DEV'
    end

    if k95info.version_release = '' then k95info.version_full = k95info.version_number
    else k95info.version_full = k95info.version_number k95info.version_release

    ver_month_num = mshort_to_num(ver_month_short)
    ver_month = mnum_to_word(ver_month_num)

    k95info.version_short_date = ver_day ver_month_short ver_year
    k95info.version_date = ver_day ver_month ver_year
    if ver_day < 10 then
        k95info.version_date_iso = ver_year"-"ver_month_num"-0"ver_day
    else
        k95info.version_date_iso = ver_year"-"ver_month_num"-"ver_day

    vnum = k95info.version_number
    parse var vnum k95info.version_major"."k95info.version_minor"."k95info.version_rev

    k95info.version = k95info.version_major"."k95info.version_minor" "k95info.version_rel

    /*say "Herald:" k95info.herald
    say "Version Number:" k95info.version_number
    say "Verison Release Actual:" k95info.version_release_actual
    say "Version Release:" k95info.version_release
    say "Version Full:" k95info.version_full
    say "Version Short Date:" k95info.version_short_date
    say "Version Date:" k95info.version_date
    say "Version Date ISO:" k95info.version_date_iso
    say "Version Major:" k95info.version_major
    say "Version Minor:" k95info.version_minor
    say "Version Revision:" k95info.version_rev*/

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

exit

/*REXX 2.0.0
Copyright (c) 2009-2020, Andrew J. Armstrong
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/**********************************************************************
**                                                                   **
** ALL CODE BELOW THIS POINT BELONGS TO THE XML PARSER. YOU MUST     **
** APPEND IT TO ANY REXX SOURCE FILE THAT REQUIRES AN XML PARSING    **
** CAPABILITY. SINCE REXX HAS NO 'LIBRARY' FUNCTIONALITY, A WAY TO   **
** AVOID HAVING DIFFERENT VERSIONS OF THE PARSER IN EACH OF YOUR     **
** REXX PROCS IS TO DYNAMICALLY APPEND A CENTRAL VERSION TO EACH OF  **
** YOUR REXX PROCS BEFORE EXECUTION.                                 **
**                                                                   **
** THE EXACT PROCEDURE TO FOLLOW DEPENDS ON YOUR PLATFORM, BUT...    **
** TO HELP YOU DO THIS, I HAVE INCLUDED A REXX PRE-PROCESSOR CALLED  **
** REXXPP THAT CAN BE USED TO SEARCH FOR 'INCLUDE' DIRECTIVES AND    **
** REPLACE THEM WITH THE SPECIFIED FILE CONTENTS. IT HAS BEEN TESTED **
** ON TSO, AND ON WIN32 USING REGINA REXX VERSION 3.3.               **
**                                                                   **
**********************************************************************/

/*REXX*****************************************************************
**                                                                   **
** NAME     - PARSEXML                                               **
**                                                                   **
** FUNCTION - A Rexx XML parser. It is non-validating, so DTDs and   **
**            XML schemas are ignored. Ok, DTD entities are processed**
**            but that's all.                                        **
**                                                                   **
** USAGE    - 1. Initialize the parser by:                           **
**                                                                   **
**               call initParser [options...]                        **
**                                                                   **
**            2. Parse the XML file to build an in-memory model      **
**                                                                   **
**               returncode = parseFile('filename')                  **
**                ...or...                                           **
**               returncode = parseString('xml in a string')         **
**                                                                   **
**            3. Navigate the in-memory model with the DOM API. For  **
**               example:                                            **
**                                                                   **
**               say 'The document element is called',               **
**                                   getName(getDocumentElement())   **
**               say 'Children of the document element are:'         **
**               node = getFirstChild(getDocumentElement())          **
**               do while node <> ''                                 **
**                 if isElementNode(node)                            **
**                 then say 'Element node:' getName(node)            **
**                 else say '   Text node:' getText(node)            **
**                 node = getNextSibling(node)                       **
**               end                                                 **
**                                                                   **
**            4. Optionally, destroy the in-memory model:            **
**                                                                   **
**               call destroyParser                                  **
**                                                                   **
** INPUT    - An XML file containing:                                **
**              1. An optional XML prolog:                           **
**                 - 0 or 1 XML declaration:                         **
**                     <?xml version="1.0" encoding="..." ...?>      **
**                 - 0 or more comments, PIs, and whitespace:        **
**                     <!-- a comment -->                            **
**                     <?target string?>                             **
**                 - 0 or 1 document type declaration. Formats:      **
**                     <!DOCTYPE root SYSTEM "sysid">                **
**                     <!DOCTYPE root PUBLIC "pubid" SYSTEM "sysid"> **
**                     <!DOCTYPE root [internal dtd]>                **
**              2. An XML body:                                      **
**                 - 1 Document element containing 0 or more child   **
**                     elements. For example:                        **
**                     <doc attr1="value1" attr2="value2"...>        **
**                       Text of doc element                         **
**                       <child1 attr1="value1">                     **
**                         Text of child1 element                    **
**                       </child1>                                   **
**                       More text of doc element                    **
**                       <!-- an empty child element follows -->     **
**                       <child2/>                                   **
**                       Even more text of doc element               **
**                     </doc>                                        **
**                 - Elements may contain:                           **
**                   Unparsed character data:                        **
**                     <![CDATA[...unparsed data...]]>               **
**                   Entity references:                              **
**                     &name;                                        **
**                   Character references:                           **
**                     &#nnnnn;                                      **
**                     &#xXXXX;                                      **
**              3. An XML epilog (which is ignored):                 **
**                 - 0 or more comments, PIs, and whitespace.        **
**                                                                   **
** API      - The basic setup/teardown API calls are:                **
**                                                                   **
**            initParser [options]                                   **
**                Initialises the parser's global variables and      **
**                remembers any runtime options you specify. The     **
**                options recognized are:                            **
**                NOBLANKS - Suppress whitespace-only nodes          **
**                DEBUG    - Display some debugging info             **
**                DUMP     - Display the parse tree                  **
**                                                                   **
**            parseFile(filename)                                    **
**                Parses the XML data in the specified filename and  **
**                builds an in-memory model that can be accessed via **
**                the DOM API (see below).                           **
**                                                                   **
**            parseString(text)                                      **
**                Parses the XML data in the specified string.       **
**                                                                   **
**            destroyParser                                          **
**                Destroys the in-memory model and miscellaneous     **
**                global variables.                                  **
**                                                                   **
**          - In addition, the following utility API calls can be    **
**            used:                                                  **
**                                                                   **
**            removeWhitespace(text)                                 **
**                Returns the supplied text string but with all      **
**                whitespace characters removed, multiple spaces     **
**                replaced with single spaces, and leading and       **
**                trailing spaces removed.                           **
**                                                                   **
**            removeQuotes(text)                                     **
**                Returns the supplied text string but with any      **
**                enclosing apostrophes or double-quotes removed.    **
**                                                                   **
**            escapeText(text)                                       **
**                Returns the supplied text string but with special  **
**                characters encoded (for example, '<' becomes &lt;) **
**                                                                   **
**            toString(node)                                         **
**                Walks the document tree (beginning at the specified**
**                node) and returns a string in XML format.          **
**                                                                   **
** DOM API  - The DOM (ok, DOM-like) calls that you can use are      **
**            listed below:                                          **
**                                                                   **
**            Document query/navigation API calls                    **
**            -----------------------------------                    **
**                                                                   **
**            getRoot()                                              **
**                Returns the node number of the root node. This     **
**                can be used in calls requiring a 'node' argument.  **
**                In this implementation, getDocumentElement() and   **
**                getRoot() are (incorrectly) synonymous - this may  **
**                change, so you should use getDocumentElement()     **
**                in preference to getRoot().                        **
**                                                                   **
**            getDocumentElement()                                   **
**                Returns the node number of the document element.   **
**                The document element is the topmost element node.  **
**                You should use this in preference to getRoot()     **
**                (see above).                                       **
**                                                                   **
**            getName(node)                                          **
**                Returns the name of the specified node.            **
**                                                                   **
**            getNodeValue(node)                                     **
**            getText(node)                                          **
**                Returns the text content of an unnamed node. A     **
**                node without a name can only contain text. It      **
**                cannot have attributes or children.                **
**                                                                   **
**            getAttributeCount(node)                                **
**                Returns the number of attributes present on the    **
**                specified node.                                    **
**                                                                   **
**            getAttributeMap(node)                                  **
**                Builds a map of the attributes of the specified    **
**                node. The map can be accessed via the following    **
**                variables:                                         **
**                  g.0ATTRIBUTE.0 = The number of attributes mapped.**
**                  g.0ATTRIBUTE.n = The name of attribute 'n' (in   **
**                                   order of appearance). n > 0.    **
**                  g.0ATTRIBUTE.name = The value of the attribute   **
**                                   called 'name'.                  **
**                                                                   **
**            getAttributeName(node,n)                               **
**                Returns the name of the nth attribute of the       **
**                specified node (1 is first, 2 is second, etc).     **
**                                                                   **
**            getAttributeNames(node)                                **
**                Returns a space-delimited list of the names of the **
**                attributes of the specified node.                  **
**                                                                   **
**            getAttribute(node,name)                                **
**                Returns the value of the attribute called 'name' of**
**                the specified node.                                **
**                                                                   **
**            getAttribute(node,n)                                   **
**                Returns the value of the nth attribute of the      **
**                specified node (1 is first, 2 is second, etc).     **
**                                                                   **
**            setAttribute(node,name,value)                          **
**                Updates the value of the attribute called 'name'   **
**                of the specified node. If no attribute exists with **
**                that name, then one is created.                    **
**                                                                   **
**            setAttributes(node,name1,value1,name2,value2,...)      **
**                Updates the attributes of the specified node. Zero **
**                or more name/value pairs are be specified as the   **
**                arguments.                                         **
**                                                                   **
**            hasAttribute(node,name)                                **
**                Returns 1 if the specified node has an attribute   **
**                with the specified name, else 0.                   **
**                                                                   **
**            getParentNode(node)                                    **
**            getParent(node)                                        **
**                Returns the node number of the specified node's    **
**                parent. If the node number returned is 0, then the **
**                specified node is the root node.                   **
**                All nodes have a parent (except the root node).    **
**                                                                   **
**            getFirstChild(node)                                    **
**                Returns the node number of the specified node's    **
**                first child node.                                  **
**                                                                   **
**            getLastChild(node)                                     **
**                Returns the node number of the specified node's    **
**                last child node.                                   **
**                                                                   **
**            getChildNodes(node)                                    **
**            getChildren(node)                                      **
**                Returns a space-delimited list of node numbers of  **
**                the children of the specified node. You can use    **
**                this list to step through the children as follows: **
**                  children = getChildren(node)                     **
**                  say 'Node' node 'has' words(children) 'children' **
**                  do i = 1 to words(children)                      **
**                     child = word(children,i)                      **
**                     say 'Node' child 'is' getName(child)          **
**                  end                                              **
**                                                                   **
**            getChildrenByName(node,name)                           **
**                Returns a space-delimited list of node numbers of  **
**                the immediate children of the specified node which **
**                are called 'name'. Names are case-sensitive.       **
**                                                                   **
**            getElementsByTagName(node,name)                        **
**                Returns a space-delimited list of node numbers of  **
**                the descendants of the specified node which are    **
**                called 'name'. Names are case-sensitive.           **
**                                                                   **
**            getNextSibling(node)                                   **
**                Returns the node number of the specified node's    **
**                next sibling node. That is, the next node sharing  **
**                the same parent.                                   **
**                                                                   **
**            getPreviousSibling(node)                               **
**                Returns the node number of the specified node's    **
**                previous sibling node. That is, the previous node  **
**                sharing the same parent.                           **
**                                                                   **
**            getProcessingInstruction(name)                         **
**                Returns the value of the PI with the specified     **
**                target name.                                       **
**                                                                   **
**            getProcessingInstructionList()                         **
**                Returns a space-delimited list of the names of all **
**                PI target names.                                   **
**                                                                   **
**            getNodeType(node)                                      **
**                Returns a number representing the specified node's **
**                type. The possible values can be compared to the   **
**                following global variables:                        **
**                g.0ELEMENT_NODE                = 1                 **
**                g.0ATTRIBUTE_NODE              = 2                 **
**                g.0TEXT_NODE                   = 3                 **
**                g.0CDATA_SECTION_NODE          = 4                 **
**                g.0ENTITY_REFERENCE_NODE       = 5                 **
**                g.0ENTITY_NODE                 = 6                 **
**                g.0PROCESSING_INSTRUCTION_NODE = 7                 **
**                g.0COMMENT_NODE                = 8                 **
**                g.0DOCUMENT_NODE               = 9                 **
**                g.0DOCUMENT_TYPE_NODE          = 10                **
**                g.0DOCUMENT_FRAGMENT_NODE      = 11                **
**                g.0NOTATION_NODE               = 12                **
**                Note: as this exposes internal implementation      **
**                details, it is best not to use this routine.       **
**                Consider using isTextNode() etc instead.           **
**                                                                   **
**            isCDATA(node)                                          **
**                Returns 1 if the specified node is an unparsed     **
**                character data (CDATA) node, else 0. CDATA nodes   **
**                are used to contain content that you do not want   **
**                to be treated as XML data. For example, HTML data. **
**                                                                   **
**            isElementNode(node)                                    **
**                Returns 1 if the specified node is an element node,**
**                else 0.                                            **
**                                                                   **
**            isTextNode(node)                                       **
**                Returns 1 if the specified node is a text node,    **
**                else 0.                                            **
**                                                                   **
**            isCommentNode(node)                                    **
**                Returns 1 if the specified node is a comment node, **
**                else 0. Note: when a document is parsed, comment   **
**                nodes are ignored. This routine returns 1 iff a    **
**                comment node has been inserted into the in-memory  **
**                document tree by using createComment().            **
**                                                                   **
**            hasChildren(node)                                      **
**                Returns 1 if the specified node has one or more    **
**                child nodes, else 0.                               **
**                                                                   **
**            getDocType(doctype)                                    **
**                Gets the text of the <!DOCTYPE> prolog node.       **
**                                                                   **
**            Document creation/mutation API calls                   **
**            ------------------------------------                   **
**                                                                   **
**            createDocument(name)                                   **
**                Returns the node number of a new document node     **
**                with the specified name.                           **
**                                                                   **
**            createDocumentFragment(name)                           **
**                Returns the node number of a new document fragment **
**                node with the specified name.                      **
**                                                                   **
**            createElement(name)                                    **
**                Returns the node number of a new empty element     **
**                node with the specified name. An element node can  **
**                have child nodes.                                  **
**                                                                   **
**            createTextNode(data)                                   **
**                Returns the node number of a new text node. A text **
**                node can *not* have child nodes.                   **
**                                                                   **
**            createCDATASection(data)                               **
**                Returns the node number of a new Character Data    **
**                (CDATA) node. A CDATA node can *not* have child    **
**                nodes. CDATA nodes are used to contain content     **
**                that you do not want to be treated as XML data.    **
**                For example, HTML data.                            **
**                                                                   **
**            createComment(data)                                    **
**                Returns the node number of a new commend node.     **
**                A command node can *not* have child nodes.         **
**                                                                   **
**            appendChild(node,parent)                               **
**                Appends the specified node to the end of the list  **
**                of children of the specified parent node.          **
**                                                                   **
**            insertBefore(node,refnode)                             **
**                Inserts node 'node' before the reference node      **
**                'refnode'.                                         **
**                                                                   **
**            removeChild(node)                                      **
**                Removes the specified node from its parent and     **
**                returns its node number. The removed child is now  **
**                an orphan.                                         **
**                                                                   **
**            replaceChild(newnode,oldnode)                          **
**                Replaces the old child 'oldnode' with the new      **
**                child 'newnode' and returns the old child's node   **
**                number. The old child is now an orphan.            **
**                                                                   **
**            setAttribute(node,attrname,attrvalue)                  **
**                Adds or replaces the attribute called 'attrname'   **
**                on the specified node.                             **
**                                                                   **
**            removeAttribute(node,attrname)                         **
**                Removes the attribute called 'attrname' from the   **
**                specified node.                                    **
**                                                                   **
**            setDocType(doctype)                                    **
**                Sets the text of the <!DOCTYPE> prolog node.       **
**                                                                   **
**            cloneNode(node,[deep])                                 **
**                Creates a copy (a clone) of the specified node     **
**                and returns its node number. If deep = 1 then      **
**                all descendants of the specified node are also     **
**                cloned, else only the specified node and its       **
**                attributes are cloned.                             **
**                                                                   **
** NOTES    - 1. This parser creates global variables and so its     **
**               operation may be severely jiggered if you update    **
**               any of them accidentally (or on purpose). The       **
**               variables you should avoid updating yourself are:   **
**                                                                   **
**               g.0ATTRIBUTE.n                                      **
**               g.0ATTRIBUTE.name                                   **
**               g.0ATTRSOK                                          **
**               g.0DTD                                              **
**               g.0ENDOFDOC                                         **
**               g.0ENTITIES                                         **
**               g.0ENTITY.name                                      **
**               g.0FIRST.n                                          **
**               g.0LAST.n                                           **
**               g.0NAME.n                                           **
**               g.0NEXT.n                                           **
**               g.0NEXTID                                           **
**               g.0OPTION.name                                      **
**               g.0OPTIONS                                          **
**               g.0PARENT.n                                         **
**               g.0PI                                               **
**               g.0PI.name                                          **
**               g.0PREV.n                                           **
**               g.0PUBLIC                                           **
**               g.0ROOT                                             **
**               g.0STACK                                            **
**               g.0SYSTEM                                           **
**               g.0TEXT.n                                           **
**               g.0TYPE.n                                           **
**               g.0WHITESPACE                                       **
**               g.0XML                                              **
**               g.?XML                                              **
**               g.?XML.VERSION                                      **
**               g.?XML.ENCODING                                     **
**               g.?XML.STANDALONE                                   **
**                                                                   **
**            2. To reduce the incidence of name clashes, procedure  **
**               names that are not meant to be part of the public   **
**               API have been prefixed with '_'.                    **
**                                                                   **
**                                                                   **
** AUTHOR   - Andrew J. Armstrong <androidarmstrong+sf@gmail.com>    **
**                                                                   **
** CONTRIBUTORS -                                                    **
**            Alessandro Battilani                                   **
**              <alessandro.battilani@bancaintesa.it>                **
**                                                                   **
**                                                                   **
** HISTORY  - Date     By  Reason (most recent at the top pls)       **
**            -------- --------------------------------------------- **
**            20090822 AJA Changed from GPL to BSD license.          **
**                         Ignore whitespace to fix parse error.     **
**            20070325 AJA Whitespace defaults to '090a0d'x.         **
**            20070323 AJA Added createDocumentFragment().           **
**                         Added isDocumentFragmentNode().           **
**                         Added isDocumentNode().                   **
**            20060915 AJA Added cloneNode().                        **
**                         Added deepClone().                        **
**                         Changed removeChild() to return the       **
**                         node number of the child instead of       **
**                         clearing it.                              **
**                         Changed replaceChild() to return the      **
**                         node number of the old child instead      **
**                         of clearing it.                           **
**            20060913 AJA Fixed bug in _resolveEntities().          **
**            20060808 AB  Added support for reading from a DD       **
**                         name when running IRXJCL on MVS.          **
**                         This change was contributed by            **
**                         Alessandro Battilani from Banca           **
**                         Intesa, Italy.                            **
**            20060803 AJA Fixed loop in getAttributeMap().          **
**            20051025 AJA Now checks parentage before adding a      **
**                         child node:                               **
**                         Fixed appendChild(id,parent)              **
**                         Fixed insertBefore(id,ref)                **
**            20051014 AJA Added alias routine names to more         **
**                         closely match the DOM specification.      **
**                         Specifically:                             **
**                         Added getNodeName()                       **
**                         Added getNodeValue()                      **
**                         Added getParentNode()                     **
**                         Added getChildNodes()                     **
**                         Added hasChildNodes()                     **
**                         Added getElementsByTagName()      .       **
**            20050919 AJA Added setAttributes helper routine.       **
**            20050914 AJA Added createComment and isComment.        **
**            20050913 AJA Added get/setDocType routines.            **
**            20050907 AJA Added _setDefaultEntities routine.        **
**            20050601 AJA Added '250d'x to whitespace for TSO.      **
**            20050514 AJA Removed getAttributes API call and        **
**                         reworked attribute processing.            **
**                         Added toString API call.                  **
**            20040706 AJA Added creation/modification support.      **
**            20031216 AJA Bugfix: _parseElement with no attrs       **
**                         causes crash.                             **
**            20031031 AJA Correctly parse '/' in attributes.        **
**                         Fixed entity resolution.                  **
**            20030912 AJA Bugfix: Initialize sXmlData first.        **
**                         Bugfix: Correctly parse a naked '>'       **
**                         present in an attribute value.            **
**                         Enhancement: DUMP option now displays     **
**                         first part of each text node.             **
**            20030901 AJA Initial version.                          **
**                                                                   **
**********************************************************************/

  parse source . . sSourceFile .
  parse value sourceline(1) with . sVersion
  say 'Rexx XML Parser' sVersion
  say 'You cannot invoke this rexx by itself!'
  say
  say 'This rexx is a collection of subroutines to be called'
  say 'from your own rexx procedures. You should either:'
  say '  - Append this procedure to your own rexx procedure,'
  say '    or,'
  say '  - Append the following line to your rexx:'
  say '    /* INCLUDE' sSourceFile '*/'
  say '    ...and run the rexx preprocessor:'
  say '    rexxpp myrexx myrexxpp'
  say '    This will create myrexxpp by appending this file to myrexx'
exit

/*-------------------------------------------------------------------*
 * Set up global variables for the parser
 *-------------------------------------------------------------------*/

initParser: procedure expose g.
  parse arg sOptions
  g. = '' /* Note: stuffs up caller who may have set g. variables */
  g.0OPTIONS = translate(sOptions)
  sOptions = 'DEBUG DUMP NOBLANKS'
  do i = 1 to words(sOptions)
    sOption = word(sOptions,i)
    g.0OPTION.sOption = wordpos(sOption,g.0OPTIONS) > 0
  end

  parse source sSystem sInvocation sSourceFile
  select
    when sSystem = 'WIN32'  then g.0WHITESPACE = '090a0d'x
    when sSystem = 'TSO'    then g.0WHITESPACE = '05250d'x
    otherwise                    g.0WHITESPACE = '090a0d'x /*20070325*/
  end

  g.0LEADERS = '_:ABCDEFGHIJKLMNOPQRSTUVWXYZ' ||,
                 'abcdefghijklmnopqrstuvwxyz'
  g.0OTHERS  = g.0LEADERS'.-0123456789'

  call _setDefaultEntities

  /* Not all of the following node types are used... */
  g.0ELEMENT_NODE            =  1; g.0NODETYPE.1 = 'Element'
  g.0ATTRIBUTE_NODE          =  2; g.0NODETYPE.2 = 'Attribute'
  g.0TEXT_NODE               =  3; g.0NODETYPE.3 = 'Text'
  g.0CDATA_SECTION_NODE      =  4; g.0NODETYPE.4 = 'CDATA Section'
  g.0ENTITY_REFERENCE_NODE   =  5     /* NOT USED */
  g.0ENTITY_NODE             =  6     /* NOT USED */
  g.0PROCESSING_INSTRUCTION_NODE = 7  /* NOT USED */
  g.0COMMENT_NODE            =  8; g.0NODETYPE.8 = 'Comment'
  g.0DOCUMENT_NODE           =  9; g.0NODETYPE.9 = 'Document'
  g.0DOCUMENT_TYPE_NODE      = 10    /* NOT USED */
  g.0DOCUMENT_FRAGMENT_NODE  = 11; g.0NODETYPE.11 = 'Document Fragment'
  g.0NOTATION_NODE           = 12    /* NOT USED */




  g.0ENDOFDOC = 0
return

/*-------------------------------------------------------------------*
 * Clean up parser
 *-------------------------------------------------------------------*/

destroyParser: procedure expose g.
  /* Note: it would be easy to just "drop g.", but this could
     possibly stuff up the caller who may be using other
     "g." variables...
     todo: revisit this one (parser may have to 'own' g. names)
  */
  drop g.?XML g.0ROOT g.0SYSTEM g.0PUBLIC g.0DTD
  do i = 1 to words(g.0PI)
    sName = word(g.0PI,i)
    drop g.0PI.sName
  end
  drop g.0PI
  do i = 1 to words(g.0ENTITIES)
    sName = word(g.0ENTITIES,i)
    drop g.0ENTITY.sName
  end
  drop g.0ENTITIES
  call _setDefaultEntities
  if datatype(g.0NEXTID,'WHOLE')
  then do
    do i = 1 to g.0NEXTID
      drop g.0PARENT.i g.0FIRST.i g.0LAST.i g.0PREV.i,
           g.0NEXT.i g.0NAME.i g.0TEXT.i
    end
  end
  drop g.0NEXTID g.0STACK g.0ENDOFDOC
return


/*-------------------------------------------------------------------*
 * Read a file into a string
 *-------------------------------------------------------------------*/

parseFile: procedure expose g.
  parse arg sFile
  parse source sSystem sInvocation sSourceFile . . . sInitEnv .
  sXmlData = ''
  select
    when sSystem = 'TSO' & sInitEnv = 'TSO' then do
      /* sFile is a dataset name */
      address TSO
      junk = OUTTRAP('junk.') /* Trap and discard messages */
      'ALLOCATE DD(INPUT) DSN('sFile')'
      'EXECIO * DISKR INPUT (FINIS'
      'FREE DD(INPUT)'
      address
      do queued()
        parse pull sLine
        sXmlData = sXmlData || sLine
      end
      junk = OUTTRAP('OFF')
    end
    when sSystem = 'TSO' & sInitEnv = 'MVS' then do
      /* sFile is a DD name */
      address MVS 'EXECIO * DISKR' sFile '(FINIS'
      do queued()
        parse pull sLine
        sXmlData = sXmlData || sLine
      end
    end
    otherwise do
      sXmlData = charin(sFile,,chars(sFile))
    end
  end
return parseString(sXmlData)

/*-------------------------------------------------------------------*
 * Parse a string containing XML
 *-------------------------------------------------------------------*/

parseString: procedure expose g.
  parse arg g.0XML
  call _parseXmlDecl
  do while pos('<',g.0XML) > 0
    parse var g.0XML sLeft'<'sData
    select
      when left(sData,1) = '?'         then call _parsePI      sData
      when left(sData,9) = '!DOCTYPE ' then call _parseDocType sData
      when left(sData,3) = '!--'       then call _parseComment sData
      otherwise                             call _parseElement sData
    end
  end
return 0

/*-------------------------------------------------------------------*
 * <?xml version="1.0" encoding="..." ...?>
 *-------------------------------------------------------------------*/

_parseXmlDecl: procedure expose g.
  if left(g.0XML,6) = '<?xml '
  then do
    parse var g.0XML '<?xml 'sXMLDecl'?>'g.0XML
    g.?xml = space(sXMLDecl)
    sTemp = _getNormalizedAttributes(g.?xml)
    parse var sTemp 'version='g.?xml.version'ff'x
    parse var sTemp 'encoding='g.?xml.encoding'ff'x
    parse var sTemp 'standalone='g.?xml.standalone'ff'x
  end
return

/*-------------------------------------------------------------------*
 * <?target string?>
 *-------------------------------------------------------------------*/

_parsePI: procedure expose g.
  parse arg '?'sProcessingInstruction'?>'g.0XML
  call _setProcessingInstruction sProcessingInstruction
return

/*-------------------------------------------------------------------*
 * <!DOCTYPE root SYSTEM "sysid">
 * <!DOCTYPE root SYSTEM "sysid" [internal dtd]>
 * <!DOCTYPE root PUBLIC "pubid" "sysid">
 * <!DOCTYPE root PUBLIC "pubid" "sysid" [internal dtd]>
 * <!DOCTYPE root [internal dtd]>
 *-------------------------------------------------------------------*/

_parseDocType: procedure expose g.
  parse arg '!DOCTYPE' sDocType'>'
  if g.0ROOT <> ''
  then call _abort 'XML002E Multiple "<!DOCTYPE" declarations'
  if pos('[',sDocType) > 0
  then do
    parse arg '!DOCTYPE' sDocType'['g.0DTD']>'g.0XML
    parse var sDocType g.0ROOT sExternalId
    if sExternalId <> '' then call _parseExternalId sExternalId
    g.0DTD = strip(g.0DTD)
    call _parseDTD g.0DTD
  end
  else do
    parse arg '!DOCTYPE' g.0ROOT sExternalId'>'g.0XML
    if sExternalId <> '' then call _parseExternalId sExternalId
  end
  g.0ROOT = strip(g.0ROOT)
return

/*-------------------------------------------------------------------*
 * SYSTEM "sysid"
 * PUBLIC "pubid" "sysid"
 *-------------------------------------------------------------------*/

_parseExternalId: procedure expose g.
  parse arg sExternalIdType .
  select
    when sExternalIdType = 'SYSTEM' then do
      parse arg . g.0SYSTEM
      g.0SYSTEM = removeQuotes(g.0SYSTEM)
    end
    when sExternalIdType = 'PUBLIC' then do
      parse arg . g.0PUBLIC g.0SYSTEM
      g.0PUBLIC = removeQuotes(g.0PUBLIC)
      g.0SYSTEM = removeQuotes(g.0SYSTEM)
    end
    otherwise do
       parse arg sExternalEntityDecl
       call _abort 'XML003E Invalid external entity declaration:',
                   sExternalEntityDecl
    end
  end
return


/*-------------------------------------------------------------------*
 * <!ENTITY name "value">
 * <!ENTITY name SYSTEM "sysid">
 * <!ENTITY name PUBLIC "pubid" "sysid">
 * <!ENTITY % name pedef>
 * <!ELEMENT elementname contentspec>
 * <!ATTLIST elementname attrname attType DefaultDecl ...>
 * <!NOTATION name notationdef>
 *-------------------------------------------------------------------*/

_parseDTD: procedure expose g.
  parse arg sDTD
  do while pos('<!',sDTD) > 0
    parse var sDTD '<!'sDecl sName sValue'>'sDTD
    select
      when sDecl = 'ENTITY' then do
        parse var sValue sWord1 .
        select
          when sName = '%'       then nop
          when sWord1 = 'SYSTEM' then nop
          when sWord1 = 'PUBLIC' then nop
          otherwise do
            sValue = _resolveEntities(removeQuotes(sValue))
            call _setEntity sName,sValue
          end
        end
      end
      otherwise nop /* silently ignore other possibilities for now */
    end
  end
return

/*-------------------------------------------------------------------*
 * <!-- comment -->
 *-------------------------------------------------------------------*/

_parseComment: procedure expose g.
  parse arg sComment'-->'g.0XML
  /* silently ignore comments */
return

/*-------------------------------------------------------------------*
 * <tag attr1="value1" attr2="value2" ...>...</tag>
 * <tag attr1="value1" attr2="value2" .../>
 *-------------------------------------------------------------------*/

_parseElement: procedure expose g.
  parse arg sXML

  if g.0ENDOFDOC
  then call _abort 'XML004E Only one top level element is allowed.',
                  'Found:' subword(g.0XML,1,3)
  call _startDocument

  g.0XML = '<'sXML
  do while pos('<',g.0XML) > 0 & \g.0ENDOFDOC
    parse var g.0XML sLeft'<'sBetween'>'g.0XML

    if length(sLeft) > 0
    then call _characters sLeft

    if g.0OPTION.DEBUG
    then say g.0STACK sBetween

    if left(sBetween,8) = '![CDATA['
    then do
      g.0XML = sBetween'>'g.0XML            /* ..back it out! */
      parse var g.0XML '![CDATA['sBetween']]>'g.0XML
      call _characterData sBetween
    end
    else do
      sBetween = removeWhiteSpace(sBetween)                /*20090822*/
      select
        when left(sBetween,3) = '!--' then do    /* <!-- comment --> */
          if right(sBetween,2) <> '--'
          then do  /* backup a bit and look for end-of-comment */
            g.0XML = sBetween'>'g.0XML
            if pos('-->',g.0XML) = 0
            then call _abort 'XML005E End of comment missing after:',
                            '<'g.0XML
            parse var g.0XML sComment'-->'g.0XML
          end
        end
        when left(sBetween,1) = '?' then do    /* <?target string?> */
          parse var sBetween '?'sProcessingInstruction'?'
          call _setProcessingInstruction sProcessingInstruction
        end
        when left(sBetween,1) = '/' then do    /* </tag> */
          call _endElement substr(sBetween,2)   /* tag */
        end
        when  right(sBetween,1) = '/'  /* <tag ...attrs.../> */
        then do
          parse var sBetween sTagName sAttrs
          if length(sAttrs) > 0                            /*20031216*/
          then sAttrs = substr(sAttrs,1,length(sAttrs)-1)  /*20031216*/
          else parse var sTagName sTagName'/'     /* <tag/>  20031216*/
          sAttrs = _getNormalizedAttributes(sAttrs)
          call _startElement sTagName sAttrs
          call _endElement sTagName
        end
        otherwise do              /* <tag ...attrs ...> ... </tag>  */
          parse var sBetween sTagName sAttrs
          sAttrs = _getNormalizedAttributes(sAttrs)
          if g.0ATTRSOK
          then do
            call _startElement sTagName sAttrs
          end
          else do /* back up a bit and look for the real end of tag */
            g.0XML = '<'sBetween'&gt;'g.0XML
            if pos('>',g.0XML) = 0
            then call _abort 'XML006E Missing end tag for:' sTagName
            /* reparse on next cycle avoiding premature '>'...*/
          end
        end
      end
    end
  end

  call _endDocument
return

_startDocument: procedure expose g.
  g.0NEXTID = 0
  g.0STACK = 0
return

_startElement:  procedure expose g.
  parse arg sTagName sAttrs
  id = _getNextId()
  call _updateLinkage id
  g.0NAME.id = sTagName
  g.0TYPE.id = g.0ELEMENT_NODE
  call _addAttributes id,sAttrs
  cid = _pushElement(id)
return

_updateLinkage: procedure expose g.
  parse arg id
  parent = _peekElement()
  g.0PARENT.id = parent
  parentsLastChild = g.0LAST.parent
  g.0NEXT.parentsLastChild = id
  g.0PREV.id = parentsLastChild
  g.0LAST.parent = id
  if g.0FIRST.parent = ''
  then g.0FIRST.parent = id
return

_characterData: procedure expose g.
  parse arg sChars
  id = _getNextId()
  call _updateLinkage id
  g.0TEXT.id = sChars
  g.0TYPE.id = g.0CDATA_SECTION_NODE
return

_characters: procedure expose g.
  parse arg sChars
  sText = _resolveEntities(sChars)
  if g.0OPTION.NOBLANKS & removeWhitespace(sText) = ''
  then return
  id = _getNextId()
  call _updateLinkage id
  g.0TEXT.id = sText
  g.0TYPE.id = g.0TEXT_NODE
return

_endElement: procedure expose g.
  parse arg sTagName
  id = _popElement()
  g.0ENDOFDOC = id = 1
  if sTagName == g.0NAME.id
  then nop
  else call _abort,
           'XML007E Expecting </'g.0NAME.id'> but found </'sTagName'>'
return

_endDocument: procedure expose g.
  id = _peekElement()
  if id <> 0
  then call _abort 'XML008E End of document tag missing: 'id getName(id)
  if g.0ROOT <> '' & g.0ROOT <> getName(getRoot())
  then call _abort 'XML009E Root element name "'getName(getRoot())'"',
                  'does not match DTD root "'g.0ROOT'"'

  if g.0OPTION.DUMP
  then call _displayTree
return

_displayTree: procedure expose g.
  say   right('',4),
        right('',4),
        left('',12),
        right('',6),
        '--child--',
        '-sibling-',
        'attribute'
  say   right('id',4),
        right('type',4),
        left('name',12),
        right('parent',6),
        right('1st',4),
        right('last',4),
        right('prev',4),
        right('next',4),
        right('1st',4),
        right('last',4)
  do id = 1 to g.0NEXTID
    if g.0PARENT.id <> '' | id = 1 /* skip orphans */
    then do
      select
        when g.0TYPE.id = g.0CDATA_SECTION_NODE then sName = '#CDATA'
        when g.0TYPE.id = g.0TEXT_NODE          then sName = '#TEXT'
        otherwise                                    sName = g.0NAME.id
      end
      say right(id,4),
          right(g.0TYPE.id,4),
          left(sName,12),
          right(g.0PARENT.id,6),
          right(g.0FIRST.id,4),
          right(g.0LAST.id,4),
          right(g.0PREV.id,4),
          right(g.0NEXT.id,4),
          right(g.0FIRSTATTR.id,4),
          right(g.0LASTATTR.id,4),
          left(removeWhitespace(g.0TEXT.id),19)
    end
  end
return

_pushElement: procedure expose g.
  parse arg id
  g.0STACK = g.0STACK + 1
  nStackDepth = g.0STACK
  g.0STACK.nStackDepth = id
return id

_popElement: procedure expose g.
  n = g.0STACK
  if n = 0
  then id = 0
  else do
    id = g.0STACK.n
    g.0STACK = g.0STACK - 1
  end
return id

_peekElement: procedure expose g.
  n = g.0STACK
  if n = 0
  then id = 0
  else id = g.0STACK.n
return id

_getNextId: procedure expose g.
  g.0NEXTID = g.0NEXTID + 1
return g.0NEXTID

_addAttributes: procedure expose g.
  parse arg id,sAttrs
  do while pos('ff'x,sAttrs) > 0
    parse var sAttrs sAttrName'='sAttrValue 'ff'x sAttrs
    sAttrName = removeWhitespace(sAttrName)
    call _addAttribute id,sAttrName,sAttrValue
  end
return

_addAttribute: procedure expose g.
  parse arg id,sAttrName,sAttrValue
  aid = _getNextId()
  g.0TYPE.aid = g.0ATTRIBUTE_NODE
  g.0NAME.aid = sAttrName
  g.0TEXT.aid = _resolveEntities(sAttrValue)
  g.0PARENT.aid = id
  g.0NEXT.aid = ''
  g.0PREV.aid = ''
  if g.0FIRSTATTR.id = '' then g.0FIRSTATTR.id = aid
  if g.0LASTATTR.id <> ''
  then do
    lastaid = g.0LASTATTR.id
    g.0NEXT.lastaid = aid
    g.0PREV.aid = lastaid
  end
  g.0LASTATTR.id = aid
return

/*-------------------------------------------------------------------*
 * Resolve attributes to an internal normalized form:
 *   name1=value1'ff'x name2=value2'ff'x ...
 * This makes subsequent parsing of attributes easier.
 * Note: this design may fail for certain UTF-8 content
 *-------------------------------------------------------------------*/

_getNormalizedAttributes: procedure expose g.
  parse arg sAttrs
  g.0ATTRSOK = 0
  sNormalAttrs = ''
  parse var sAttrs sAttr'='sAttrs
  do while sAttr <> ''
    sAttr = removeWhitespace(sAttr)
    select
      when left(sAttrs,1) = '"' then do
        if pos('"',sAttrs,2) = 0 /* if no closing "   */
        then return ''           /* then not ok       */
        parse var sAttrs '"'sAttrValue'"'sAttrs
      end
      when left(sAttrs,1) = "'" then do
        if pos("'",sAttrs,2) = 0 /* if no closing '   */
        then return ''           /* then not ok       */
        parse var sAttrs "'"sAttrValue"'"sAttrs
      end
      otherwise return ''        /* no opening ' or " */
    end
    sAttrValue = removeWhitespace(sAttrValue)
    sNormalAttrs = sNormalAttrs sAttr'='sAttrValue'ff'x
    parse var sAttrs sAttr'='sAttrs
  end
  g.0ATTRSOK = 1
  /* Note: always returns a leading blank and is required by
    this implementation */
return _resolveEntities(sNormalAttrs)


/*-------------------------------------------------------------------*
 *  entityref  := '&' entityname ';'
 *  entityname := ('_',':',letter) (letter,digit,'.','-','_',':')*
 *-------------------------------------------------------------------*/


_resolveEntities: procedure expose g.
  parse arg sText
  if pos('&',sText) > 0
  then do
    sNewText = ''
    do while pos('&',sText) > 0
      parse var sText sLeft'&'sEntityRef
      if pos(left(sEntityRef,1),'#'g.0LEADERS) > 0
      then do
        n = verify(sEntityRef,g.0OTHERS,'NOMATCH',2)
        if n > 1
        then do
          if substr(sEntityRef,n,1) = ';'
          then do
            sEntityName = left(sEntityRef,n-1)
            sEntity = _getEntity(sEntityName)
            sNewText = sNewText || sLeft || sEntity
            sText = substr(sEntityRef,n+1)
          end
          else do
            sNewText = sNewText || sLeft'&'
            sText = sEntityRef
          end
        end
        else do
          sNewText = sNewText || sLeft'&'
          sText = sEntityRef
        end
      end
      else do
        sNewText = sNewText || sLeft'&'
        sText = sEntityRef
      end
    end
    sText = sNewText || sText
  end
return sText

/*-------------------------------------------------------------------*
 * &entityname;
 * &#nnnnn;
 * &#xXXXX;
 *-------------------------------------------------------------------*/

_getEntity: procedure expose g.
  parse arg sEntityName
  if left(sEntityName,1) = '#' /* #nnnnn  OR  #xXXXX */
  then sEntity = _getCharacterEntity(sEntityName)
  else sEntity = _getStringEntity(sEntityName)
return sEntity

/*-------------------------------------------------------------------*
 * &#nnnnn;
 * &#xXXXX;
 *-------------------------------------------------------------------*/

_getCharacterEntity: procedure expose g.
  parse arg sEntityName
  if substr(sEntityName,2,1) = 'x'
  then do
    parse arg 3 xEntity
    if datatype(xEntity,'XADECIMAL')
    then sEntity = x2c(xEntity)
    else call _abort,
              'XML010E Invalid hexadecimal character reference: ',
              '&'sEntityName';'
  end
  else do
    parse arg 2 nEntity
    if datatype(nEntity,'WHOLE')
    then sEntity = d2c(nEntity)
    else call _abort,
              'XML011E Invalid decimal character reference:',
              '&'sEntityName';'
  end
return sEntity

/*-------------------------------------------------------------------*
 * &entityname;
 *-------------------------------------------------------------------*/

_getStringEntity: procedure expose g.
  parse arg sEntityName
  if wordpos(sEntityName,g.0ENTITIES) = 0
  then call _abort 'XML012E Unable to resolve entity &'sEntityName';'
  sEntity = g.0ENTITY.sEntityName
return sEntity

_setDefaultEntities: procedure expose g.
  g.0ENTITIES = ''
  g.0ESCAPES = '<>&"' || "'"
  sEscapes = 'lt gt amp quot apos'
  do i = 1 to length(g.0ESCAPES)
    c = substr(g.0ESCAPES,i,1)
    g.0ESCAPE.c = word(sEscapes,i)
  end
  call _setEntity 'amp','&'
  call _setEntity 'lt','<'
  call _setEntity 'gt','>'
  call _setEntity 'apos',"'"
  call _setEntity 'quot','"'
return

_setEntity: procedure expose g.
  parse arg sEntityName,sValue
  if wordpos(sEntityName,g.0ENTITIES) = 0
  then g.0ENTITIES = g.0ENTITIES sEntityName
  g.0ENTITY.sEntityName = sValue
return

_setProcessingInstruction: procedure expose g.
  parse arg sTarget sInstruction
  if wordpos(sTarget,g.0PI) = 0
  then g.0PI = g.0PI sTarget
  g.0PI.sTarget = strip(sInstruction)
return

_abort: procedure expose g.
  parse arg sMsg
  say 'ABORT:' sMsg
  call destroyParser
exit 16

_clearNode: procedure expose g.
  parse arg id
  g.0NAME.id       = '' /* The node's name */
  g.0PARENT.id     = '' /* The node's parent */
  g.0FIRST.id      = '' /* The node's first child */
  g.0LAST.id       = '' /* The node's last child */
  g.0NEXT.id       = '' /* The node's next sibling */
  g.0PREV.id       = '' /* The node's previous sibling */
  g.0TEXT.id       = '' /* The node's text content */
  g.0TYPE.id       = '' /* The node's type */
  g.0FIRSTATTR.id  = '' /* The node's first attribute */
  g.0LASTATTR.id   = '' /* The node's last attribute */
return

/*-------------------------------------------------------------------*
 * Utility API
 *-------------------------------------------------------------------*/

removeWhitespace: procedure expose g.
  parse arg sData
return space(translate(sData,'',g.0WHITESPACE))

removeQuotes: procedure expose g.
  parse arg sValue
  c = left(sValue,1)
  select
    when c = '"' then parse var sValue '"'sValue'"'
    when c = "'" then parse var sValue "'"sValue"'"
    otherwise nop
  end
return sValue

/*-------------------------------------------------------------------*
 * Document Object Model ;-) API
 *-------------------------------------------------------------------*/

getRoot: procedure expose g. /* DEPRECATED */
return 1

getDocumentElement: procedure expose g.
return 1

getName: getNodeName: procedure expose g.
  parse arg id
return g.0NAME.id

getText: getNodeValue: procedure expose g.
  parse arg id
return g.0TEXT.id

getNodeType: procedure expose g.
  parse arg id
return g.0TYPE.id

isElementNode: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0ELEMENT_NODE

isTextNode: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0TEXT_NODE

isCommentNode: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0COMMENT_NODE

isCDATA: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0CDATA_SECTION_NODE

isDocumentNode: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0DOCUMENT_NODE

isDocumentFragmentNode: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0DOCUMENT_FRAGMENT_NODE

/**
 * This is similar to the DOM API's NamedNodeMap concept, except that
 * the returned structure is built in global variables (so calling
 * it a second time will destroy the structure built on the first
 * call). The other difference is that you can access the attributes
 * by name or ordinal number. For example, g.0ATTRIBUTE.2 is the value
 * of the second attribute. If the second attribute was called 'x',
 * then you could also access it by g.0ATTRIBUTE.x (as long as x='x')
 * Note, g.0ATTRIBUTE.0 will always contain a count of the number of
 * attributes in the map.
 */
getAttributeMap: procedure expose g.
  parse arg id
  if datatype(g.0ATTRIBUTE.0,'WHOLE')  /* clear any existing map */
  then do
    do i = 1 to g.0ATTRIBUTE.0
      sName = g.0ATTRIBUTE.i
      drop g.0ATTRIBUTE.sName g.0ATTRIBUTE.i
    end
  end
  g.0ATTRIBUTE.0 = 0
  if \_canHaveAttributes(id) then return
  aid = g.0FIRSTATTR.id /* id of first attribute of element 'id' */
  do i = 1 while aid <> ''
    sName = g.0NAME.aid
    sValue = g.0TEXT.aid
    g.0ATTRIBUTE.0 = i
    g.0ATTRIBUTE.i = sName
    g.0ATTRIBUTE.sName = sValue
    aid = g.0NEXT.aid /* id of next attribute */
  end
return

getAttributeCount: procedure expose g.
  parse arg id
  nAttributeCount = 0
  if _canHaveAttributes(id)
  then do
    aid = g.0FIRSTATTR.id /* id of first attribute of element 'id' */
    do while aid <> ''
      nAttributeCount = nAttributeCount + 1
      aid = g.0NEXT.aid /* id of next attribute */
    end
  end
return nAttributeCount

getAttributeNames: procedure expose g.
  parse arg id
  sNames = ''
  if _canHaveAttributes(id)
  then do
    aid = g.0FIRSTATTR.id /* id of first attribute of element 'id' */
    do while aid <> ''
      sNames = sNames g.0NAME.aid
      aid = g.0NEXT.aid /* id of next attribute */
    end
  end
return strip(sNames)

getAttribute: procedure expose g.
  parse arg id,sAttrName
  sValue = ''
  if _canHaveAttributes(id)
  then do
    aid = g.0FIRSTATTR.id /* id of first attribute of element 'id' */
    if aid <> ''
    then do
      n = 1
      do while aid <> '' & (g.0NAME.aid <> sAttrName & n <> sAttrName)
        aid = g.0NEXT.aid
        n = n + 1
      end
      if g.0NAME.aid = sAttrName | n = sAttrName
      then sValue = g.0TEXT.aid
    end
  end
return sValue

getAttributeName: procedure expose g.
  parse arg id,n
  sName = ''
  if _canHaveAttributes(id)
  then do
    aid = g.0FIRSTATTR.id /* id of first attribute of element 'id' */
    if aid <> ''
    then do
      do i = 1 while aid <> '' & i < n
        aid = g.0NEXT.aid
      end
      if i = n then sName = g.0NAME.aid
    end
  end
return sName

hasAttribute: procedure expose g.
  parse arg id,sAttrName
  bHasAttribute = 0
  if _canHaveAttributes(id)
  then do
    aid = g.0FIRSTATTR.id
    if aid <> ''
    then do
      do while aid <> '' & g.0NAME.aid <> sAttrName
        aid = g.0NEXT.aid
      end
      bHasAttribute = g.0NAME.aid = sAttrName
    end
  end
return bHasAttribute

_canHaveAttributes: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0ELEMENT_NODE |,
       g.0TYPE.id = g.0DOCUMENT_NODE |,
       g.0TYPE.id = g.0DOCUMENT_FRAGMENT_NODE

_canHaveChildren: procedure expose g.
  parse arg id
return g.0TYPE.id = g.0ELEMENT_NODE |,
       g.0TYPE.id = g.0DOCUMENT_NODE |,
       g.0TYPE.id = g.0DOCUMENT_FRAGMENT_NODE

getParent: getParentNode: procedure expose g.
  parse arg id
return g.0PARENT.id

getFirstChild: procedure expose g.
  parse arg id
return g.0FIRST.id

getLastChild: procedure expose g.
  parse arg id
return g.0LAST.id

getChildren: getChildNodes: procedure expose g.
  parse arg id
  ids = ''
  id = getFirstChild(id)
  do while id <> ''
    ids = ids id
    id = getNextSibling(id)
  end
return strip(ids)

getChildrenByName: procedure expose g.
  parse arg id,sName
  ids = ''
  id = getFirstChild(id)
  do while id <> ''
    if getName(id) = sName
    then ids = ids id
    id = getNextSibling(id)
  end
return strip(ids)

getElementsByTagName: procedure expose g.
  parse arg id,sName
  ids = ''
  id = getFirstChild(id)
  do while id <> ''
    if getName(id) = sName
    then ids = ids id
    ids = ids getElementsByTagName(id,sName)
    id = getNextSibling(id)
  end
return space(ids)

getNextSibling: procedure expose g.
  parse arg id
return g.0NEXT.id

getPreviousSibling: procedure expose g.
  parse arg id
return g.0PREV.id

getProcessingInstruction: procedure expose g.
  parse arg sTarget
return g.0PI.sTarget

getProcessingInstructionList: procedure expose g.
return g.0PI

hasChildren: hasChildNodes: procedure expose g.
  parse arg id
return g.0FIRST.id <> ''

createDocument: procedure expose g.
  parse arg sName
  if sName = ''
  then call _abort,
            'XML013E Tag name omitted:',
            'createDocument('sName')'
  call destroyParser
  g.0NEXTID = 0
  id = _getNextId()
  call _clearNode id
  g.0TYPE.id = g.0DOCUMENT_NODE /* 20070323 */
  g.0NAME.id = sName
  g.0PARENT.id = 0
return id

createDocumentFragment: procedure expose g. /* 20070323 */
  parse arg sName
  if sName = ''
  then call _abort,
            'XML014E Tag name omitted:',
            'createDocumentFragment('sName')'
  id = _getNextId()
  call _clearNode id
  g.0TYPE.id = g.0DOCUMENT_FRAGMENT_NODE
  g.0NAME.id = sName
  g.0PARENT.id = 0
return id

createElement: procedure expose g.
  parse arg sName
  id = _getNextId()
  call _clearNode id
  g.0TYPE.id = g.0ELEMENT_NODE
  g.0NAME.id = sName
return id

createCDATASection: procedure expose g.
  parse arg sCharacterData
  id = _getNextId()
  call _clearNode id
  g.0TYPE.id = g.0CDATA_SECTION_NODE
  g.0TEXT.id = sCharacterData
return id

createTextNode: procedure expose g.
  parse arg sData
  id = _getNextId()
  call _clearNode id
  g.0TYPE.id = g.0TEXT_NODE
  g.0TEXT.id = sData
return id

appendChild: procedure expose g.
  parse arg id, parent
  if \_canHaveChildren(parent)
  then call _abort,
            'XML015E' g.0NODETYPE.parent 'node cannot have children:',
            'appendChild('id','parent')'
  if g.0PARENT.id = ''
  then g.0PARENT.id = parent
  else call _abort,
            'XML016E Node <'getNodeName(id)'> is already a child',
            'of <'getNodeName(g.0PARENT.id)'>:',
            'appendChild('id','parent')'
  parentsLastChild = g.0LAST.parent
  g.0NEXT.parentsLastChild = id
  g.0PREV.id = parentsLastChild
  g.0LAST.parent = id
  if g.0FIRST.parent = ''
  then g.0FIRST.parent = id
return

insertBefore: procedure expose g.
  parse arg id, ref
  parent = g.0PARENT.ref
  if \_canHaveChildren(parent)
  then call _abort,
            'XML017E' g.0NODETYPE.parent 'node cannot have children:',
            'insertBefore('id','ref')'
  if g.0PARENT.id = ''
  then g.0PARENT.id = parent
  else call _abort,
            'XML018E Node <'getNodeName(id)'> is already a child',
            'of <'getNodeName(g.0PARENT.id)'>:',
            'insertBefore('id','ref')'
  g.0NEXT.id = ref
  oldprev = g.0PREV.ref
  g.0PREV.ref = id
  g.0NEXT.oldprev = id
  g.0PREV.id = oldprev
  if g.0FIRST.parent = ref
  then g.0FIRST.parent = id
return

removeChild: procedure expose g.
  parse arg id
  parent = g.0PARENT.id
  if \_canHaveChildren(parent)
  then call _abort,
            'XML019E' g.0NODETYPE.parent 'node cannot have children:',
            'removeChild('id')'
  next = g.0NEXT.id
  prev = g.0PREV.id
  g.0NEXT.prev = next
  g.0PREV.next = prev
  if g.0FIRST.parent = id
  then g.0FIRST.parent = next
  if g.0LAST.parent = id
  then g.0LAST.parent = prev
  g.0PARENT.id = ''
  g.0NEXT.id = ''
  g.0PREV.id = ''
return id

replaceChild: procedure expose g.
  parse arg id, extant
  parent = g.0PARENT.extant
  if \_canHaveChildren(parent)
  then call _abort,
            'XML020E' g.0NODETYPE.parent 'node cannot have children:',
            'replaceChild('id','extant')'
  g.0PARENT.id = parent
  g.0NEXT.id = g.0NEXT.extant
  g.0PREV.id = g.0PREV.extant
  if g.0FIRST.parent = extant
  then g.0FIRST.parent = id
  if g.0LAST.parent = extant
  then g.0LAST.parent = id
  g.0PARENT.extant = ''
  g.0NEXT.extant = ''
  g.0PREV.extant = ''
return extant

setAttribute: procedure expose g.
  parse arg id,sAttrName,sValue
  if \_canHaveAttributes(id)
  then call _abort,
            'XML021E' g.0NODETYPE.id 'node cannot have attributes:',
            'setAttribute('id','sAttrName','sValue')'
  aid = g.0FIRSTATTR.id
  do while aid <> '' & g.0NAME.aid <> sAttrName
    aid = g.0NEXT.aid
  end
  if aid <> '' & g.0NAME.aid = sAttrName
  then g.0TEXT.aid = sValue
  else call _addAttribute id,sAttrName,sValue
return

setAttributes: procedure expose g.
  parse arg id /* ,name1,value1,name2,value2,...,namen,valuen */
  do i = 2 to arg() by 2
    sAttrName = arg(i)
    sValue = arg(i+1)
    call setAttribute id,sAttrName,sValue
  end
return

removeAttribute: procedure expose g.
  parse arg id,sAttrName
  if \_canHaveAttributes(id)
  then call _abort,
            'XML022E' g.0NODETYPE.id 'node cannot have attributes:',
            'removeAttribute('id','sAttrName')'
  aid = g.0FIRSTATTR.id
  do while aid <> '' & g.0NAME.aid <> sAttrName
    aid = g.0NEXT.aid
  end
  if aid <> '' & g.0NAME.aid = sAttrName
  then do
    prevaid = g.0PREV.aid
    nextaid = g.0NEXT.aid
    if prevaid = ''  /* if we are deleting the first attribute */
    then g.0FIRSTATTR.id = nextaid /* make next attr the first */
    else g.0NEXT.prevaid = nextaid /* link prev attr to next attr */
    if nextaid = '' /* if we are deleting the last attribute */
    then g.0LASTATTR.id  = prevaid /* make prev attr the last */
    else g.0PREV.nextaid = prevaid /* link next attr to prev attr */
    call _clearNode aid
  end
return

toString: procedure expose g.
  parse arg node
  if node = '' then node = getRoot()
  if node = getRoot()
  then sXML = _getProlog()_getNode(node)
  else sXML = _getNode(node)
return sXML

_getProlog: procedure expose g.
  if g.?xml.version = ''
  then sVersion = '1.0'
  else sVersion = g.?xml.version
  if g.?xml.encoding = ''
  then sEncoding = 'UTF-8'
  else sEncoding = g.?xml.encoding
  if g.?xml.standalone = ''
  then sStandalone = 'yes'
  else sStandalone = g.?xml.standalone
  sProlog = '<?xml version="'sVersion'"',
            'encoding="'sEncoding'"',
            'standalone="'sStandalone'"?>'
return sProlog

_getNode: procedure expose g.
  parse arg node
  select
    when g.0TYPE.node = g.0ELEMENT_NODE then,
         sXML = _getElementNode(node)
    when g.0TYPE.node = g.0TEXT_NODE then,
         sXML = escapeText(removeWhitespace(getText(node)))
    when g.0TYPE.node = g.0ATTRIBUTE_NODE then,
         sXML = getName(node)'="'escapeText(getText(node))'"'
    when g.0TYPE.node = g.0CDATA_SECTION_NODE then,
         sXML = '<![CDATA['getText(node)']]>'
    otherwise sXML = '' /* TODO: throw an error here? */
  end
return sXML

_getElementNode: procedure expose g.
  parse arg node
  sName = getName(node)
  sAttrs = ''
  attr = g.0FIRSTATTR.node
  do while attr <> ''
    sAttrs = sAttrs _getNode(attr)
    attr = g.0NEXT.attr
  end
  if hasChildren(node)
  then do
    if sAttrs = ''
    then sXML = '<'sName'>'
    else sXML = '<'sName strip(sAttrs)'>'
    child = getFirstChild(node)
    do while child <> ''
      sXML = sXML || _getNode(child)
      child = getNextSibling(child)
    end
    sXML = sXML'</'sName'>'
  end
  else do
    if sAttrs = ''
    then sXML = '<'sName'/>'
    else sXML = '<'sName strip(sAttrs)'/>'
  end
return sXML

escapeText: procedure expose g.
  parse arg sText
  n = verify(sText,g.0ESCAPES,'MATCH')
  if n > 0
  then do
    sNewText = ''
    do while n > 0
      sLeft = ''
      n = n - 1
      if n = 0
      then parse var sText c +1 sText
      else parse var sText sLeft +(n) c +1 sText
      sNewText = sNewText || sLeft'&'g.0ESCAPE.c';'
      n = verify(sText,g.0ESCAPES,'MATCH')
    end
    sText = sNewText || sText
  end
return sText

/*-------------------------------------------------------------------*
 * SYSTEM "sysid"
 * PUBLIC "pubid" "sysid"
 *-------------------------------------------------------------------*/
setDocType: procedure expose g.
  parse arg sDocType
  g.0DOCTYPE = sDocType
return

getDocType: procedure expose g.
return g.0DOCTYPE

createComment: procedure expose g.
  parse arg sData
  id = _getNextId()
  call _clearNode id
  g.0TYPE.id = g.0COMMENT_NODE
  g.0TEXT.id = sData
return id

deepClone: procedure expose g.
  parse arg node
return cloneNode(node,1)

cloneNode: procedure expose g.
  parse arg node,bDeep
  clone = _getNextId()
  call _clearNode clone
  g.0TYPE.clone = g.0TYPE.node
  g.0NAME.clone = g.0NAME.node
  g.0TEXT.clone = g.0TEXT.node
  /* clone any attributes...*/
  aidin = g.0FIRSTATTR.node
  do while aidin <> ''
    aid = _getNextId()
    g.0TYPE.aid = g.0TYPE.aidin
    g.0NAME.aid = g.0NAME.aidin
    g.0TEXT.aid = g.0TEXT.aidin
    g.0PARENT.aid = clone
    g.0NEXT.aid = ''
    g.0PREV.aid = ''
    if g.0FIRSTATTR.clone = '' then g.0FIRSTATTR.clone = aid
    if g.0LASTATTR.clone <> ''
    then do
      lastaid = g.0LASTATTR.clone
      g.0NEXT.lastaid = aid
      g.0PREV.aid = lastaid
    end
    g.0LASTATTR.clone = aid
    aidin = g.0NEXT.aidin
  end
  /* clone any children (if deep clone was requested)...*/
  if bDeep = 1
  then do
    childin = g.0FIRST.node /* first child of node being cloned */
    do while childin <> ''
      child = cloneNode(childin,bDeep)
      g.0PARENT.child = clone
      parentsLastChild = g.0LAST.clone
      g.0NEXT.parentsLastChild = child
      g.0PREV.child = parentsLastChild
      g.0LAST.clone = child
      if g.0FIRST.clone = ''
      then g.0FIRST.clone = child
      childin = g.0NEXT.childin /* next child of node being cloned */
    end
  end
return clone