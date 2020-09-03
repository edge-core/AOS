<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl"
    exclude-result-prefixes="xd"
    xmlns:jsonx="http://www.mibx.com/jsonx"
    version="1.0">
    <xd:doc scope="stylesheet">
        <xd:desc>
            <xd:p><xd:b>Created on:</xd:b> Jan 22, 2013</xd:p>
            <xd:p><xd:b>Author:</xd:b> yehjunying</xd:p>
            <xd:p></xd:p>
        </xd:desc>
    </xd:doc>

    <xsl:include href="jsonx-to-jsonp-config.xsl"/>

    <xsl:output method="text" encoding="UTF-8"/>
    <!--<xsl:strip-space elements="*"/>-->

    <xsl:variable name="nl"><xsl:text>&#xa;</xsl:text></xsl:variable>
    <xsl:variable name="tab"><xsl:text>&#9;</xsl:text></xsl:variable>
    <xsl:variable name="quote"><xsl:text>"</xsl:text></xsl:variable>

    <xsl:template match="/">
        <xsl:value-of select="$callback"/>
        <xsl:text>({</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>});</xsl:text>
    </xsl:template>

    <xsl:template match="jsonx:object">
        <xsl:if test="./@name">
            <xsl:text>"</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>":</xsl:text>
        </xsl:if>

        <xsl:text>{</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
        <xsl:if test="position() &lt; last() - 1">
            <!-- TODO: find out why is the -1 necessary? -->
            <xsl:text>,</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template match="jsonx:array">
        <xsl:if test="@name">
            <xsl:text>"</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text>":</xsl:text>
        </xsl:if>

        <xsl:text>[</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>]</xsl:text>
        <xsl:if test="position() &lt; last() - 1">
            <!-- TODO: find out why is the -1 necessary? -->
            <xsl:text>,</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template match="jsonx:number|jsonx:boolean">
        <xsl:variable name="text">
            <xsl:call-template name="escape-linebreaks">
                <xsl:with-param name="string">
                    <xsl:call-template name="escape-quotes">
                        <xsl:with-param name="string">
                            <xsl:value-of select="text()"/>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:variable>

        <xsl:text>"</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>":</xsl:text>
        <xsl:value-of select="$text"/>
        <xsl:if test="position() &lt; last() - 1">
        <!--<xsl:if test="not(position()=last())">-->
            <xsl:text>,</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template match="jsonx:string">
        <xsl:variable name="text">
            <xsl:call-template name="escape-linebreaks">
                <xsl:with-param name="string">
                    <xsl:call-template name="escape-quotes">
                        <xsl:with-param name="string">
                            <xsl:value-of select="text()"/>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:variable>

        <xsl:text>"</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>":"</xsl:text>
        <xsl:value-of select="$text"/>
        <xsl:text>"</xsl:text>
        <xsl:if test="position() &lt; last() - 1">
            <!-- TODO: find out why is the -1 necessary? -->
            <xsl:text>,</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template name="escape-linebreaks">
        <xsl:param name="string"/>

        <xsl:call-template name="replace-all">
            <xsl:with-param name="string">
                <xsl:value-of select="$string"/>
            </xsl:with-param>
            <xsl:with-param name="old">
                <xsl:value-of select="$nl"/>
            </xsl:with-param>
            <xsl:with-param name="new">
                <xsl:text>\n</xsl:text>
            </xsl:with-param>
        </xsl:call-template>
    </xsl:template>

    <xsl:template name="escape-quotes">
        <xsl:param name="string"/>

        <xsl:call-template name="replace-all">
            <xsl:with-param name="string">
                <xsl:value-of select="$string"/>
            </xsl:with-param>
            <xsl:with-param name="old">
                <xsl:value-of select="$quote"/>
            </xsl:with-param>
            <xsl:with-param name="new">
                <xsl:text>\</xsl:text>
                <xsl:value-of select="$quote"/>
            </xsl:with-param>
        </xsl:call-template>
    </xsl:template>

    <xsl:template name="replace-all">
        <xsl:param name="string"/>
        <xsl:param name="old"/>
        <xsl:param name="new"/>

        <xsl:choose>
            <xsl:when test='contains($string, $old)'>
                <xsl:value-of select="substring-before($string, $old)" />
                <xsl:value-of select="$new"/>
                <xsl:call-template name="replace-all">
                    <xsl:with-param name="string" select="substring-after($string, $old)" />
                    <xsl:with-param name="old"    select="$old" />
                    <xsl:with-param name="new"    select="$new" />
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$string" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>