<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl"
    exclude-result-prefixes="xd"
    xmlns:dyn="http://www.mibx.com/dyn"
    extension-element-prefixes="dyn">
    
    <xd:doc scope="stylesheet">
        <xd:desc>
            <xd:p><xd:b>Created on:</xd:b> Jan 17, 2013</xd:p>
            <xd:p><xd:b>Author:</xd:b> yehjunying</xd:p>
            <xd:p></xd:p>
        </xd:desc>
    </xd:doc>

    <!--
        @description Evaluate expression
        @arg An expression. It may contain
            1. constansts
            2. true, means 1
            3. false, means 0
            4. logical operations (or, and, =, !=).
    -->
    <xsl:template name="dyn:evaluate">
        <xsl:param name="arg"/>
        
        <xsl:variable name="result">
            <xsl:choose>

                <xsl:when test="contains($arg, 'or')">
                    <xsl:variable name="op" select="'or'"/>
                    <xsl:variable name="lhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-before($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="rhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-after($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:if test="($lhs='1') or ($rhs='1')">
                        <xsl:value-of select="1"/>
                    </xsl:if>
                </xsl:when>
                
                <xsl:when test="contains($arg, 'and')">
                    <xsl:variable name="op" select="'and'"/>
                    <xsl:variable name="lhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-before($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="rhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-after($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:if test="($lhs='1') and ($rhs='1')">
                        <xsl:value-of select="1"/>
                    </xsl:if>
                </xsl:when>                

                <xsl:when test="contains($arg, '!=')">
                    <xsl:variable name="op" select="'!='"/>
                    <xsl:variable name="lhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-before($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="rhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-after($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:if test="not($lhs=$rhs)">
                        <xsl:value-of select="1"/>
                    </xsl:if>
                </xsl:when>

                <xsl:when test="contains($arg, '=')">
                    <xsl:variable name="op" select="'='"/>
                    <xsl:variable name="lhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-before($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="rhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-after($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:if test="$lhs=$rhs">
                        <xsl:value-of select="1"/>
                    </xsl:if>
                </xsl:when>
                
                <xsl:when test="contains($arg, '&gt;')">
                    <xsl:variable name="op" select="'&gt;'"/>
                    <xsl:variable name="lhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-before($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="rhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-after($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:if test="$lhs&gt;$rhs">
                        <xsl:value-of select="1"/>
                    </xsl:if>
                </xsl:when>

                <xsl:when test="contains($arg, '&lt;')">
                    <xsl:variable name="op" select="'&lt;'"/>
                    <xsl:variable name="lhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-before($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:variable name="rhs">
                        <xsl:call-template name="dyn:evaluate">
                            <xsl:with-param name="arg" select="substring-after($arg, $op)"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:if test="$lhs&lt;$rhs">
                        <xsl:value-of select="1"/>
                    </xsl:if>
                </xsl:when>
               
                <xsl:when test="normalize-space($arg)='1' or normalize-space($arg)='true'">
                    <xsl:value-of select="1"/>
                </xsl:when>
                
                <xsl:when test="normalize-space($arg)='0' or normalize-space($arg)='false'"/>

                <xsl:otherwise>
                    <xsl:value-of select="normalize-space($arg)"/>
                </xsl:otherwise>

            </xsl:choose>
        </xsl:variable>
        
        <xsl:value-of select="$result"/>
    </xsl:template>

</xsl:stylesheet>
