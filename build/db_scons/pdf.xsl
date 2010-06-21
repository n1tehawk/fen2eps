<?xml version='1.0'?>
<xsl:stylesheet
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:fo="http://www.w3.org/1999/XSL/Format" 
	version="1.0"> 

	<xsl:import href="file:///usr/share/docbook-xsl-1.71.1/fo/docbook.xsl"/> 
<xsl:param name="paper.size" select="A4"></xsl:param>
<xsl:param name="l10n.gentext.default.language" select="'en'"/>
<xsl:param name="section.autolabel" select="1"></xsl:param>
<xsl:param name="toc.indent.width" select="0"></xsl:param>
<xsl:param name="body.start.indent">0pt</xsl:param>
<xsl:param name="shade.verbatim" select="1"></xsl:param>
<xsl:param name="generate.toc">
/appendix toc,title
article/appendix  nop
/article  toc,title
book      toc,title,figure,table,example,equation
/chapter  toc,title
part      toc,title
/preface  toc,title
reference toc,title
/sect1    toc
/sect2    toc
/sect3    toc
/sect4    toc
/sect5    toc
/section  toc
set       toc,title
</xsl:param>

<xsl:template match="varlistentry/term">
	<xsl:call-template name="inline.boldseq"/>
</xsl:template>

<!-- create ToC on a separate page, ie insert a page break -->
<xsl:template name="component.toc.separator">
<fo:block xmlns:fo="http://www.w3.org/1999/XSL/Format" break-after="page"/>
</xsl:template>

<xsl:template match="processing-instruction('hard-pagebreak')">
   <fo:block break-after='page'/>
 </xsl:template>

</xsl:stylesheet> 

