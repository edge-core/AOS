<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:string="http://www.mibx.com/string"
    extension-element-prefixes="string"
    version="1.0">
    
    <xsl:template name="string:strcmp">
        <xsl:param name="str1"/>
        <xsl:param name="str2"/>
        <xsl:if test="normalize-space($str1) = normalize-space($str2)">
            <xsl:value-of select="1"/>
        </xsl:if>
    </xsl:template>

    <xsl:template name="string:repeat">
        <xsl:param name="string" select="''" />
        <xsl:param name="times"  select="1" />
        
        <xsl:if test="number($times) &gt; 0">
            <xsl:value-of select="$string" />
            <xsl:call-template name="string:repeat">
                <xsl:with-param name="string" select="$string" />
                <xsl:with-param name="times"  select="$times - 1" />
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <xsl:template name="string:capitalize-first">
        <xsl:param name="arg"/>
            
        <xsl:value-of select="concat(translate(substring($arg, 1, 1),'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring($arg, 2, string-length($arg)-1))"/>
    </xsl:template>
    
</xsl:stylesheet>