#-*- mode: makefile; -*-

########################################################################
# scriptlet: format a resource label for autoscaling
########################################################################
define generate_resource_label =

my $alb = $ENV{ALB_ARN}  // die "ALB_ARN not set\n";
my $tg  = $ENV{TG_ARN}   // die "TG_ARN not set\n";

my @parts = ($alb =~ m{loadbalancer/app/([^/]+)/([^/]+)});
push @parts, ($tg =~ m{targetgroup/([^/]+)/([^/]+)});

print {*STDOUT} sprintf "app/%s/%s/targetgroup/%s/%s\n", @parts;

endef

export s_generate_resource_label = $(value generate_resource_label)
########################################################################
# end scriptlet
########################################################################

resource-label: state/target-group.json state/load-balancer-arn.json
	TG_ARN=$$(cat state/target-group.json | jq -r '.TargetGroups[]|.TargetGroupArn'); \
	ALB_ARN=$$(cat state/load-balancer-arn.json | jq -r .LoadBalancerArn); \
	ALB_ARN=$$ALB_ARN TG_ARN=$$TG_ARN \
	perl -e "$$s_generate_resource_label" > $@

SCALING_POLICY_CONFIGURATION_DEPS = \
    templates/policy-configuration.json.in \
    resource-label

state/policy-configuration.json: $(SCALING_POLICY_CONFIGURATION_DEPS)
	resource_label=$$(cat resource-label); \
	sed -e "s|[@]resource_label[@]|$$resource_label|g" \
	    -e "s/[@]cluster_name[@]/$(AWS_CLUSTER_NAME)/g" \
	    -e "s/[@]service[@]/$(SERVICE)/g" $< >$@

PHONY_TARGETS += autoscaling
autoscaling: state/scaling-policy.json

SCALING_POLICY_DEPS = \
    state/policy-configuration.json \
    state/scalable-target

state/scaling-policy.json: $(SCALING_POLICY_DEPS)
	aws application-autoscaling put-scaling-policy --profile $(AWS_PROFILE) \
	  --service-namespace ecs \
	  --resource-id service/$(AWS_CLUSTER_NAME)/$(SERVICE) \
	  --scalable-dimension ecs:service:DesiredCount \
	  --policy-name web-scale-policy \
	  --policy-type TargetTrackingScaling \
	  --target-tracking-scaling-policy-configuration file://$< > $@

PHONY_TARGETS += scalable-target
scalable-target: state/scalable-target

MAX_CAPACITY = @max_capacity@

state/scalable-target:
	status=$$(aws application-autoscaling describe-scalable-targets --profile $(AWS_PROFILE)  \
	            --service-namespace ecs \
	            --resource-id service/$(AWS_CLUSTER_NAME)/$(SERVICE) | jq -r '.ScalableTargets[]|.') \
	if test -z "$$status"; then \
	  aws application-autoscaling register-scalable-target --profile $(AWS_PROFILE) \
	    --service-namespace ecs \
	    --scalable-dimension ecs:service:DesiredCount \
	    --resource-id service/$(AWS_CLUSTER_NAME)/$(SERVICE) \
	    --min-capacity 1 \
	    --max-capacity $(MAX_CAPACITY) >$@; \
	fi
