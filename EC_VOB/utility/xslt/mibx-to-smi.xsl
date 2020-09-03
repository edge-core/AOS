<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl"
    exclude-result-prefixes="xd"
    xmlns:smi="http://www.mibx.com/smi"
    xmlns:string="http://www.mibx.com/string"
    version="1.0">
    
    <xsl:import href="lib/lib.xsl"/>
    
    <xd:doc scope="stylesheet">
        <xd:desc>
            <xd:p><xd:b>Created on:</xd:b> Jan 18, 2013</xd:p>
            <xd:p><xd:b>Author:</xd:b> yehjunying</xd:p>
            <xd:p></xd:p>
        </xd:desc>
    </xd:doc>
    
    <xsl:include href="mibx-to-smi-config.xsl"/>
    <xsl:param name="cr">
        <xsl:text>&#xA;</xsl:text>
    </xsl:param>
    
    <xsl:output method="text"/>
    <xsl:strip-space elements="*"/>
        
    <xsl:template name="smi:root" match="smi:SMI">
        <xsl:param name="root" select="."/>
                
        <xsl:value-of select="$root/smi:module/smi:name"/>
        <xsl:text> </xsl:text>
        <xsl:text>DEFINITIONS ::= BEGIN</xsl:text>
        <xsl:value-of select="$cr"/>
        <xsl:value-of select="$cr"/>
        
        <xsl:text>IMPORTS</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:apply-templates select="$root/smi:import"/>
        
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:value-of select="$cr"/>
        
        <xsl:apply-templates select="$root/*[not(local-name(.)='import')] | $root/comment()"/>
        
        <xsl:text>END</xsl:text>
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="comment" match="comment()">
        <xsl:param name="text" select="."/>
        
        <xsl:if test="not($ignore-comment=true())">
            <xsl:choose>
                <xsl:when test="contains($text, $cr)">
                    <xsl:call-template name="comment">
                        <xsl:with-param name="text" select="substring-before($text, $cr)"/>
                    </xsl:call-template>
                    <xsl:call-template name="comment">
                        <xsl:with-param name="text" select="substring-after($text, $cr)"/>
                    </xsl:call-template>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>-- </xsl:text>
                    
                    <xsl:choose>
                        <xsl:when test="$preserve-white-space=true()">
                            <xsl:value-of select="$text"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="normalize-space($text)"/>
                        </xsl:otherwise>
                    </xsl:choose>
                    
                    <xsl:value-of select="$cr"/>                
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>
    
    <xsl:template name="import" match="smi:import">
        <xsl:param name="import" select="."/>
        <xsl:param name="indent" select="$indent-space"/>
                
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:value-of select="$import/smi:name"/>
        <xsl:text> FROM </xsl:text>
        <xsl:value-of select="$import/smi:module"/>
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:module-identity" match="smi:module">
        <xsl:param name="module" select="."/>
        <xsl:param name="indent" select="0"/>
        
        <xsl:value-of select="$module/smi:identity"/>
        <xsl:text> MODULE-IDENTITY</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:call-template name="smi:show-text">
            <xsl:with-param name="title" select="'LAST-UPDATED'"/>
            <xsl:with-param name="content">
                <xsl:for-each select="$module/smi:revision">
                    <xsl:sort select="smi:date" data-type="text" order="descending"/>
                    <xsl:if test="position()=1">
                        <xsl:value-of select="smi:date"/>
                    </xsl:if>
                </xsl:for-each>
            </xsl:with-param>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:show-text">
            <xsl:with-param name="title" select="'ORGANIZATION'"/>
            <xsl:with-param name="content" select="$module/smi:organization"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:show-text">
            <xsl:with-param name="title" select="'CONTACT-INFO'"/>
            <xsl:with-param name="content" select="$module/smi:contact"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:show-text">
            <xsl:with-param name="title" select="'DESCRIPTION'"/>
            <xsl:with-param name="content" select="$module/smi:description"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        <xsl:value-of select="$cr"/>
        
        <xsl:for-each select="$module/smi:revision">
            <xsl:sort select="smi:date" data-type="text" order="descending"/>
            
            <xsl:call-template name="smi:show-text">
                <xsl:with-param name="title" select="'REVISION   '"/>
                <xsl:with-param name="content" select="smi:date"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
            
            <xsl:call-template name="smi:show-text">
                <xsl:with-param name="title" select="'DESCRIPTION'"/>
                <xsl:with-param name="content" select="smi:description"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
        </xsl:for-each>
        
        <xsl:call-template name="smi:oid">
            <xsl:with-param name="oid" select="$module/smi:oid"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:object-identifier" match="smi:node">
        <xsl:param name="node" select="."/>
        
        <xsl:variable name="export-node">
            <xsl:choose>
                <xsl:when test="parent::node()/parent::node()/smi:module[smi:identity/@node=$node/@name]">
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="'1'"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>        
        
        <xsl:if test="$export-node=true()">
            <xsl:value-of select="$node/smi:name"/>
            <xsl:text> OBJECT IDENTIFIER</xsl:text>
            
            <xsl:call-template name="smi:oid">
                <xsl:with-param name="oid" select="$node/smi:oid"/>
                <xsl:with-param name="indent" select="1"/>
            </xsl:call-template>
            <xsl:value-of select="$cr"/>
        </xsl:if>
    </xsl:template>
    
    <xsl:template name="smi:textual-convention" match="smi:typedef">
        <xsl:param name="typedef" select="."/>
        <xsl:param name="indent" select="0"/>
        <xsl:value-of select="$typedef/smi:name"/>
        <xsl:text> ::= TEXTUAL-CONVENTION</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:if test="$typedef/smi:display">
            <xsl:call-template name="smi:show-text">
                <xsl:with-param name="title" select="'DISPLAY-HINT'"/>
                <xsl:with-param name="content" select="$typedef/smi:display"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
        </xsl:if>
        
        <xsl:call-template name="smi:status">
            <xsl:with-param name="status" select="$typedef/smi:status"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:show-text">
            <xsl:with-param name="title" select="'DESCRIPTION'"/>
            <xsl:with-param name="content" select="$typedef/smi:description"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:syntax">
            <xsl:with-param name="syntax" select="$typedef/smi:syntax"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:notification-type" match="smi:notification">
        <xsl:param name="notification" select="."/>
        <xsl:param name="indent" select="0"/>
        
        <xsl:value-of select="$notification/smi:name"/>
        <xsl:text> NOTIFICATION-TYPE</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:call-template name="smi:objects">
            <xsl:with-param name="objects" select="$notification/smi:objects"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:status">
            <xsl:with-param name="status" select="$notification/smi:status"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:show-text">
            <xsl:with-param name="title" select="'DESCRIPTION'"/>
            <xsl:with-param name="content" select="$notification/smi:description"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:oid">
            <xsl:with-param name="oid" select="$notification/smi:oid"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:objects">
        <xsl:param name="objects"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:text>OBJECTS</xsl:text>
        
        <xsl:text> </xsl:text>
        <xsl:text>{ </xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:for-each select="$objects/smi:object">
            <xsl:call-template name="string:repeat">
                <xsl:with-param name="string" select="' '"/>
                <xsl:with-param name="times" select="$indent + $indent-space"/>
            </xsl:call-template>
            
            <xsl:value-of select="."/>
            
            <xsl:if test="position()!=last()">
                <xsl:text>,</xsl:text>
            </xsl:if>
            
            <xsl:value-of select="$cr"/>
        </xsl:for-each>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:text> }</xsl:text>
        
        <xsl:value-of select="$cr"/>        
    </xsl:template>
    
    <xsl:template name="smi:object-type" match="smi:scalar|smi:table|smi:row|smi:column">
        <xsl:param name="object-type" select="."/>
        <xsl:param name="indent" select="0"/>
        
        <xsl:value-of select="$object-type/smi:name"/>
        <xsl:text> OBJECT-TYPE</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:choose>
            <xsl:when test="local-name($object-type)='table' or local-name($object-type)='row'">
                <xsl:call-template name="smi:syntax">
                    <xsl:with-param name="syntax" select="$object-type"/>
                    <xsl:with-param name="indent" select="$indent + $indent-space"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="smi:syntax">
                    <xsl:with-param name="syntax" select="$object-type/smi:syntax"/>
                    <xsl:with-param name="indent" select="$indent + $indent-space"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
        
        <xsl:if test="$object-type/smi:units">
            <xsl:call-template name="smi:show-text">
                <xsl:with-param name="title" select="'UNITS'"/>
                <xsl:with-param name="content" select="$object-type/smi:units"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
        </xsl:if>
        
        <xsl:call-template name="smi:access">
            <xsl:with-param name="access">
                <xsl:choose>
                    <xsl:when test="local-name($object-type)='table' or local-name($object-type)='row'">
                        <xsl:value-of select="'not-accessible'"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$object-type/smi:access"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:with-param>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:status">
            <xsl:with-param name="status" select="$object-type/smi:status"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:call-template name="smi:show-text">
            <xsl:with-param name="title" select="'DESCRIPTION'"/>
            <xsl:with-param name="content" select="$object-type/smi:description"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:if test="$object-type/smi:reference">
            <xsl:call-template name="smi:show-text">
                <xsl:with-param name="title" select="'REFERENCE  '"/>
                <xsl:with-param name="content" select="$object-type/smi:reference"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
        </xsl:if>
        
        <xsl:if test="$object-type/smi:index">
            <xsl:call-template name="smi:index">
                <xsl:with-param name="index" select="$object-type/smi:index"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
        </xsl:if>
        
        <xsl:if test="$object-type/smi:augments">
            <xsl:call-template name="smi:augments">
                <xsl:with-param name="index" select="$object-type/smi:augments"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
        </xsl:if>
        
        <xsl:if test="$object-type/smi:default">
            <xsl:call-template name="smi:default">
                <xsl:with-param name="index" select="$object-type/smi:default"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
        </xsl:if>        
        
        <xsl:call-template name="smi:oid">
            <xsl:with-param name="oid" select="$object-type/smi:oid"/>
            <xsl:with-param name="indent" select="$indent + $indent-space"/>
        </xsl:call-template>
        
        <xsl:value-of select="$cr"/>
        
        <xsl:if test="local-name($object-type)='table'">
            <xsl:apply-templates select="$object-type/smi:row"/>
        </xsl:if>
        
        <xsl:if test="local-name($object-type)='row'">
            
            <xsl:call-template name="smi:entry-type">
                <xsl:with-param name="row" select="$object-type"/>
                <xsl:with-param name="indent" select="$indent"/>
            </xsl:call-template>
            <xsl:value-of select="$cr"/>
            
            <xsl:apply-templates select="$object-type/smi:column"/>
        </xsl:if>
        
    </xsl:template>
        
    <xsl:template name="smi:entry-type">
        <xsl:param name="row"/>
        <xsl:param name="indent"/>
        
        <xsl:call-template name="string:capitalize-first">
            <xsl:with-param name="arg" select="$row/smi:name"/>
        </xsl:call-template>
        
        <xsl:text> ::= SEQUENCE {</xsl:text>
        <xsl:value-of select="$cr"/>
                
        <xsl:for-each select="$row/smi:column">
            <xsl:call-template name="string:repeat">
                <xsl:with-param name="string" select="' '"/>
                <xsl:with-param name="times" select="$indent + $indent-space"/>
            </xsl:call-template>
            
            <xsl:value-of select="smi:name"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="smi:syntax/smi:baseType"/>
            
            <xsl:if test="position()!=last()">
                <xsl:text>,</xsl:text>
                <xsl:value-of select="$cr"/>
            </xsl:if>
        </xsl:for-each>
        
        <xsl:value-of select="$cr"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:text>}</xsl:text>
        
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:syntax">
        <xsl:param name="syntax"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:variable name="title" select="'SYNTAX      '"/>
        <xsl:value-of select="$title"/>
        
        <xsl:choose>
            <xsl:when test="local-name($syntax)='table'">
                <xsl:text>SEQUENCE OF </xsl:text>
                <xsl:call-template name="string:capitalize-first">
                    <xsl:with-param name="arg" select="$syntax/smi:row/smi:name"/>
                </xsl:call-template>
            </xsl:when>
            
            <xsl:when test="local-name($syntax)='row'">
                <xsl:call-template name="string:capitalize-first">
                    <xsl:with-param name="arg" select="$syntax/smi:name"/>
                </xsl:call-template>
            </xsl:when>
            
            <xsl:otherwise>
                <xsl:value-of select="$syntax/smi:baseType"/>                
                <xsl:call-template name="smi:subtype">
                    <xsl:with-param name="subtype" select="$syntax"/>
                    <xsl:with-param name="indent" select="$indent + string-length($title)"/>
                </xsl:call-template>
            </xsl:otherwise>
            
        </xsl:choose>
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:subtype">
        <xsl:param name="subtype"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:if test="$subtype/smi:range">
            
            <xsl:text> </xsl:text>
            
            <xsl:variable name="is-octet-string-type">
                <xsl:call-template name="smi:is-octet-string-type">
                    <xsl:with-param name="subtype" select="$subtype"/>
                </xsl:call-template>
            </xsl:variable>
            
            <xsl:if test="$is-octet-string-type='1'">
                <xsl:text>(SIZE </xsl:text>
            </xsl:if>
            
            <xsl:text>(</xsl:text>
            <xsl:call-template name="smi:range">
                <xsl:with-param name="list" select="$subtype/smi:range"/>
            </xsl:call-template>
            <xsl:text>)</xsl:text>
            
            <xsl:if test="$is-octet-string-type='1'">
                <xsl:text>)</xsl:text>
            </xsl:if>
            
        </xsl:if>
        
        <xsl:if test="$subtype/smi:namedNumber">
            <xsl:text> {</xsl:text>
            <xsl:value-of select="$cr"/>
            
            <xsl:call-template name="smi:namedNumber">
                <xsl:with-param name="list" select="$subtype/*[local-name(.)='option' or local-name(.)='namedNumber']"/>
                <xsl:with-param name="indent" select="$indent + $indent-space"/>
            </xsl:call-template>
            
            <xsl:value-of select="$cr"/>
            
            <xsl:call-template name="string:repeat">
                <xsl:with-param name="string" select="' '"/>
                <xsl:with-param name="times" select="$indent"/>
            </xsl:call-template>
            
            <xsl:text>}</xsl:text>
        </xsl:if>
        
    </xsl:template>
    
    <xsl:template name="smi:is-octet-string-type">
        <xsl:param name="subtype"/>
        
        <xsl:choose>
            <xsl:when test="$subtype/smi:baseType='OCTET STRING'">
                <xsl:value-of select="1"/>
            </xsl:when>
            
            <!-- SNMPv2-TC -->
            <xsl:when test="$subtype/smi:baseType='DisplayString'">
                <xsl:value-of select="1"/>
            </xsl:when>
            
            <xsl:when test="$subtype/smi:baseType='PhysAddress'">
                <xsl:value-of select="1"/>
            </xsl:when>
            
            <xsl:when test="$subtype/smi:baseType='MacAddress'">
                <xsl:value-of select="1"/>
            </xsl:when>
            
            <xsl:when test="$subtype/smi:baseType='DateAndTime'">
                <xsl:value-of select="1"/>
            </xsl:when>
            
            <xsl:when test="$subtype/smi:baseType='TAddress'">
                <xsl:value-of select="1"/>
            </xsl:when>
            
            <!-- INET-ADDRESS-MIB -->
            <xsl:when test="$subtype/smi:baseType='InetAddress'">
                <xsl:value-of select="1"/>
            </xsl:when>
            
        </xsl:choose>
        
    </xsl:template>
    
    <xsl:template name="smi:range">
        <xsl:param name="list"/>
        
        <xsl:variable name="group" select="$list[1]"/>
        
        <xsl:choose>
            <xsl:when test="$group/smi:min = $group/smi:max">
                <xsl:value-of select="$group/smi:min"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$group/smi:min"/>
                <xsl:text>..</xsl:text>
                <xsl:value-of select="$group/smi:max"/>
            </xsl:otherwise>
        </xsl:choose>
        
        <xsl:if test="count($list)>count($group)">
            
            <xsl:text> | </xsl:text>
            
            <xsl:call-template name="smi:range">
                <xsl:with-param name="list" select="$list[not(position()=1)]"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    
    <xsl:template name="smi:namedNumber">
        <xsl:param name="list"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:variable name="group-identifier" select="$list[1]/smi:name"/>
        <xsl:variable name="group" select="$list[smi:name=$group-identifier]"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:value-of select="$group/smi:name"/>
        <xsl:text>(</xsl:text>
        <xsl:value-of select="$group/smi:number"/>
        <xsl:text>)</xsl:text>
        
        <xsl:if test="count($list)>count($group)">
            
            <xsl:text>,</xsl:text>
            <xsl:value-of select="$cr"/>
            
            <xsl:call-template name="smi:namedNumber">
                <xsl:with-param name="list" select="$list[not(smi:name=$group-identifier)]"/>
                <xsl:with-param name="indent" select="$indent"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    
    <xsl:template name="smi:access">
        <xsl:param name="access"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:text>MAX-ACCESS  </xsl:text>
        <xsl:value-of select="$access"/>
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:index">
        <xsl:param name="index"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:variable name="title" select="'INDEX      '"></xsl:variable>
        <xsl:value-of select="$title"/>
        
        <xsl:text> {</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:for-each select="$index">
            
            <xsl:call-template name="string:repeat">
                <xsl:with-param name="string" select="' '"/>
                <xsl:with-param name="times" select="$indent + string-length($title) + $indent-space"/>
            </xsl:call-template>            
            
            <xsl:if test="smi:implied/text()='true'">
                <xsl:text>IMPLIED </xsl:text>
            </xsl:if>
            
            <xsl:value-of select="smi:name"/>
            
            <xsl:if test="position()!=last()">
                <xsl:text>, </xsl:text>
            </xsl:if>
            
            <xsl:value-of select="$cr"/>
            
        </xsl:for-each>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent + string-length($title)"/>
        </xsl:call-template>
        
        <xsl:text> }</xsl:text>
        
        <xsl:value-of select="$cr"/>
        
    </xsl:template>
    
    <xsl:template name="smi:augments">
        <xsl:param name="augments"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:variable name="title" select="'AUGMENTS   '"/>
        <xsl:value-of select="$title"/>
        
        <xsl:text> {</xsl:text>
        <xsl:value-of select="$cr"/>

        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent + string-length($title) + $indent-space"/>
        </xsl:call-template>  
        
        <xsl:value-of select="smi:augments"/>
        
        <xsl:value-of select="$cr"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent + string-length($title)"/>
        </xsl:call-template>
        
        <xsl:text> }</xsl:text>
        
        <xsl:value-of select="$cr"/>
        
    </xsl:template>
    
    <xsl:template name="smi:default">
        <xsl:param name="default"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:variable name="title" select="'DEFVAL     '"/>
        <xsl:value-of select="$title"/>
        
        <xsl:text> {</xsl:text>
        <xsl:value-of select="$cr"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent + string-length($title) + $indent-space"/>
        </xsl:call-template>  
        
        <xsl:value-of select="smi:default"/>
        
        <xsl:value-of select="$cr"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent + string-length($title)"/>
        </xsl:call-template>
        
        <xsl:text> }</xsl:text>
        
        <xsl:value-of select="$cr"/>
        
    </xsl:template>    
    
    <xsl:template name="smi:status">
        <xsl:param name="status"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:text>STATUS      </xsl:text>
        
        <xsl:choose>
            <xsl:when test="$status">
                <xsl:value-of select="$status"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>current</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:value-of select="$cr"/>
    </xsl:template>
    
    <xsl:template name="smi:show-text">
        <xsl:param name="title"/>
        <xsl:param name="content"/>
        <xsl:param name="indent" select="0"/>
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:value-of select="$title"/>
        <xsl:value-of select="' '"/>
        
        <xsl:call-template name="string:petty-print">
            <xsl:with-param name="text" select="$content"/>
            <xsl:with-param name="indent" select="$indent + string-length($title) + 1"/>
            <xsl:with-param name="show-quote" select="true()"/>
            <xsl:with-param name="line-wrap" select="true()"/>
            <xsl:with-param name="wrap-col" select="$wrap-col"/>
        </xsl:call-template>
        <xsl:value-of select="$cr"/>        
    </xsl:template>
    
    <xsl:template name="smi:oid">
        <xsl:param name="oid"/>
        <xsl:param name="indent" select="$indent-space"/>
        
        <xsl:call-template name="string:repeat">
            <xsl:with-param name="string" select="' '"/>
            <xsl:with-param name="times" select="$indent"/>
        </xsl:call-template>
        
        <xsl:choose>
            <xsl:when test="$oid">
                <xsl:text>::= { </xsl:text>
                <xsl:value-of select="$oid"/>
                <xsl:text> }</xsl:text>
            </xsl:when>
        </xsl:choose>
        <xsl:value-of select="$cr"/>       
    </xsl:template>    

</xsl:stylesheet>