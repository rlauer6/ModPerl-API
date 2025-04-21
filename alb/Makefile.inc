#-*- mode: makefile; -*-

########################################################################
# Listener, Listener Rule, Target Group
########################################################################

########################################################################
# Target group
########################################################################
PHONY_TARGETS += target-group
target-group: state/target-group.json
########################################################################

state/target-group.json:
	aws elbv2 describe-target-groups --profile $(AWS_PROFILE) \
	  --query "TargetGroups[?TargetGroupName=='$(SERVICE)-tg']" | jq -r '.[]|.' >$@

	if ! test -s $@; then \
	  aws elbv2 create-target-group  --profile $(AWS_PROFILE) \
	    --name $(SERVICE)-tg \
	    --protocol HTTP \
	    --port 80 \
	    --target-type ip \
	    --vpc-id $(AWS_VPC_ID) \
	    --health-check-protocol HTTP \
	    --health-check-port traffic-port \
	    --health-check-path /example-api/version \
	    --health-check-interval-seconds 30 \
	    --health-check-timeout-seconds 5 \
	    --healthy-threshold-count 2 \
	    --unhealthy-threshold-count 3 >$@; \
	fi

########################################################################
PHONY_TARGETS += load-balancer-arn
load-balancer-arn: state/load-balancer-arn.json
########################################################################

state/load-balancer-arn.json:
	aws elbv2 describe-load-balancers --profile $(AWS_PROFILE) \
	  --names $(AWS_ALB_NAME) \
	  --query "LoadBalancers[0]" \
	  --output json > $@
	test -s $@ || rm $@ && false;


########################################################################
# Listener, Listener rule
########################################################################
PHONY_TARGETS += listener
listener: state/listener.json
########################################################################

LISTENER_DEPS = \
    state/target-group.json \
    state/load-balancer-arn.json \
    state/listener-rule-arn.json \
    state/listener-actions.json \
    state/listener-rule.json \
    state/listener-arn.json

state/listener.json: $(LISTENER_DEPS)
	TARGET_GROUP_ARN=$$(cat state/target-group.json | jq -r '.TargetGroups[]|.TargetGroupArn'); \
	ALB_ARN=$$(cat state/load-balancer-arn.json | jq -r .LoadBalancerArn); \
	LISTENER_ARN=$$(cat state/listener-arn.json | jq -r .ListenerArn); \
	if test -z "$$LISTENER_ARN"; then \
	  aws elbv2 create-listener --profile $(AWS_PROFILE) \
	    --load-balancer-arn $$ALB_ARN \
	    --protocol HTTP \
	    --port 80 \
	    --default-actions Type=forward,TargetGroupArn=$$TARGET_GROUP_ARN >$@; \
	fi;
	if ! test -s state/listener-rule-arn.json; then
	  aws elbv2 create-rule --profile $(AWS_PROFILE) \
	    --listener-arn $$LISTENER_ARN \
	    --priority 10 \
	    --conditions file://state/listener-rule.json \
	    --actions file://state/listener-actions.json;
	fi

########################################################################
PHONY_TARGETS += listener-actions
listener-actions: state/listener-actions.json
########################################################################

state/listener-actions.json: templates/listener-actions.json.in state/target-group.json
	TARGET_GROUP_ARN=$$(cat state/target-group.json | jq -r '.TargetGroups[]|.TargetGroupArn'); \
	sed -e "s/[@]target_group_arn[@]/$$TARGET_GROUP_ARN/" < $< > $@

########################################################################
PHONY_TARGETS += listener-rule
listener-rule: state/listener-rule.json
########################################################################

state/listener-rule.json: templates/listener-rule.json.in
	sed -e "s/[@]api_host[@]/$(API_HOST)/" \
	    -e "s/[@]base_url[@]/$(BASE_URL)/" < $< > $@

########################################################################
PHONY_TARGETS += listener-arn
listener-arn: state/listener-arn.json
########################################################################

state/listener-arn.json: state/load-balancer-arn.json
	ALB_ARN=$$(cat state/load-balancer-arn.json | jq -r .LoadBalancerArn); \
	aws elbv2 describe-listeners --profile $(AWS_PROFILE) \
	  --load-balancer-arn $$ALB_ARN \
	  --query "Listeners[?Port==\`80\`]" \
	  --output json 2>/dev/null | jq -r '.[]|.' >$@

########################################################################
PHONY_TARGETS += listener-rule-arn
listener-rule-arn: state/listener-rule-arn.json
########################################################################

state/listener-rule-arn.json:
	tmpfile=$$(mktemp); \
	if ! aws elbv2 describe-rules --profile $(AWS_PROFILE) \
	       --listener-arn $$(cat state/listener-arn.json | jq -r .ListenerArn) \
	       --query "Rules[?Priority=='10']" 2>/dev/null >$$tmpfile; then \
	  rm -f $$tmpfile; \
	  false; \
	fi; \
	cat $$tmpfile | jq -r '.[]|.' >$@; \
	rm $$tmpfile
