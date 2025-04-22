#-*- mode: makefile; -*-

docker/debian.env: templates/debian.env.in
	sed -e 's/[@]api_name[@]/$(API_NAME)/g;' \
	    -e 's/[@]mysql_user[@]/$(MYSQL_USER)/g;' \
	    -e 's/[@]mysql_host[@]/$(MYSQL_HOST)/g;' \
	    -e 's/[@]mysql_database[@]/$(MYSQL_DATABASE)/g;' \
	    -e 's/[@]mysql_password[@]/$(MYSQL_PASSWORD)/g;' \
	    -e 's/[@]api_key[@]/$(API_KEY)/g;' $< >$@ || rm $@

docker/setup.sql: templates/setup.sql.in
	sed -e 's/[@]mysql_user[@]/$(MYSQL_USER)/g;' \
	    -e 's/[@]mysql_host[@]/$(MYSQL_HOST)/g;' \
	    -e 's/[@]mysql_database[@]/$(MYSQL_DATABASE)/g;' \
	    -e 's/[@]mysql_password[@]/$(MYSQL_PASSWORD)/g;' $< >$@ || rm $@

DOCKER_CONFIG_FILES = $(addprefix docker/,$(CONFIG_FILES)) docker/$(TARBALL)

CLEANFILES += \
    $(DOCKER_CONFIG_FILES) \
    docker/debian.env \
    docker/setup.sql

########################################################################
# Docker image
########################################################################
PHONY_TARGETS += image
image:  state/image.json
########################################################################

########################################################################
# Strictly speaking `docker/debian.env` is not a dependency for the
# image. It is used when running docker-compose locally:
#
#  docker-compose --env debian.env up
########################################################################

IMAGE_DEPS = \
    docker/Dockerfile \
    $(TARBALL) \
    $(CONFIG_FILES) \
    docker/debian.env \
    docker/setup.sql \
    docker/start-server

state/image.json: $(IMAGE_DEPS)
	for a in $(CONFIG_FILES) $(TARBALL); do \
	  cp $$a docker/; \
	done; \
	test -n "$$NO_CACHE" && NO_CACHE='--no-cache'; \
	echo "$$NO_CACHE"; \
	pushd docker >/dev/null; \
	docker build $$NO_CACHE --build-arg API_NAME=$(API_NAME) --build-arg VERSION=$(VERSION) -f Dockerfile . -t $(TAG); \
	popd >/dev/null; \
	docker inspect $(TAG):latest  | jq -r '.[]|.' > $@
