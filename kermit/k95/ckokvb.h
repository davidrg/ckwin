/*
  Author: Jeffrey E Altman <jaltman@secure-endpoints.com>,
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/

/* DEC VT keyboard key definitions */

#define K_COMPOSE      1                /* Compose key */
#define K_C_UNI16      2                /* Compose Unicode in HEX */
#define K_C_OEM10      3                /* Compose OEM in Decimal */
#define K_C_OEM16      4                /* Compose OEM in Hex */
#define K_C_WIN16      5                /* Compose Windows in Hex */

#ifdef COMMENT
A note to save for sometime when you're caught up...

There has been a lengthy discussion on Unicode mailing list about how
to enter Unicode characters that your keyboard driver does not support,
that convinces me that we need something like this in K95 too, since
the Alt-nnn and Alt-0nnn methods are not available to us.  I'd suggest
adding several Kverbs that are similar to Kcompose, but that take
numeric codes:

  \Kc_uni16
    The next 4 keystrokes are the hex value of a Unicode character.

  \Kc_oem10
    The next 3 keystrokes are the decimal value of an OEM codepage character.

  \Kc_oem16
    The next 2 keystrokes are the hex value of an OEM codepage character.

  \Kc_win10
    The next 3 keystrokes are the decimal value of a Windows codepage character

  \Kc_win16
    The next 2 keystrokes are the hex value of a Windows codepage character.

We would have to think about under what conditions each one is valid, etc.
#endif /* COMMENT */

/* DEC arrow keys */

#define K_ARR_MIN     10
#define K_UPARR       10                /* DEC Up Arrow key */
#define K_DNARR       11                /* DEC Down Arrow key */
#define K_RTARR       12                /* DEC Right Arrow key */
#define K_LFARR       13                /* DEC Left Arrow key */
#define K_ARR_MAX     13

#define K_GOLD        15                /* DEC Gold key (same as PF1) */
#define K_PF1         15                /* DEC PF1 key */
#define K_PF2         16                /* DEC PF2 key */
#define K_PF3         17                /* DEC PF3 key */
#define K_PF4         18                /* DEC PF4 key */

#define K_KP0         20                /* DEC Keypad 0 */
#define K_KP1         21                /* DEC Keypad 1 */
#define K_KP2         22                /* etc ... through 9 */
#define K_KP3         23
#define K_KP4         24
#define K_KP5         25
#define K_KP6         26
#define K_KP7         27
#define K_KP8         28
#define K_KP9         29
#define K_KPCOMA      30                /* DEC keypad comma */
#define K_KPMINUS     31                /* DEC keypad minus */
#define K_KPDOT       32                /* DEC keypad period */
#define K_KPENTER     33                /* DEC keypad enter */

/* DEC Top-Rank F keys */

#define K_DECF1       41                /* DEC F1 key (PC Keyboard) */
#define K_DECF2       42                /* DEC F2 key (PC Keyboard) */
#define K_DECF3       43                /* DEC F3 key (PC Keyboard) */
#define K_DECF4       44                /* DEC F4 key (PC Keyboard) */
#define K_DECF5       45                /* DEC F5 key (PC Keyboard) */
#define K_DECF6       46                /* DEC F6 key */
#define K_DECF7       47                /* etc, through F20 */
#define K_DECF8       48
#define K_DECF9       49
#define K_DECF10      50
#define K_DECF11      51
#define K_DECF12      52
#define K_DECF13      53
#define K_DECF14      54
#define K_DECF15      55
#define K_DECHELP     55                /* DEC Help key */
#define K_DECF16      56
#define K_DECDO       56                /* DEC Do key */
#define K_DECF17      57
#define K_DECF18      58
#define K_DECF19      59
#define K_DECF20      60

/* DEC editing keys */

#define K_DECFIND     71                /* DEC Find key */
#define K_DECINSERT   72                /* DEC Insert key */
#define K_DECREMOVE   73                /* DEC Remove key */
#define K_DECSELECT   74                /* DEC Select key */
#define K_DECPREV     75                /* DEC Previous Screen key */
#define K_DECNEXT     76                /* DEC Next Screen key */
#define K_DECHOME     77                /* DEC PC Home key */

/* DEC User-Defined Keys */

