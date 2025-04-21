#-*- mode: makefile; -*-

########################################################################
# SecretsManager secrets
########################################################################
PHONY_TARGETS += secrets
secrets: $(SECRETS)

SECRETS = \
   state/aws-secret-api-key.json \
   state/aws-secret-mysql-password.json

state/aws-secret-api-key.json:
	@api_key="$(API_KEY)"
	if test -z "$$api_key"; then \
	  api_key=$$(uuid | tr a-z A-Z); \
	fi; \
	tmpfile=$$(mktemp); \
	aws secretsmanager describe-secret --profile $(AWS_PROFILE) \
	  --secret-id api-key 2>/dev/null >$$tmpfile || rm $$tmpfile; \
	if ! test -e $$tmpfile; then \
	  aws secretsmanager create-secret --profile $(AWS_PROFILE) \
	    --name api-key --secret-string $$api_key >$@; \
	else \
	  aws secretsmanager put-secret-value --profile $(AWS_PROFILE) \
	    --secret-id api-key --secret-string $$api_key >$@; \
	fi; \
	rm $$tmpfile;

state/aws-secret-mysql-password.json:
	@tmpfile=$$(mktemp); \
	if test -z "$(MYSQL_PASSWORD)"; then \
	  echo "set --aws-secret-mysql-password!"; \
	  exit 1; \
	fi;\
	aws secretsmanager describe-secret --profile $(AWS_PROFILE) \
	  --secret-id mysql-password >$$tmpfile || rm $$tmpfile; \
	if ! test -e $$tmpfile; then \
	  aws secretsmanager create-secret --profile $(AWS_PROFILE) \
	    --name mysql-password --secret-string "$(MYSQL_PASSWORD)" >$@; \
	else \
	  aws secretsmanager put-secret-value --profile $(AWS_PROFILE) \
	    --secret-id mysql-password --secret-string "$(MYSQL_PASSWORD)" >$@; \
	fi; \
	rm $$tmpfile
