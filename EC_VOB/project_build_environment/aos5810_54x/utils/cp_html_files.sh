#! /bin/sh

WEB_ROOT=${ACCROOTFS}/usr/webroot
test -d ${WEB_ROOT} || mkdir -p ${WEB_ROOT}
rm -rf ${WEB_ROOT}/*

cd ${WEB_ROOT}
DEFAULT_PAGE=/index.htm
echo -ne $DEFAULT_PAGE > default_page.cfg

test -d ${WEB_ROOT}/js || mkdir -p ${WEB_ROOT}/js
cat <<EOF >> ${WEB_ROOT}/js/lang.js
i18n = {};
i18n.conf = {};
i18n.conf.panelLanguageOpValList = '$LANGUAGES';
i18n.conf.defaultLanguage = '$DEFAULT_LANGUAGE';
EOF

cd ${WEB_ROOT}
DEFAULT_WEB_ROOT_DIR=/usr/webroot
cat <<EOF >> ${WEB_ROOT}/config.json
{
    "api.url": "${DEFAULT_WEB_ROOT_DIR}/${DEFAULT_API_FILE_NAME}",
    "rootDir": "${DEFAULT_WEB_ROOT_DIR}"
}
EOF

cd ${SOURCE_PATH}/user/apps/web/html/
#cp --parents -f config/* ${WEB_ROOT}
#cp --parents -f css/edgecore/* ${WEB_ROOT}
#cp --parents -rf help/${PROJECT_ID}/* ${WEB_ROOT}
#cp --parents -f home/*.htm ${WEB_ROOT}
#cp --parents -f home/${PROJECT_ID}/* ${WEB_ROOT}
#cp --parents -f images/edgecore/* ${WEB_ROOT}
#cp --parents -f js/*.js ${WEB_ROOT}
#cp --parents -f js/${PROJECT_ID}/* ${WEB_ROOT}
#cp --parents -f webauth/* ${WEB_ROOT}
#cp -f index_${PROJECT_ID}.htm ${WEB_ROOT}/index.htm
#cp --parents -rf resources/* ${WEB_ROOT}
cp -f api.json ${WEB_ROOT}/api.json