#define K_UDKF1       81                /* F1 - F5 are XTERM extensions */
#define K_UDKF2       82
#define K_UDKF3       83
#define K_UDKF4       84
#define K_UDKF5       85
#define K_UDKF6       86                /* DEC User Defined Key F6 */
#define K_UDKF7       87                /* DEC User Defined Key F7 */
#define K_UDKF8       88                /* etc ... through F20 */
#define K_UDKF9       89
#define K_UDKF10      90
#define K_UDKF11      91
#define K_UDKF12      92
#define K_UDKF13      93
#define K_UDKF14      94
#define K_UDKF15      95
#define K_UDKHELP     95
#define K_UDKF16      96
#define K_UDKDO       96
#define K_UDKF17      97
#define K_UDKF18      98
#define K_UDKF19      99
#define K_UDKF20     100

/* Emacs Keys */
#define K_EMACS_OVER 105

/* Kermit screen-scrolling keys */

#define K_ROLLMIN    110
#define K_DNONE      110                /* Screen rollback: down one line */
#define K_DNSCN      111                /* Screen rollback: down one screen */
#define K_UPONE      112                /* Screen rollback: Up one line */
#define K_UPSCN      113                /* Screen rollback: Up one screen */
#define K_ENDSCN     114                /* Screen rollback: latest screen */
#define K_HOMSCN     115                /* Screen rollback: oldest screen */
#define K_GO_BOOK    116        /* Scroll to bookmark */
#define K_GOTO       117        /* Scroll to line number */
#define K_ROLLMAX    117

#define K_LFONE      130                /* Horizontal Scroll: Left one cell */
#define K_LFPAGE     131                /* Horizontal Scroll: Left one page */
#define K_LFALL      132
#define K_RTONE      133                /* Horizontal Scroll: Right one cell */
#define K_RTPAGE     134                /* Horizontal Scroll: Right one page */
#define K_RTALL      135

/* Keyboard language switching verbs */

#define K_KB_ENG     140                /* English keyboard mode */
#define K_KB_HEB     141                /* Hebrew keyboard mode */
#define K_KB_RUS     142                /* Russian keyboard mode */
#define K_KB_EMA     143                /* Emacs keyboard mode */
#define K_KB_WP      144                /* Word Perfect 5.1 mode */

/* Mark Mode actions */

#define K_MARK_MIN        180
#define K_MARK_START      180   /* Enter Mark Mode/Start marking */
#define K_MARK_CANCEL     181   /* Exit Mark Mode - Do Nothing */
#define K_MARK_COPYCLIP   182   /* Exit Mark Mode - Copy data to clipboard */
#define K_MARK_COPYHOST   183   /* Exit Mark Mode - Copy data to host   */
#define K_MARK_COPYCLIP_NOEOL   184   /* Exit Mark Mode - Copy data to clipboard */
#define K_MARK_COPYHOST_NOEOL   185   /* Exit Mark Mode - Copy data to host   */
#define K_MARK_SELECT_NOEOL     186   /* Exit Mark Mode - Select */
#define K_MARK_SELECT     187   /* Exit Mark Mode - Select */
#define K_BACKSRCH        188        /* Search Backwards for text */
#define K_FWDSRCH         189        /* Search Forwards for text */
#define K_BACKNEXT        190        /* Search Backwards for next instance of text */
#define K_FWDNEXT         191        /* Search Forwards for next instance of text */
#define K_MARK_MAX        191

/* Miscellaneous Kermit actions */

