@Comment(-*-SCRIBE-*-)
@Comment(SCRIBE Text Formatter Input for the UNIX Kermit User Guide)

@Make<Manual>
@Comment(Use /draft:F on command line to produce .LPT file w/cc in column 1)

@case(device,postscript="@LibraryFile<Mathematics10>")

@String<Ellips="...">

@Style<Justification On, Hyphenation On, WidestBlank 1.4, Spacing 1,
        Spread 1, Indent 0, HyphenBreak Off, SingleSided>
@Use<Hyphendic="kuser.hyp">
@Modify<IndexEnv,Boxed,
	Columns 2,ColumnMargin 0.5inch,LineWidth 2.2inch,ColumnWidth 2.7inch>

@Comment(Set desired spacing around various environments)

@Modify<Quotation,Indentation 0, Above 1, Below 1, Spacing 1>
@Modify<Example, Above 1, Below 1, Blanklines Hinge>
@Modify<Verbatim, Leftmargin 0>
@Modify<Itemize, Above 1, Below 1, Spacing 1, Spread 1>
@Modify<Enumerate, Above 1, Below 1, Spacing 1, Spread 1>
@Modify<Description, Above 1, Below 1, Spacing 1>
@Define<MD,Use Display>
@Define<Q,FaceCode R>
@Define<QQ,FaceCode R,AfterEntry=["],BeforeExit=["]>
@Define<SubH,Use Display,FaceCode R,Above 1.6,Below 1>
@Define<SubU,Use UX,FaceCode R,Above 1.6,Below 1,need 6>
@define<xx,use b>
@define<yy,use i>

@Comment(Printing Device Dependencies)

