#-*- mode: makefile; -*-

########################################################################
# ECS task execution role
########################################################################
PHONY_TARGETS += role
policies: state/ecs-task-execution-role.json

########################################################################
PHONY_TARGETS += secrets-policy
secrets-policy: state/secrets-policy.json
########################################################################

SECRETS_POLICY_DEPS = \
    templates/secrets-policy.json.in \
    $(SECRETS)

state/secrets-policy.json: $(SECRETS_POLICY_DEPS)
	@tmpfile=$$(mktemp); \
	aws_secret_mysql_password_arn=$$(cat state/aws-secret-mysql-password.json | jq -r .ARN); \
	aws_secret_api_key_arn=$$(cat state/aws-secret-api-key.json | jq -r .ARN); \
	cat $< | sed -e "s/[@]aws_secret_mysql_password_arn[@]/$$aws_secret_mysql_password_arn/g" \
	             -e  "s/[@]aws_secret_api_key_arn[@]/$$aws_secret_api_key_arn/g" > $$tmpfile; \
	aws iam create-policy --profile $(AWS_PROFILE) \
	  --policy-name api-secrets \
	  --policy-document file://$$tmpfile; \
	aws iam list-policies --profile $(AWS_PROFILE) \
	  --query "Policies[?PolicyName=='api-secrets']" | jq -r '.[]|.' > $@; \
	rm $$tmpfile;

########################################################################
PHONY_TARGETS += ecs-task-execution-role
ecs-task-execution-role: state/ecs-task-execution-role.json
########################################################################

ECS_TASK_EXECUTION_ROLE_DEPS = \
    state/secrets-policy.json \
    iam-policies/ecs-task-trust-policy.json

state/ecs-task-execution-role.json: $(ECS_TASK_EXECUTION_ROLE_DEPS)
	@if ! aws iam get-role --profile $(AWS_PROFILE) --role-name ecsTaskExecutionRole; then \
	  aws iam create-role --profile $(AWS_PROFILE) \
	    --role-name ecsTaskExecutionRole \
	    --assume-role-policy-document file://state/secrets-policy.json >/dev/null; \
	  aws iam attach-role-policy  --profile $(AWS_PROFILE) \
	    --role-name ecsTaskExecutionRole \
	    --policy-arn arn:aws:iam::aws:policy/service-role/AmazonECSTaskExecutionRolePolicy; \
	  aws iam attach-role-policy --profile $(AWS_PROFILE) \
	    --role-name ecsTaskExecutionRole \
	    --policy-arn $$(cat secrets-policy.json | jq -r .Policy.Arn); \
	  aws iam get-role --profile $(AWS_PROFILE) \
	    --role-name ecsTaskExecutionRole >$@;
	fi