#define K_ACT_MIN    200                /* Lowest Action verb number */
#define K_EXIT       200                /* Return to command parser */
#define K_BREAK      201                /* Send a BREAK */
#define K_RESET      202                /* Reset emulator */
#define K_DOS        203                /* Push to DOS (i.e. OS/2) */
#define K_HANGUP     204                /* Hang up the connection */
#define K_DUMP       205                /* Dump/Print current screen */
#define K_LBREAK     206                /* Send a Long BREAK */
#define K_NULL       207                /* Send a NUL */
#define K_HELP       208                /* Pop-up help */
#define K_HOLDSCRN   209                /* Hold screen */
#define K_IGNORE     210                /* Ignore this key, don't even beep */
#define K_LOGOFF     211                /* Turn off session logging */
#define K_LOGON      212                /* Turn on session logging */
#define K_MODELINE   213                /* Toggle mode line */
#define K_NETHOLD    214                /* Put network connection on hold */
#define K_NEXTSESS   215                /* Toggle to next network session */
#define K_STATUS     216                /* Show status */
#define K_TERMTYPE   217                /* Toggle term type: text/graphics */
#define K_PRTCTRL    218                /* Print Controller mode */
#define K_PRINTFF    219                /* Print formfeed */
#define K_FLIPSCN    220                /* Flip screen */
#define K_DEBUG      221                /* Toggle debugging */
#define K_PASTE      222                /* Paste data from clipboard */
#define K_CLRSCRN    223                /* Clear Terminal Screen */
#define K_PRTAUTO    224                /* Print Auto mode */
#define K_ANSWERBACK 225                /* Transmit Answerback String */
#define K_SET_BOOK   226                /* Set Bookmark */
#define K_QUIT       227                /* Quit Kermit */
#define K_KEYCLICK   228                /* Toggle Keyclick */
#define K_LOGDEBUG   229                /* Toggle Debug Log File */
#define K_FNKEYS     230                /* Show Function Key Labels */
#define K_PRTCOPY    231                /* Print Copy mode */
#define K_TN_AYT     232                /* TELNET Are You There */
#define K_TN_IP      233                /* TELNET Interrupt Process */
#define K_TN_AO      234                /* Telnet Abort Output */
#define K_TN_GA      235                /* Telnet Go Ahead */
#define K_TN_EC      236                /* Telnet Erase Character */
#define K_TN_EL      237                /* Telnet Erase Line */
#define K_TN_NAWS    238                /* Telnet Send Window Size */
#define K_TN_LOGOUT  239                /* Telnet Logout */
#define K_TN_SAK     240                /* Telnet Secure Access Key */
#define K_BYTESIZE   241                /* Toggle Bytesize 7/8 */
#define K_AUTODOWN   242                /* Toggle Autodownload on/off */
#define K_SESSION    243                /* Toggle Session Log on/off */
#define K_CURSOR_URL 244                /* Screen Cursor URL */
#define K_CLRSCROLL  245                /* Clear Scrollback */
#define K_URLHILT    246                /* Toggle URL Highlight */
#define K_PCTERM     247                /* Toggle PCTERM (invisible) */
#define K_ACT_MAX    247                /* Highest Action verb number */

#ifdef OS2MOUSE
/* Mouse only Kverbs */

#define K_MOUSE_MIN           300        /* Lowest Mouse verb number */
#define K_MOUSE_CURPOS        300
#define K_MOUSE_MARK          301
#define K_MOUSE_URL           302
#define K_MOUSE_MAX           302        /* Highest Mouse verb number */
#endif /* OS2MOUSE */

/* ANSI Function Key definitions */
#define K_ANSIF01               401
#define K_ANSIF02               402
#define K_ANSIF03               403
#define K_ANSIF04               404
#define K_ANSIF05               405
#define K_ANSIF06               406
#define K_ANSIF07               407
#define K_ANSIF08               408
#define K_ANSIF09               409
#define K_ANSIF10               410
#define K_ANSIF11               411
#define K_ANSIF12               412
#define K_ANSIF13               413
#define K_ANSIF14               414
#define K_ANSIF15               415
#define K_ANSIF16               416
#define K_ANSIF17               417
#define K_ANSIF18               418
#define K_ANSIF19               419
#define K_ANSIF20               420
#define K_ANSIF21               421
#define K_ANSIF22               422
#define K_ANSIF23               423
#define K_ANSIF24               424
#define K_ANSIF25               425
#define K_ANSIF26               426
#define K_ANSIF27               427
#define K_ANSIF28               428
#define K_ANSIF29               429
#define K_ANSIF30               430
#define K_ANSIF31               431
#define K_ANSIF32               432
#define K_ANSIF33               433
#define K_ANSIF34               434
#define K_ANSIF35               435
#define K_ANSIF36               436
#define K_ANSIF37               437
#define K_ANSIF38               438
#define K_ANSIF39               439
#define K_ANSIF40               440
#define K_ANSIF41               441
#define K_ANSIF42               442
#define K_ANSIF43               443
#define K_ANSIF44               444
#define K_ANSIF45               445
#define K_ANSIF46               446
#define K_ANSIF47               447
#define K_ANSIF48               448
#define K_ANSIF49               449
#define K_ANSIF50               450
#define K_ANSIF51               451
#define K_ANSIF52               452
#define K_ANSIF53               453
#define K_ANSIF54               454
#define K_ANSIF55               455
#define K_ANSIF56               456
#define K_ANSIF57               457
#define K_ANSIF58               458
#define K_ANSIF59               459
#define K_ANSIF60               460
#define K_ANSIF61               461

