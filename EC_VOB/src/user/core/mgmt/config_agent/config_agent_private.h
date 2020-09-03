//
//  config_agent_private.h
//  config_agent
//
//  Created by JunYing Yeh on 2014/11/6.
//
//

#ifndef _CONFIG_AGENT_HEADER_PRIVATE_H_
#define _CONFIG_AGENT_HEADER_PRIVATE_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "config_agent_private_config.h"
#include "config_agent_config.h"
#include "config_agent_auto_config.h"
#include "config_agent.h"

#if defined(CONFIG_AGENT_HAVE_MONGOC)
#  include <mongoc.h>
#endif

#if defined(CONFIG_AGENT_HAVE_BSON)
#  include <bson.h>
#endif

#if defined(CONFIG_AGENT_HAVE_LIBXML2)
#  include <libxml\parser.h>
#  include <libxml\tree.h>
#endif

#if defined(CONFIG_AGENT_HAVE_INIH)
#  include <ini.h>
#endif

#if defined(CONFIG_AGENT_HAVE_LIBUCL)
#  include <ucl.h>
#endif

#if defined(CONFIG_AGENT_HAVE_PCRE)
#  include <pcreposix.h>
#  define CONFIG_AGENT_HAVE_REGEX 1
#elif defined(CONFIG_AGENT_HAVE_PCRE2)
#  define PCRE2_CODE_UNIT_WIDTH 8
#  include <pcre2.h>
#  define CONFIG_AGENT_HAVE_REGEX 1
#endif

#if !defined(_countof)
#  define _countof(_array) (sizeof(_array)/sizeof(*_array))
#endif

#include "config_agent_lock.h"

#endif /* defined(_CONFIG_AGENT_HEADER_PRIVATE_H_) */