@Case{Device,
 LPT="@use(Auxfile='CKLP.AUX')
    @Case[Draft,F=`@Style(CarriageControl=FORTRAN)']
    @Style(linewidth 79)
    @Define<SubH,Use Display,FaceCode B,Above 1.6,Below 1>",
 Printronix="@use(AuxFile='CKPX.AUX')
    @Case[Draft,F=`@Style(CarriageControl=FORTRAN)']
    @Style(LineWidth 74, PaperLength 11 inches,BottomMargin 5 spacings)",
 PagedFile="@use(AuxFile='CKPF.AUX')
    @Style(LineWidth 79, PaperLength 10.8 inches)
    @Style(TopMargin 3 spacings,BottomMargin 6 Spacings)
    @Modify(Example,Leftmargin +2)",
 Diablo="@Use(Auxfile='CKDIA.AUX')
    @TypeWheel(Titan 10)
    @Define<SubH,Use Display,FaceCode B,Above 1.6,Below 1>",
 Postscript="@Use(Auxfile='CKPS.AUX')
    @Style<Doublesided>
    @Style<Fontscale 10>
    @Define<MD,Use Mathdisplay>
    @String<Ellips='@Math(@Ldots)'>
    @Define<xx,use b,Size 12>
    @Define<yy,use i,Size 10>
    @Define<Q,FaceCode T>
    @Define(QQ,FaceCode T,AfterEntry=[@r<``>],BeforeExit=[@r<''>])
    @Define<SubH,Use Display,FaceCode B,Above 1.6,Below 1>
    @Modify<Quotation,Size +0>
    @Modify<Itemize,Spread 0.8>",
 Imprint10="@Use(AuxFile='CKIMP.AUX')
    @Define<Q,FaceCode U>
    @Define(QQ,FaceCode U,AfterEntry=[@r<``>],BeforeExit=[@r<''>])
    @Define<SubH,Use Display,FaceCode B,Above 1.6,Below 1>
    @define<xx,use b,font bodyfont>
    @define<yy,use i,font bodyfont>
    @Modify<Insert,Spacing 1>
    @Modify<Verbatim,FaceCode U>
    @Modify<Example,FaceCode U,spacing 1.2>
    @Modify<Itemize,Spread 0.8>
    @Style<FontFamily SmallRoman12,Spacing 1.6,SingleSided>",
 Imagen300="@Use(AuxFile='CKKIM3.AUX')
    @Define<Q,FaceCode U>
    @Define(QQ,FaceCode U,AfterEntry=[@r<``>],BeforeExit=[@r<''>])
    @Define<SubH,Use Display,FaceCode B,Above 1.6,Below 1>
    @define<xx,use b,font bodyfont>
    @define<yy,use i,font bodyfont>
    @Modify<Insert,Spacing 1>
    @Modify<Verbatim,FaceCode U>
    @Modify<Example,FaceCode U,spacing 1.2>
    @Modify<Itemize,Spread 0.8>
    @Style<FontFamily SmallRoman12,Spacing 1.6,SingleSided>",
 X9700="@Use<AuxFile='CKX9.AUX'>
    @Style<FontFamily Univers10, DoubleSided, Spacing 0.9, Spread 0.8>
    @Style<Scriptpush No>
    @define<xx,use b,font bodyfont>
    @define<yy,use i,font bodyfont>
    @Define<Q,FaceCode U>
    @Define(QQ,FaceCode U,AfterEntry=[@r<``>],BeforeExit=[@r<''>])
    @Define<SubH,Use Display,FaceCode B,Above 1.6,Below 1>
    @Modify<Description,Spacing 0.8,Spread 0.75> 
    @Modify<Quotation,Spacing 0.8,Spread 0.75> 
    @Modify<Enumerate,Spacing 0.8,Spread 0.75> 
    @Modify(Verbatim,Spacing 0.8,Spread 0.75,FaceCode U)
    @Modify<Example,FaceCode U, Spacing 1.1>
    @Modify[Itemize, Numbered < @,- >, Spacing 0.8, Spread 0.5]"
}
@Comment(Set spacing and paging requirements for chapter & section headings)

@Modify(Hdx,Above 2,Below 1,Need 8)
@Modify(Hd0,Above 2,Below 1,Need 8)
@Modify(Hd2,Above 2,Below 1,Need 8)
@Modify(Hd3,Above 2,Below 1,Need 8)
@Modify(Hd4,Above 2,Below 1,Need 8)
@Modify<Heading, Above 2, Need 8>
@Modify<Subheading, Above 1.5, Need 6>

@Pageheading<>
@Begin(TitlePage,Initialize "@BlankSpace(2.4inches)",sink 0)
@MajorHeading(C-KERMIT USER GUIDE)
@heading(For UNIX, VMS, and Many Other Operating Systems)

@b<Version 4F(085)>

C.@ Gianone, F.@ da Cruz

Columbia University Center for Computing Activities
New York, New York  10027


@i<July 14, 1989>



Copyright (C) 1981,1989
Trustees of Columbia University in the City of New York

@i<Permission is granted to any individual or institution to use, copy,
or redistribute this document so long as it is not sold for profit, and
provided this copyright notice is retained.>
@case[device, x9700="@blankpage(1)"]
@end<titlepage>
@PageHeading(Odd,Immediate,
	Left="@xx<@Value[SectionNumber]. @Value[SectionTitle]>",
	Right="@yy<Page @ref(page)>",
	Line="@bar()@blankspace(2)")
@PageHeading(Even,
	Left="@yy<Page @ref(page)>",
	Right="@xx<Kermit User Guide: @Title(Chapter) @Value[SectionNumber]>",
	Line="@bar()@blankspace(2)")
@set(page=1)
@Include<ckuker>
@PageHeading(Odd,Immediate,
	Left="@xx<C-Kermit>",
	Right="@yy<Page @ref(page)>",
	Line="@bar()@blankspace(2)")
@PageHeading(Even,
	Left="@yy<Page @ref(page)>",
	Right="@xx<Kermit User Guide>",
	Line="@bar()@blankspace(2)")
@Case(Device,x9700="@Comment<Begin Duplex Kludge>
@SendEnd(#Index `@begin<Transparent,PageBreak UntilOdd>@end<Transparent>')
@Comment<End Duplex Kludge>")