/* Note: If you are going to add definitions to Wyse   */
/* be sure to update the wysedefkey() switch statement */

/* WYSE Function Keys (unshifted) */
#define K_WYF01                 500
#define K_WYF02                 501
#define K_WYF03                 502
#define K_WYF04                 503
#define K_WYF05                 504
#define K_WYF06                 505
#define K_WYF07                 506
#define K_WYF08                 507
#define K_WYF09                 508
#define K_WYF10                 509
#define K_WYF11                 510
#define K_WYF12                 511
#define K_WYF13                 512
#define K_WYF14                 513
#define K_WYF15                 514
#define K_WYF16                 515
#define K_WYF17                 516
#define K_WYF18                 517
#define K_WYF19                 518
#define K_WYF20                 519

/* WYSE Function Keys (shifted) */
#define K_WYSF01                520
#define K_WYSF02                521
#define K_WYSF03                522
#define K_WYSF04                523
#define K_WYSF05                524
#define K_WYSF06                525
#define K_WYSF07                526
#define K_WYSF08                527
#define K_WYSF09                528
#define K_WYSF10                529
#define K_WYSF11                530
#define K_WYSF12                531
#define K_WYSF13                532
#define K_WYSF14                533
#define K_WYSF15                534
#define K_WYSF16                535
#define K_WYSF17                536
#define K_WYSF18                537
#define K_WYSF19                538
#define K_WYSF20                539

/* WYSE Edit and Special Keys */
#define K_WYBS                  540
#define K_WYCLRLN               541
#define K_WYSCLRLN              542
#define K_WYCLRPG               543
#define K_WYSCLRPG              544
#define K_WYDELCHAR             545
#define K_WYDELLN               546
#define K_WYENTER               547
#define K_WYESC                 548
#define K_WYHOME                549
#define K_WYSHOME               550
#define K_WYINSERT              551
#define K_WYINSCHAR             552
#define K_WYINSLN               553
#define K_WYPGNEXT              554
#define K_WYPGPREV              555
#define K_WYREPLACE             556
#define K_WYRETURN              557
#define K_WYTAB                 558
#define K_WYSTAB                559
#define K_WYPRTSCN              560
#define K_WYSESC                561
#define K_WYSBS                 562
#define K_WYSENTER              563
#define K_WYSRETURN             564
#define K_WYUPARR               565
#define K_WYDNARR               566
#define K_WYLFARR               567
#define K_WYRTARR               568
#define K_WYSUPARR              569
#define K_WYSDNARR              570
#define K_WYSLFARR              571
#define K_WYSRTARR              572
#define K_WYSEND                573
#define K_WYSSEND               574

/* Data General Function Keys (unshifted) */
#define K_DGF01                 576
#define K_DGF02                 577
#define K_DGF03                 578
#define K_DGF04                 579
#define K_DGF05                 580
#define K_DGF06                 581
#define K_DGF07                 582
#define K_DGF08                 583
#define K_DGF09                 584
#define K_DGF10                 585
#define K_DGF11                 586
#define K_DGF12                 587
#define K_DGF13                 588
#define K_DGF14                 589
#define K_DGF15                 590

/* Data General Function Keys (shifted) */
#define K_DGSF01                591
#define K_DGSF02                592
#define K_DGSF03                593
#define K_DGSF04                594
#define K_DGSF05                595
#define K_DGSF06                596
#define K_DGSF07                597
#define K_DGSF08                598
#define K_DGSF09                599
#define K_DGSF10                600
#define K_DGSF11                601
#define K_DGSF12                602
#define K_DGSF13                603
#define K_DGSF14                604
#define K_DGSF15                605

