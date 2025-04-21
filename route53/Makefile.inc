#-*- mode: makefile; -*-

########################################################################
# Route 53 - create CNAME to ALB
########################################################################
PHONY_TARGETS += update-route53
update-route53: state/update-route53
########################################################################

state/update-route53: state/alb-dns.json state/resource-record-set
	hosted_zone_id=$(AWS_HOSTED_ZONE_ID); \
	hosted_zone_id=$${hosted_zone_id:-$$(cat $< | jq -r .ZoneId)}; \
	aws route53 change-resource-record-sets --profile $(AWS_ROUTE53_PROFILE) \
	  --hosted-zone-id $$hosted_zone_id \
	  --change-batch file://state/resource-record-set >$@

########################################################################
PHONY_TARGETS += alb-dns
########################################################################
alb-dns: state/alb-dns.json

state/alb-dns.json:
	aws elbv2 describe-load-balancers --profile $(AWS_PROFILE) \
	  --names $(AWS_ALB_NAME) \
	  --query "LoadBalancers[0].{DNSName:DNSName,ZoneId:CanonicalHostedZoneId}" >$@
	test -s $@ || rm $@ && false;

########################################################################
PHONY_TARGETS += resource-record-set
########################################################################
resource-record-set: state/resource-record-set.json

state/resource-record-set.json: templates/resource-record-set.json.in state/alb-dns.json
	ALB_DNS=$$(cat state/alb-dns.json | jq -r .DNSName); \
	sed -e "s/[@]api_host[@]/$(API_HOST)/g" \
	    -e "s/[@]alb_dns[@]/$$ALB_DNS/g" < $< > $@
