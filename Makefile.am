SUBDIRS =  .
#-*- mode: makefile; -*-

########################################################################
# See README-BUILD.md for recipes
########################################################################

SHELL = /bin/bash
.SHELLFLAGS = -ec

# base url of the API, e.g. /example-api
BASE_URL       = @base_url@

# domain used to allow cross-site access (see api.conf)
DOMAIN         = @domain@

# this is the base module we build in the CPAN distribution
MODULE_NAME    = Bedrock::Apache::API

# the suffix name of your mod_perl handler, e.g. ExampleAPI
HANDLER        = @handler@

# API key sent in the header as X-API-KEY which your API can use to
# authenticate requests
API_KEY        = @api_key@

# API name e.g. example-api
API_NAME       = @api_name@

# host name that will be added to Route53 as a CNAME to your ALB if
# both are enabled
API_HOST       = @api_host@

MYSQL_USER     = @mysql_user@
MYSQL_PASSWORD = @mysql_password@
MYSQL_HOST     = @mysql_host@
MYSQL_DATABASE = @mysql_database@

# name used to tag docker images
TAG  = @service_name@

# PHONY targets are collected in each Makefile.inc files like this:
# PHONY_TARGETS += foo
PHONY_TARGETS = 

# ...likewise with CLEANFILES
CLEANFILES = 

all: $(TARBALL) image

# Apache configs (.conf), application configs (.xml)
include config/Makefile.inc

# Perl source
include source/Makefile.inc

# Docker image build, docker-compose configuration, image startup scripts
include docker/Makefile.inc

if USE_AWS
AWS_ACCOUNT              = @aws_account@
AWS_PROFILE              = @aws_profile@
AWS_REGION               = @aws_region@
AWS_CLUSTER_NAME         = @aws_cluster_name@
AWS_ALB_NAME             = @aws_alb_name@
AWS_ROUTE53_PROFILE      = @aws_route53_profile@
AWS_HOSTED_ZONE_ID       = @aws_hosted_zone_id@
AWS_MYSQL_SECURITY_GROUP = @aws_mysql_security_group@
AWS_VPC_ID               = @aws_vpc_id@
SERVICE                  = @service_name@
ECS_REPO                 = $(AWS_ACCOUNT).dkr.ecr.$(AWS_REGION).amazonaws.com

########################################################################
# Artifacts for creating a Fargate service
########################################################################
include ecr/Makefile.inc
include cloudmap/Makefile.inc
include cloudwatch-logs/Makefile.inc
include mysql-service/Makefile.inc
include app-service/Makefile.inc
include secrets/Makefile.inc
include iam-policies/Makefile.inc

if USE_AUTOSCALING
include autoscaling/Makefile.inc
endif

if USE_LOADBALANCER
include alb/Makefile.inc

if USE_ROUTE53
include route53/Makefile.inc
endif

endif

endif


.PHONY: $(PHONY_TARGETS)