/* Data General Function Keys (control) */
#define K_DGCF01                606
#define K_DGCF02                607
#define K_DGCF03                608
#define K_DGCF04                609
#define K_DGCF05                610
#define K_DGCF06                611
#define K_DGCF07                612
#define K_DGCF08                613
#define K_DGCF09                614
#define K_DGCF10                615
#define K_DGCF11                616
#define K_DGCF12                617
#define K_DGCF13                618
#define K_DGCF14                619
#define K_DGCF15                620

/* Data General Function Keys (control shifted) */
#define K_DGCSF01               621
#define K_DGCSF02               622
#define K_DGCSF03               623
#define K_DGCSF04               624
#define K_DGCSF05               625
#define K_DGCSF06               626
#define K_DGCSF07               627
#define K_DGCSF08               628
#define K_DGCSF09               629
#define K_DGCSF10               630
#define K_DGCSF11               631
#define K_DGCSF12               632
#define K_DGCSF13               633
#define K_DGCSF14               634
#define K_DGCSF15               635

#define K_DGUPARR               636
#define K_DGDNARR               637
#define K_DGLFARR               638
#define K_DGRTARR               639
#define K_DGSUPARR              640
#define K_DGSDNARR              641
#define K_DGSLFARR              642
#define K_DGSRTARR              643

#define K_DGERASEPAGE           644
#define K_DGC1                  645
#define K_DGC2                  646
#define K_DGERASEEOL            647
#define K_DGC3                  648
#define K_DGC4                  649
#define K_DGCMDPRINT            650
#define K_DGHOME                651
#define K_DGSERASEPAGE          652
#define K_DGSC1                 653
#define K_DGSC2                 654
#define K_DGSERASEEOL           655
#define K_DGSC3                 656
#define K_DGSC4                 657
#define K_DGSCMDPRINT           658
#define K_DGBS                  659
#define K_DGSHOME               660


/* Televideo Function Keys (unshifted) */
#define K_TVIF01                661
#define K_TVIF02                662
#define K_TVIF03                663
#define K_TVIF04                664
#define K_TVIF05                665
#define K_TVIF06                666
#define K_TVIF07                667
#define K_TVIF08                668
#define K_TVIF09                669
#define K_TVIF10                670
#define K_TVIF11                671
#define K_TVIF12                672
#define K_TVIF13                673
#define K_TVIF14                674
#define K_TVIF15                675
#define K_TVIF16                676

/* Televideo Function Keys (shifted) */
#define K_TVISF01               677
#define K_TVISF02               678
#define K_TVISF03               679
#define K_TVISF04               680
#define K_TVISF05               681
#define K_TVISF06               682
#define K_TVISF07               683
#define K_TVISF08               684
#define K_TVISF09               685
#define K_TVISF10               686
#define K_TVISF11               687
#define K_TVISF12               688
#define K_TVISF13               689
#define K_TVISF14               690
#define K_TVISF15               691
#define K_TVISF16               692

/* Televideo Edit and Special Keys */
#define K_TVIBS                 693
#define K_TVICLRLN              694
#define K_TVISCLRLN             695
#define K_TVICLRPG              696
#define K_TVISCLRPG             697
#define K_TVIDELCHAR            698
#define K_TVIDELLN              699
#define K_TVIENTER              700
#define K_TVIESC                701
#define K_TVIHOME               702
#define K_TVISHOME              703
#define K_TVIINSERT             704
#define K_TVIINSCHAR            705
#define K_TVIINSLN              706
#define K_TVIPGNEXT             707
#define K_TVIPGPREV             708
#define K_TVIREPLACE            709
#define K_TVIRETURN             710
#define K_TVITAB                711
#define K_TVISTAB               712
#define K_TVIPRTSCN             713
#define K_TVISESC               714
#define K_TVISBS                715
#define K_TVISENTER             716
#define K_TVISRETURN            717
#define K_TVIUPARR              718
#define K_TVIDNARR              719
#define K_TVILFARR              720
#define K_TVIRTARR              721
#define K_TVISUPARR             722
#define K_TVISDNARR             723
#define K_TVISLFARR             724
#define K_TVISRTARR             725
#define K_TVISEND               726
#define K_TVISSEND              727

