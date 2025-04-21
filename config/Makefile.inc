#-*- mode: makefile; -*-

config/mysql-session.xml: config/mysql-session.xml.in
	sed -e 's/[@]db_host[@]/$(MYSQL_HOST)/g' \
	-e 's/[@]db_user[@]/$(MYSQL_USER)/g' \
	-e 's/[@]db_password[@]/$(MYSQL_PASSWORD)/g' \
	-e 's/[@]db_name[@]/$(MYSQL_DATABASE)/g' $< > $@

config/api.xml: config/api.xml.in
	sed -e 's/[@]db_host[@]/$(MYSQL_HOST)/g' \
	-e 's/[@]db_user[@]/$(MYSQL_USER)/g' \
	-e 's/[@]db_password[@]/$(MYSQL_PASSWORD)/g' \
	-e 's/[@]db_name[@]/$(MYSQL_DATABASE)/g' \
	-e 's/[@]api_key[@]/$(API_KEY)/g' $< > $@

config/api.conf: config/api.conf.in
	sed -e 's|[@]base_url[@]|$(BASE_URL)|g' \
	-e 's/[@]api_name[@]/$(API_NAME)/g' \
	-e 's/[@]handler[@]/$(HANDLER)/g' \
	-e 's/[@]domain[@]/$(DOMAIN)/g' $< > $@

CONFIG_FILES = \
    config/api.xml \
    config/mysql-session.xml \
    config/api.conf \
    config/api-vhost.conf \
    config/apache2.conf

CLEANFILES += \
    config/mysql-session.xml \
    config/api.conf \
    config/api.xml
