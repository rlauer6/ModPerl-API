SUBDIRS = .

BASE_URL = @base_url@
HANDLER  = @handler@
DOMAIN   = @domain@
API_KEY  = @api_key@

MYSQL_HOST = @mysql_host@
MYSQL_USER = @mysql_user@
MYSQL_PASS = @mysql_password@

all: api.conf api.xml

api.xml: api.xml.in
	sed -e 's/[@]db_hostl[@]/$(MYSQL_HOST)/g' \
	-e 's/[@]db_user[@]/$(MYSQL_USER)/g' \
	-e 's/[@]db_password[@]/$(MYSQL_PASS)/g' \
	-e 's/[@]api_key[@]/$(API_KEY)/g' $< > $@

api.conf: config/api.conf.in
	sed -e 's/[@]base_url[@]/$(BASE_URL)/g' \
	-e 's/[@]api_name[@]/$(HANDLER)/g' \
	-e 's/[@]handler[@]/$(HANDLER)/g' \
	-e 's/[@]domain[@]/$(DOMAIN)/g' $< > $@

CONFIG_FILES = \
    api.conf \
    api-vhost.conf \
    apache2.conf

CLEANFILES = \
    api.conf \
    api.xml