/* HP Function and Edit keys */
#define K_HPF01                 730
#define K_HPF02                 731
#define K_HPF03                 732
#define K_HPF04                 733
#define K_HPF05                 734
#define K_HPF06                 735
#define K_HPF07                 736
#define K_HPF08                 737
#define K_HPF09                 738
#define K_HPF10                 739
#define K_HPF11                 740
#define K_HPF12                 741
#define K_HPF13                 742
#define K_HPF14                 743
#define K_HPF15                 744
#define K_HPF16                 745
#define K_HPBACKTAB             746
#define K_HPRETURN              747
#define K_HPENTER               748

/* SNI Function and Edit Keys */
#define K_SNI_MIN               801
#define K_SNI_F01               801
#define K_SNI_F02               802
#define K_SNI_F03               803
#define K_SNI_F04               804
#define K_SNI_F05               805
#define K_SNI_F06               806
#define K_SNI_F07               807
#define K_SNI_F08               808
#define K_SNI_F09               809
#define K_SNI_F10               810
#define K_SNI_F11               811
#define K_SNI_F12               812
#define K_SNI_F13               813
#define K_SNI_F14               814
#define K_SNI_F15               815
#define K_SNI_F16               816
#define K_SNI_F17               817
#define K_SNI_F18               818
#define K_SNI_F19               819
#define K_SNI_F20               820
#define K_SNI_F21               821
#define K_SNI_F22               822
#define K_SNI_MODE              823
#define K_SNI_PRINT             824
#define K_SNI_TAB               825
#define K_SNI_LEFT_TAB          826
#define K_SNI_CE                827
#define K_SNI_HELP              828
#define K_SNI_START             829
#define K_SNI_END               830
#define K_SNI_SCROLL_UP         831
#define K_SNI_SCROLL_DOWN       832
#define K_SNI_HOME              833
#define K_SNI_DELETE_CHAR       834
#define K_SNI_CURSOR_UP         835
#define K_SNI_DELETE_WORD       836
#define K_SNI_CURSOR_LEFT       837
#define K_SNI_PAGE              838
#define K_SNI_CURSOR_RIGHT      839
#define K_SNI_DELETE_LINE       840
#define K_SNI_CURSOR_DOWN       841
#define K_SNI_DOUBLE_0          842
#define K_SNI_COMPOSE           843
#define K_SNI_ENDMARKE          844
#define K_SNI_INSERT_CHAR       845
#define K_SNI_INSERT_WORD       846
#define K_SNI_INSERT_LINE       847
#define K_SNI_USER1             848
#define K_SNI_USER2             849
#define K_SNI_USER3             850
#define K_SNI_USER4             851
#define K_SNI_USER5             852
#define K_SNI_S_F01             853
#define K_SNI_S_F02             854
#define K_SNI_S_F03             855
#define K_SNI_S_F04             856
#define K_SNI_S_F05             857
#define K_SNI_S_F06             858
#define K_SNI_S_F07             859
#define K_SNI_S_F08             860
#define K_SNI_S_F09             861
#define K_SNI_S_F10             862
#define K_SNI_S_F11             863
#define K_SNI_S_F12             864
#define K_SNI_S_F13             865
#define K_SNI_S_F14             866
#define K_SNI_S_F15             867
#define K_SNI_S_F16             868
#define K_SNI_S_F17             869
#define K_SNI_S_F18             870
#define K_SNI_S_F19             871
#define K_SNI_S_F20             872
#define K_SNI_S_F21             873
#define K_SNI_S_F22             874
#define K_SNI_S_MODE            875
#define K_SNI_S_PRINT           876
#define K_SNI_S_TAB             877
#define K_SNI_S_LEFT_TAB        878
#define K_SNI_S_CE              879
#define K_SNI_S_HELP            880
#define K_SNI_S_START           881
#define K_SNI_S_END             882
#define K_SNI_S_SCROLL_UP       883
#define K_SNI_S_SCROLL_DOWN     884
#define K_SNI_S_HOME            885
#define K_SNI_S_DELETE_CHAR     886
#define K_SNI_S_CURSOR_UP       887
#define K_SNI_S_DELETE_WORD     888
#define K_SNI_S_CURSOR_LEFT     889
#define K_SNI_S_PAGE            890
#define K_SNI_S_CURSOR_RIGHT    891
#define K_SNI_S_DELETE_LINE     892
#define K_SNI_S_CURSOR_DOWN     893
#define K_SNI_S_DOUBLE_0        894
#define K_SNI_S_COMPOSE         895
#define K_SNI_S_ENDMARKE        896
#define K_SNI_S_INSERT_CHAR     897
#define K_SNI_S_INSERT_WORD     898
#define K_SNI_S_INSERT_LINE     899
#define K_SNI_S_USER1           900
#define K_SNI_S_USER2           901
#define K_SNI_S_USER3           902
#define K_SNI_S_USER4           903
#define K_SNI_S_USER5           904
#define K_SNI_C_F01             905
#define K_SNI_C_F02             906
#define K_SNI_C_F03             907
#define K_SNI_C_F04             908
#define K_SNI_C_F05             909
#define K_SNI_C_F06             910
#define K_SNI_C_F07             911
#define K_SNI_C_F08             912
#define K_SNI_C_F09             913
#define K_SNI_C_F10             914
#define K_SNI_C_F11             915
#define K_SNI_C_F12             916
#define K_SNI_C_F13             917
#define K_SNI_C_F14             918
#define K_SNI_C_F15             919
#define K_SNI_C_F16             920
#define K_SNI_C_F17             921
#define K_SNI_C_F18             922
#define K_SNI_C_F19             923
#define K_SNI_C_F20             924
#define K_SNI_C_F21             925
#define K_SNI_C_F22             926
#define K_SNI_C_MODE            927
#define K_SNI_C_PRINT           928
#define K_SNI_C_TAB             929
#define K_SNI_C_LEFT_TAB        930
#define K_SNI_C_CE              931
#define K_SNI_C_HELP            932
#define K_SNI_C_START           933
#define K_SNI_C_END             934
#define K_SNI_C_SCROLL_UP       935
#define K_SNI_C_SCROLL_DOWN     936
#define K_SNI_C_HOME            937
#define K_SNI_C_DELETE_CHAR     938
#define K_SNI_C_CURSOR_UP       939
#define K_SNI_C_DELETE_WORD     940
#define K_SNI_C_CURSOR_LEFT     941
#define K_SNI_C_PAGE            942
#define K_SNI_C_CURSOR_RIGHT    943
#define K_SNI_C_DELETE_LINE     944
#define K_SNI_C_CURSOR_DOWN     945
#define K_SNI_C_DOUBLE_0        946
#define K_SNI_C_COMPOSE         947
#define K_SNI_C_ENDMARKE        948
#define K_SNI_C_INSERT_CHAR     949
#define K_SNI_C_INSERT_WORD     950
#define K_SNI_C_INSERT_LINE     951
#define K_SNI_C_USER1           952
#define K_SNI_C_USER2           953
#define K_SNI_C_USER3           954
#define K_SNI_C_USER4           955
#define K_SNI_C_USER5           956
#define K_SNI_CH_CODE           957
#define K_SNI_MAX               K_SNI_CH_CODE

