#-*- mode: makefile; -*-

########################################################################
# Cloud Map - create DNS names for services
########################################################################
PHONY_TARGETS += service-discovery
service-discovery: state/service-discovery.json
########################################################################

state/service-discovery.json:
	aws servicediscovery create-private-dns-namespace --profile $(AWS_PROFILE) \
	  --name local \
	  --vpc $(AWS_VPC_ID) \
	  --region us-east-1 >$@

########################################################################
PHONY_TARGETS += service-discovery-operation
service-discovery-operation: state/service-discovery-operation.json
########################################################################

state/service-discovery-operation.json: state/service-discovery.json
	service=$$(aws servicediscovery list-services --profile $(AWS_PROFILE) \
	             --query "Services[?Name=='example-xapi']" |jq -r '.[]|.Arn'); \
	if test -z "$service"; then \
	  operation_id=$$(cat $< | jq -r .OperationId); \
	  until aws servicediscovery get-operation --profile $(AWS_PROFILE) --operation-id "$$op_id" \
	    | jq -r .Operation.Status | grep -q SUCCESS; do \
	    sleep 2; \
	  done; \
	  aws servicediscovery get-operation --profile $(AWS_PROFILE) \
	    --operation-id $$operation_id >$@; \
	fi

########################################################################
PHONY_TARGETS += mysql-local
mysql-local: state/mysql-local-registry-arn.json
########################################################################

state/mysql-local-registry-arn.json: state/service-discovery-operation.json
	namespace=$$(cat $< | jq -r .Operation.Targets.NAMESPACE); \
	aws servicediscovery create-service --profile $(AWS_PROFILE) \
	  --name mysql \
	  --dns-config "NamespaceId=$$namespace,DnsRecords=[{Type=A,TTL=60}]" \
	  --health-check-custom-config FailureThreshold=1 \
	  --query 'Service' --output json >$@

########################################################################
PHONY_TARGETS += web-local
web-local: state/web-local-registry-arn.json
########################################################################

state/web-local-registry-arn.json: state/service-discovery-operation.json
	namespace=$$(cat $< | jq -r .Operation.Targets.NAMESPACE); \
	aws servicediscovery create-service --profile $(AWS_PROFILE) \
	  --name $(SERVICE) \
	  --dns-config "NamespaceId=$$namespace,DnsRecords=[{Type=A,TTL=60}]" \
	  --health-check-custom-config FailureThreshold=1 \
	  --query 'Service' --output json >$@
