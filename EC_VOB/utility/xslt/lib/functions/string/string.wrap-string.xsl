<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl"
    exclude-result-prefixes="xd"
    xmlns:string="http://www.mibx.com/string"
    extension-element-prefixes="string"
    version="1.0">
    <xd:doc scope="stylesheet">
        <xd:desc>
            <xd:p><xd:b>Created on:</xd:b> Jan 21, 2013</xd:p>
            <xd:p><xd:b>Author:</xd:b> yehjunying</xd:p>
            <xd:p></xd:p>
        </xd:desc>
    </xd:doc>
    
    <!-- Copyright 2010 Aristotle Pagaltzis; under the MIT licence -->
    <!-- http://www.opensource.org/licenses/mit-license.php -->
    <xsl:template name="string:wrap-string">
        <xsl:param name="str" />
        <xsl:param name="wrap-col" />
        <xsl:param name="break-mark" />
        <xsl:param name="pos" select="0" />
        <xsl:choose>
            
            <xsl:when test="contains( $str, ' ' )">
                <xsl:variable name="first-word" select="substring-before( $str, ' ' )" />
                <xsl:variable name="pos-now" select="$pos + 1 + string-length( $first-word )" />
                <xsl:choose>
                    
                    <xsl:when test="$pos > 0 and $pos-now >= $wrap-col">
                        <xsl:copy-of select="$break-mark" />
                        <xsl:call-template name="string:wrap-string">
                            <xsl:with-param name="str" select="$str" />
                            <xsl:with-param name="wrap-col" select="$wrap-col" />
                            <xsl:with-param name="break-mark" select="$break-mark" />
                            <xsl:with-param name="pos" select="0" />
                        </xsl:call-template>
                    </xsl:when>
                    
                    <xsl:otherwise>
                        <xsl:value-of select="$first-word" />
                        <xsl:text> </xsl:text>
                        <xsl:call-template name="string:wrap-string">
                            <xsl:with-param name="str" select="substring-after( $str, ' ' )" />
                            <xsl:with-param name="wrap-col" select="$wrap-col" />
                            <xsl:with-param name="break-mark" select="$break-mark" />
                            <xsl:with-param name="pos" select="$pos-now" />
                        </xsl:call-template>
                    </xsl:otherwise>
                    
                </xsl:choose>
            </xsl:when>
            
            <xsl:otherwise>
                <xsl:if test="$pos + string-length( $str ) >= $wrap-col">
                    <xsl:copy-of select="$break-mark" />
                </xsl:if>
                <xsl:value-of select="$str" />
            </xsl:otherwise>
            
        </xsl:choose>
    </xsl:template>
    
</xsl:stylesheet>