/* Nixdorf BA-80 keyboard verbs */
#define K_BA80_MIN             1000
#define K_BA80_SOFTKEY1        1000
#define K_BA80_SOFTKEY2        1001
#define K_BA80_SOFTKEY3        1002
#define K_BA80_SOFTKEY4        1003
#define K_BA80_SOFTKEY5        1004
#define K_BA80_SOFTKEY6        1005
#define K_BA80_SOFTKEY7        1006
#define K_BA80_SOFTKEY8        1007
#define K_BA80_SOFTKEY9        1008
#define K_BA80_PA01            1009
#define K_BA80_PA02            1010
#define K_BA80_PA03            1011
#define K_BA80_PA04            1012
#define K_BA80_PA05            1013
#define K_BA80_PA06            1014
#define K_BA80_PA07            1015
#define K_BA80_PA08            1016
#define K_BA80_PA09            1017
#define K_BA80_PA10            1018
#define K_BA80_PA11            1019
#define K_BA80_PA12            1020
#define K_BA80_PA13            1021
#define K_BA80_PA14            1022
#define K_BA80_PA15            1023
#define K_BA80_PA16            1024
#define K_BA80_PA17            1025
#define K_BA80_PA18            1026
#define K_BA80_PA19            1027
#define K_BA80_PA20            1028
#define K_BA80_PA21            1029
#define K_BA80_PA22            1030
#define K_BA80_PA23            1031
#define K_BA80_PA24            1032
#define K_BA80_HELP            1033
#define K_BA80_END             1034
#define K_BA80_INS             1035
#define K_BA80_DEL             1036
#define K_BA80_INS_B           1037
#define K_BA80_DEL_B           1038
#define K_BA80_ATTR            1039
#define K_BA80_SAVE            1040
#define K_BA80_C_KEY           1041
#define K_BA80_RUBOUT          1042
#define K_BA80_DO              1043
#define K_BA80_PRINT           1044
#define K_BA80_HOME            1045
#define K_BA80_UNDO            1046
#define K_BA80_PICK            1047
#define K_BA80_CMD             1048
#define K_BA80_PGUP            1049
#define K_BA80_PGDN            1050
#define K_BA80_RESET           1051
#define K_BA80_PUT             1052
#define K_BA80_MARK            1053
#define K_BA80_REFRESH         1054
#define K_BA80_CLEAR           1055
#define K_BA80_EOP             1056
#define K_BA80_ENV             1057
#define K_BA80_FMT             1058
#define K_BA80_ERASE           1059
#define K_BA80_MOVE            1060
#define K_BA80_COPY            1061
#define K_BA80_MAX             K_BA80_COPY

