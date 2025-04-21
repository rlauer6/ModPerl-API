#-*- mode: makefile; -*-

########################################################################
# CloudWatch log group
########################################################################
PHONY_TARGETS += log-group
log-group: state/log-group.json

state/log-group.json:
	log_group="/ecs/$(SERVICE)"; \
	aws logs describe-log-groups --profile $(AWS_PROFILE) | \
	  jq --arg name "$$log_group" '.logGroups[] | select(.logGroupName == $$name)' >$@

	if ! test -e $@; then \
	  aws logs create-log-group --profile $(AWS_PROFILE) --log-group-name $$log_group >$@; \
	fi

