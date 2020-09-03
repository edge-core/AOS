<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl"
    exclude-result-prefixes="xd"
    xmlns:string="http://www.mibx.com/string"
    extension-element-prefixes="string"
    version="1.0">
    
    <xsl:import href="string.comm.xsl"/>
    <xsl:import href="string.wrap-string.xsl"/>
    
    <xd:doc scope="stylesheet">
        <xd:desc>
            <xd:p><xd:b>Created on:</xd:b> Jan 18, 2013</xd:p>
            <xd:p><xd:b>Author:</xd:b> yehjunying</xd:p>
            <xd:p></xd:p>
        </xd:desc>
    </xd:doc>
    
    <xsl:template name="string:petty-print">
        <xsl:param name="text" select="''"/>
        <xsl:param name="indent" select="0"/>
        <xsl:param name="line-wrap" select="false()"/>
        <xsl:param name="wrap-col" select="80"/>
        <xsl:param name="show-quote" select="false()"/>
        <xsl:param name="preserve-white-space" select="false()"/>
        <xsl:param name="cr">
            <xsl:text>&#xA;</xsl:text>
        </xsl:param>
        <xsl:param name="_first-line" select="true()"/>
        <xsl:param name="_show-lead-quote" select="false()"/>
        
        <xsl:variable name="max-col" select="9999"/>
        
        <xsl:choose>
            <xsl:when test="contains($text, $cr)">
                
                <xsl:if test="$_first-line=false()">
                    <xsl:call-template name="string:repeat">
                        <xsl:with-param name="string" select="' '"/>
                        <xsl:with-param name="times">
                            <xsl:choose>
                                <xsl:when test="$show-quote=true()">
                                    <xsl:value-of select="$indent + 1"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:value-of select="$indent"/>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:if>
                
                <xsl:if test="$show-quote=true() and $_first-line=true()">
                    <xsl:text>"</xsl:text>
                </xsl:if>
                
                <xsl:choose>
                    <xsl:when test="$preserve-white-space=true()">
                        <xsl:value-of select="substring-before($text, $cr)"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="normalize-space(substring-before($text, $cr))"/>
                    </xsl:otherwise>
                </xsl:choose>
                
                <xsl:value-of select="$cr"/>
                
                <xsl:call-template name="string:petty-print">
                    <xsl:with-param name="text" select="substring-after($text, $cr)"/>
                    <xsl:with-param name="indent" select="$indent"/>
                    <xsl:with-param name="show-quote" select="$show-quote"/>
                    <xsl:with-param name="preserve-white-space" select="$preserve-white-space"/>
                    <xsl:with-param name="cr" select="$cr"/>
                    <xsl:with-param name="_first-line" select="false()"/>
                </xsl:call-template>
            </xsl:when>
            
            <xsl:otherwise>
                <xsl:if test="$_first-line=false()">
                    <xsl:call-template name="string:repeat">
                        <xsl:with-param name="string" select="' '"/>
                        <xsl:with-param name="times">
                            <xsl:choose>
                                <xsl:when test="$show-quote=true()">
                                    <xsl:value-of select="$indent + 1"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:value-of select="$indent"/>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:if>
                
                <xsl:if test="$show-quote=true() and $_first-line=true()">
                    <xsl:text>"</xsl:text>
                </xsl:if>
                
                <xsl:choose>
                    <xsl:when test="$preserve-white-space=true()">
                        <xsl:value-of select="$text"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="normalize-space($text)"/>
                    </xsl:otherwise>
                </xsl:choose>
                
                <xsl:if test="$show-quote=true()">
                    <xsl:text>"</xsl:text>
                </xsl:if>
            </xsl:otherwise>
            
        </xsl:choose>
        
        <!--<xsl:choose>
            <xsl:when test="$line-wrap=true()">
                <xsl:variable name ="line">
                    <xsl:call-template name="string:wrap-string">
                        <xsl:with-param name="str">
                            <xsl:choose>
                                <xsl:when test="contains($text, $cr) and $preserve-white-space=false()">
                                    <xsl:value-of select="normalize-space(substring-before($text, $cr))"/>
                                </xsl:when>
                                
                                <xsl:when test="not(contains($text, $cr)) and $preserve-white-space=false()">
                                    <xsl:value-of select="normalize-space($text)"/>
                                </xsl:when>
                                
                                <xsl:when test="contains($text, $cr) and $preserve-white-space=true()">
                                    <xsl:value-of select="substring-before($text, $cr)"/>
                                </xsl:when>
                                
                                <xsl:otherwise>
                                    <xsl:value-of select="$text"/>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:with-param>
                        <xsl:with-param name="wrap-col">
                            <xsl:value-of select="number($wrap-col - $indent)"/>
                        </xsl:with-param>
                        <xsl:with-param name="break-mark" select="$cr"/>
                    </xsl:call-template>
                </xsl:variable>
                
                <xsl:choose>
                    <xsl:when test="contains($line, $cr)">
                        
                        <xsl:call-template name="string:petty-print">
                            <xsl:with-param name="text" select="concat($line, concat($cr, substring-after($text, $cr)))"/>
                            <xsl:with-param name="indent" select="$indent"/>
                            <xsl:with-param name="line-wrap" select="$line-wrap"/>
                            <xsl:with-param name="wrap-col" select="$wrap-col"/>
                            <xsl:with-param name="show-quote" select="$show-quote"/>
                            <xsl:with-param name="cr" select="$cr"/>
                            <xsl:with-param name="_first-line" select="$_first-line"/>
                            <xsl:with-param name="_show-lead-quote" select="$_show-lead-quote"/>
                        </xsl:call-template>
                        
                    </xsl:when>
                    
                    <xsl:otherwise>
                        <xsl:call-template name="string:repeat">
                            <xsl:with-param name="string" select="' '"/>
                            <xsl:with-param name="times" select="$indent"/>
                        </xsl:call-template>
                        
                        <xsl:if test="$_show-lead-quote=false()">
                            <xsl:if test="$show-quote=true() and $_first-line=true()">
                                <xsl:text>"</xsl:text>
                            </xsl:if>
                        </xsl:if>
                        
                        <xsl:choose>
                            <!-\- Last line or 
                                 Contains no child nodes or 
                                 Contains no text content
                            -\->
                            <!-\-<xsl:when test="not(substring-after($text, concat($line, $cr))) or not($text) or not(string($text))">-\->
                            <xsl:when test="not(substring-after($text, concat($line, $cr)))">
                                <!-\-<xsl:value-of select="$text"/>-\->
                                <xsl:value-of select="$line"/>
                                <xsl:if test="$show-quote=true()">
                                    <xsl:text>"</xsl:text>
                                </xsl:if>
                            </xsl:when>
                            
                            <xsl:otherwise>
                                
                                <xsl:value-of select="$line"/>
                                                                    
                                <xsl:value-of select="$cr"/>
                                
                                <xsl:call-template name="string:petty-print">
                                    <xsl:with-param name="text" select="substring-after($text, concat($line, $cr))"/>
                                    <xsl:with-param name="indent">
                                        <xsl:choose>
                                            <xsl:when test="$show-quote=true() and $_first-line=true()">
                                                <xsl:value-of select="($indent + 1)"/>
                                            </xsl:when>
                                            <xsl:otherwise>
                                                <xsl:value-of select="$indent"/>
                                            </xsl:otherwise>
                                        </xsl:choose>
                                    </xsl:with-param>
                                    <xsl:with-param name="line-wrap" select="$line-wrap"/>
                                    <xsl:with-param name="wrap-col" select="$wrap-col"/>
                                    <xsl:with-param name="show-quote" select="$show-quote"/>
                                    <xsl:with-param name="_first-line" select="false()"/>
                                    <xsl:with-param name="_show-lead-quote" select="true()"/>
                                </xsl:call-template>
                                
                            </xsl:otherwise>
                        </xsl:choose>
                        
                    </xsl:otherwise>
                </xsl:choose>
                
            </xsl:when>
            
            <xsl:otherwise>
                <xsl:call-template name="string:repeat">
                    <xsl:with-param name="string" select="' '"/>
                    <xsl:with-param name="times" select="$indent"/>
                </xsl:call-template>
                
                <xsl:if test="$_show-lead-quote=false()">
                    <xsl:if test="$show-quote=true() and $_first-line=true()">
                        <xsl:text>"</xsl:text>
                    </xsl:if>
                </xsl:if>
                
                <xsl:value-of select="$text"/>
                
                <xsl:if test="$show-quote=true()">
                    <xsl:text>"</xsl:text>
                </xsl:if>
            </xsl:otherwise>
            
        </xsl:choose>-->
        
    </xsl:template>
    
</xsl:stylesheet>