/* IBM 31xx Emulation */
#define K_I31_MIN               1100
#define K_I31_F01               1100
#define K_I31_F02               1101
#define K_I31_F03               1102
#define K_I31_F04               1103
#define K_I31_F05               1104
#define K_I31_F06               1105
#define K_I31_F07               1106
#define K_I31_F08               1107
#define K_I31_F09               1108
#define K_I31_F10               1109
#define K_I31_F11               1110
#define K_I31_F12               1111
#define K_I31_F13               1112
#define K_I31_F14               1113
#define K_I31_F15               1114
#define K_I31_F16               1115
#define K_I31_F17               1116
#define K_I31_F18               1117
#define K_I31_F19               1118
#define K_I31_F20               1119
#define K_I31_F21               1120
#define K_I31_F22               1121
#define K_I31_F23               1122
#define K_I31_F24               1123
#define K_I31_F25               1124
#define K_I31_F26               1125
#define K_I31_F27               1126
#define K_I31_F28               1127
#define K_I31_F29               1128
#define K_I31_F30               1129
#define K_I31_F31               1130
#define K_I31_F32               1131
#define K_I31_F33               1132
#define K_I31_F34               1133
#define K_I31_F35               1134
#define K_I31_F36               1135
#define K_I31_PA1               1136
#define K_I31_PA2               1137
#define K_I31_PA3               1138
#define K_I31_RESET             1139
#define K_I31_JUMP              1140
#define K_I31_CLEAR             1141
#define K_I31_ERASE_EOF         1142
#define K_I31_ERASE_EOP         1143
#define K_I31_ERASE_INP         1144
#define K_I31_INSERT_CHAR       1145
#define K_I31_INSERT_SPACE      1146
#define K_I31_DELETE            1147
#define K_I31_INS_LN            1148
#define K_I31_DEL_LN            1149
#define K_I31_PRINT_LINE        1150
#define K_I31_PRINT_MSG         1151
#define K_I31_PRINT_SHIFT       1152
#define K_I31_CANCEL            1153
#define K_I31_SEND_LINE         1154
#define K_I31_SEND_MSG          1155
#define K_I31_SEND_PAGE         1156
#define K_I31_HOME              1157
#define K_I31_BACK_TAB          1158
#define K_I31_MAX               K_I31_BACK_TAB

#define K_SUN_MIN               1200
#define K_SUN_STOP              1200
#define K_SUN_AGAIN             1201
#define K_SUN_PROPS             1202
#define K_SUN_UNDO              1203
#define K_SUN_FRONT             1204
#define K_SUN_COPY              1205
#define K_SUN_OPEN              1206
#define K_SUN_PASTE             1207
#define K_SUN_FIND              1208
#define K_SUN_CUT               1209
#define K_SUN_HELP              1210
#define K_SUN_MAX               K_SUN_HELP
