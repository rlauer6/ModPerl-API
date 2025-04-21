#-*- mode: makefile; -*-

########################################################################
# stop-service
########################################################################
PHONY_TARGETS += stop-service
stop-service:
	count=$$(aws ecs describe-services \
	  --cluster $(AWS_CLUSTER_NAME) \
	  --services $(SERVICE) \
	  --query "services[0].runningCount" \
	  --output text); \
	if [[ "$$count" = "0" ]]; then \
	  aws ecs update-service --profile $(AWS_PROFILE) \
	    --cluster $(AWS_CLUSTER_NAME) \
	    --service $(SERVICE) \
	    --desired-count 0; \
	else \
	  echo "$(SERVICE) is already stopped."; \
	fi

########################################################################
# start-service
########################################################################
PHONY_TARGETS += start-service
start-service: state/service.json
	count=$$(aws ecs describe-services \
	  --cluster $(AWS_CLUSTER_NAME) \
	  --services $(SERVICE) \
	  --query "services[0].runningCount" \
	  --output text); \
	if [[ "$$count" = "0" ]]; then \
	  echo aws ecs update-service --profile $(AWS_PROFILE) \
	    --cluster $(AWS_CLUSTER_NAME) \
	    --service $(SERVICE) \
	    --desired-count 1; \
	else \
	  echo "$(SERVICE) is already running."; \
	fi

########################################################################
# delete-service
########################################################################
PHONY_TARGETS += delete-service
delete-service: stop-service
	aws ecs delete-service --profile $(AWS_PROFILE) \
	  --cluster $(AWS_CLUSTER_NAME) \
	  --service $(SERVICE) \
	  --force

########################################################################
# restart-service
########################################################################
PHONY_TARGETS += restart-service
restart-service:  state/update-service.json
	aws ecs update-service \
	  --cluster $(AWS_CLUSTER_NAME) \
	  --service $(SERVICE) \
	  --force-new-deployment \
	  --profile $(AWS_PROFILE)

########################################################################
# Create ECS service
########################################################################
PHONY_TARGETS += service
service: state/service.json
########################################################################

SERVICE_DEPS = \
    state/task-definition-revision.txt \
    state/vpc-configuration.json \
    state/web-local-registry-arn.json \
    state/target-group.json

if USE_DATABASE
    SERVICE_DEPS += state/mysql-task-revision.txt
endif

state/service.json: $(SERVICE_DEPS)
	REGISTRY_ARN=$$(cat state/web-local-registry-arn.json | jq -r .Arn); \
	TARGET_GROUP_ARN=$$(cat state/target-group.json | jq -r '.TargetGroups[]|.TargetGroupArn'); \
	if ! aws ecs list-services --profile $(AWS_PROFILE) \
	  --cluster $(AWS_CLUSTER_NAME) 2>/dev/null | grep -q $(AWS_CLUSTER_NAME)/$(SERVICE); then \
	  health_check_grace_period=$(HEALTH_CHECK_GRACE_PERIOD); \
	  health_check_grace_period=$${health_check_grace_period:-60}; \
	  revision=$$(cat $<); \
	  aws ecs create-service --service-name $(SERVICE) \
	    --task-definition $(SERVICE):$$revision \
	    --desired-count 1 \
	    --launch-type FARGATE \
	    --network-configuration file://state/vpc-configuration.json \
	    --profile $(AWS_PROFILE) \
	    --health-check-grace-period-seconds $$health_check_grace_period \
	    --service-registries registryArn=$$REGISTRY_ARN,containerName=$(SERVICE) \
	    --load-balancers "targetGroupArn=$$TARGET_GROUP_ARN,containerName=$(SERVICE),containerPort=80" \
	    --cluster $(AWS_CLUSTER_NAME) >$@ || rm $@; \
	else \
	  aws ecs describe-services --profile $(AWS_PROFILE) \
	    --cluster $(AWS_CLUSTER_NAME) --services $(SERVICE) | jq -r '.services[]|.' >$@; \
	fi

