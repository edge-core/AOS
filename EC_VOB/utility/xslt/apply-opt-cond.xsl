<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl"
    exclude-result-prefixes="xd"
    xmlns:dyn="http://www.mibx.com/dyn"
    version="1.0">
    
    <xsl:import href="lib/lib.xsl"/>
    
    <xd:doc scope="stylesheet">
        <xd:desc>
            <xd:p><xd:b>Created on:</xd:b> Jan 20, 2013</xd:p>
            <xd:p><xd:b>Author:</xd:b> yehjunying</xd:p>
            <xd:p></xd:p>
        </xd:desc>
    </xd:doc>
    
    <xsl:output method="xml" standalone="yes" indent="yes"/>
    
    <xsl:template match="*">
        <xsl:element name="{name(.)}" namespace="{namespace-uri(.)}">
            <xsl:copy-of select="@*"/>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    
    <xsl:template match="comment()">
        <xsl:copy>
            <xsl:apply-templates/>
        </xsl:copy>        
    </xsl:template>
    
    <xsl:template match="*[local-name(.)='option']">
        <xsl:variable name="test">
            <xsl:call-template name="dyn:evaluate">
                <xsl:with-param name="arg" select="@test"/>
            </xsl:call-template>
        </xsl:variable>
                
        <xsl:if test="$test='1'">
            <xsl:apply-templates/>
        </xsl:if>
    </xsl:template>
    
</xsl:stylesheet>