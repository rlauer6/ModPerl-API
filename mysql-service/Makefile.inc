#-*- mode: makefile; -*-
########################################################################
# Create MySQL service
########################################################################
PHONY_TARGETS += mysql-service
mysql-service: state/mysql-service.json
########################################################################

MYSQL_SERVICE_DEPS = \
  state/mysql-local-registry-arn.json \
  state/mysql-task-revision.txt \
  state/mysql-vpc-configuration.json

state/mysql-service.json: $(MYSQL_SERVICE_DEPS)
	SERVICE_NAME=mysql8; \
	REGISTRY_ARN=$$(cat $< | jq -r .Arn); \
	if ! aws ecs list-services --profile $(AWS_PROFILE) \
	  --cluster $(AWS_CLUSTER_NAME) 2>/dev/null | grep -q $(AWS_CLUSTER_NAME)/$$SERVICE_NAME; then \
	  revision=$$(cat state/mysql-task-revision.txt); \
	  aws ecs create-service --service-name $$SERVICE_NAME \
	      --task-definition $$SERVICE_NAME:$$revision \
	      --desired-count 1 \
	      --launch-type FARGATE \
	      --network-configuration file://state/mysql-vpc-configuration.json \
	      --profile $(AWS_PROFILE) \
	      --health-check-grace-period-seconds 60 \
	      --service-registries registryArn=$$REGISTRY_ARN,containerName=mysql \
	      --cluster $(AWS_CLUSTER_NAME) >$@; \
	else \
	  aws ecs describe-services --profile $(AWS_PROFILE) \
	    --cluster $(AWS_CLUSTER_NAME) --services $$SERVICE_NAME | jq -r '.services[]|.' >$@; \
	fi

state/mysql-task-definition.json: templates/mysql-task-definition.json.in $(SECRETS)
	log_group="/ecs/$(SERVICE)"; \
	aws_secret_mysql_password_arn=$$(cat state/aws-secret-mysql-password.json | jq -r .ARN); \
	sed -e "s/[@]aws_account[@]/$(AWS_ACCOUNT)/g" \
	    -e "s/[@]aws_secret_mysql_password_arn[@]/$$aws_secret_mysql_password_arn/g;" \
	    -e "s|[@]log_group[@]|$$log_group|g;" < $< > $@

state/mysql-task-revision.txt: state/mysql-task-definition.json
	task_definition=$$(mktemp); \
	aws ecs register-task-definition \
	  --cli-input-json file://$< --profile $(AWS_PROFILE) > $$task_definition; \
	revision=$$(cat $$task_definition | jq -r .taskDefinition.taskDefinitionArn); \
	echo $${revision##*:} > $@; \
	rm $$task_definition

state/mysql-vpc-configuration.json: templates/mysql-vpc-configuration.json.in
	SUBNETS=@aws_subnets@; \
	sed -e "s/[@]subnets[@]/$$SUBNETS/g" \
	    -e "s/[@]security_groups[@]/$(AWS_MYSQL_SECURITY_GROUP)/g" < $< > $@

########################################################################
PHONY_TARGETS += update-mysql-service
update-mysql-service: state/mysql-update-service.json
########################################################################

MYSQL_UPDATE_SERVICE_DEPS = \
    state/mysql-task-revision.txt \
    state/log-group.json \
    state/mysql-service.json

state/mysql-update-service.json: $(MYSQL_UPDATE_SERVICE_DEPS)
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
# start/stop MySQL service
########################################################################
PHONY_TARGETS += start-mysql-service
start-mysql-service: state/mysql-service.json
	SERVICE_NAME=mysql8; \
	count=$$(aws ecs describe-services \
	  --cluster $(AWS_CLUSTER_NAME) \
	  --services $$SERVICE_NAME \
	  --query "services[0].runningCount" \
	  --output text); \
	if [[ "$$count" = "0" ]]; then \
	  echo aws ecs update-service --profile $(AWS_PROFILE) \
	    --cluster $(AWS_CLUSTER_NAME) \
	    --service $$SERVICE_NAME \
	    --desired-count 1; \
	else \
	  echo "$$SERVICE_NAME is already running."; \
	fi

########################################################################
PHONY_TARGETS += stop-mysql-service
stop-mysql-service:
	count=$$(aws ecs describe-services \
	  --cluster $(AWS_CLUSTER_NAME) \
	  --services mysql8 \
	  --query "services[0].runningCount" \
	  --output text); \
	if [[ "$$count" = "0" ]]; then \
	  aws ecs update-service --profile $(AWS_PROFILE) \
	    --cluster $(AWS_CLUSTER_NAME) \
	    --service mysql8 \
	    --desired-count 0; \
	else \
	  echo "mysql8 is already stopped."; \
	fi