########################################################################
# Update ECS service
########################################################################
PHONY_TARGETS += service-status
service-status:
	@status=$$(aws ecs describe-services --profile $(AWS_PROFILE) \
	  --cluster $(AWS_CLUSTER_NAME) --services $(SERVICE) | jq -r '.services[]|.status'); \
	if [[ "$$status" = "INACTIVE" || -z "$$status" ]]; then \
	  rm -f state/service.json state/update-service.json; \
	fi 

########################################################################
PHONY_TARGETS += update-service
update-service: service-status state/update-service.json
########################################################################

UPDATE_SERVICE_DEPS = \
    state/task-definition-revision.txt \
    state/log-group.json \
    state/service.json

state/update-service.json: $(UPDATE_SERVICE_DEPS)
	revision=$$(cat $<); \
	if ! aws ecs update-service \
	  --cluster $(AWS_CLUSTER_NAME) \
	  --service $(SERVICE) \
	  --task-definition $(SERVICE):$$revision \
	  --profile=$(AWS_PROFILE) > $@; then \
	  rm -f $@; \
	fi; \
	echo $$revision

########################################################################
# ECS task-definition
########################################################################
PHONY_TARGETS += task-definition
task-definition: state/task-definition.json
########################################################################

TASK_DEFINITION_DEPS = \
    templates/task-definition.json.in \
    state/update-repo.json \
    $(SECRETS) \
    state/ecs-task-execution-role.json

state/task-definition.json: $(TASK_DEFINITION_DEPS)
	digest=$$(docker inspect $(ECS_REPO)/$(TAG):v$(VERSION) | jq -r '.[]|.RepoDigests[]'); \
	aws_secret_mysql_password_arn=$$(cat state/aws-secret-mysql-password.json | jq -r .ARN); \
	aws_secret_api_key_arn=$$(cat state/aws-secret-api-key.json | jq -r .ARN); \
	cat $< | sed -e "s|\$${digest}|$$digest|g;" \
	  -e 's/[@]api_name[@]/$(API_NAME)/g;' \
	  -e 's/[@]aws_account[@]/$(AWS_ACCOUNT)/g;' \
	  -e 's/[@]service_name[@]/$(SERVICE)/g;' \
	  -e 's/[@]mysql_user[@]/$(MYSQL_USER)/g;' \
	  -e 's/[@]mysql_host[@]/$(MYSQL_HOST)/g;' \
	  -e 's/[@]mysql_password[@]/$(MYSQL_PASSWORD)/g;' \
	  -e 's/[@]api_key[@]/$(API_KEY)/g;' \
	  -e "s/[@]aws_secret_mysql_password_arn[@]/$$aws_secret_mysql_password_arn/g;" \
	  -e "s/[@]aws_secret_api_key_arn[@]/$$aws_secret_api_key_arn/g;"  >$@ || rm $@

state/task-definition-revision.txt: state/task-definition.json
	task_definition=$$(mktemp); \
	aws ecs register-task-definition \
	  --cli-input-json file://$< --profile $(AWS_PROFILE) > $$task_definition; \
	revision=$$(cat $$task_definition | jq -r .taskDefinition.taskDefinitionArn); \
	echo $${revision##*:} > $@; \
	rm $$task_definition; \
	touch $@


########################################################################
# VPC configuration
########################################################################
PHONY_TARGETS += vpc-configuration
vpc-configuration: state/vpc-configuration.json
########################################################################

SUBNETS         = @aws_subnets@
SECURITY_GROUPS = @aws_security_group@

state/vpc-configuration.json: templates/vpc-configuration.json.in
	subnets="$(SUBNETS)"; \
	security_groups="$(SECURITY_GROUPS)"; \
	cat $< | sed -e "s/[@]subnets[@]/$$subnets/g" \
	-e "s/[@]security_groups[@]/$$security_groups/g" | tee $@
