FROM bedrock-debian

ARG VERSION

RUN apt update && apt-get install -y \
       unzip autoconf autoconf-archive make libtool automake \
       libxml2-dev pkg-config libgd-dev default-mysql-client

ENV PERL_CPANM_OPT="-n -v --mirror https://cpan.treasurersbriefcase.com/orepan2 --mirror https://cpan.metacpan.org"

RUN a2enmod headers
RUN a2enmod rewrite

RUN a2dissite 000-default

RUN cpanm -n -v --mirror-only --reinstall Bedrock

COPY Bedrock-Apache-API-$VERSION.tar.gz .
RUN cpanm -n -v Bedrock-Apache-API-$VERSION.tar.gz

COPY apache2.conf /etc/apache2
COPY api-vhost.conf /etc/apache2/sites-available
COPY api.conf /etc/apache2/conf-available

RUN a2ensite api-vhost
RUN a2enconf api

# STARTUP BLMS
COPY mysql-session.xml /var/www/bedrock/config.d/startup

# CONFIGS
COPY api.xml /var/www/bedrock/config/example-api.xml

COPY setup.sql /usr/local/share/setup.sql

COPY start-server /usr/local/bin/start-server

RUN rm -rf /var/www/html/*

CMD /usr/local/bin/start-server
