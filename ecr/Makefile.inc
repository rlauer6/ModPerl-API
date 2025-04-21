#-*- mode: makefile; -*-
########################################################################
# ECR repository
########################################################################
PHONY_TARGETS += ecr-repo
ecr-repo: state/ecr-repo.json
########################################################################

state/ecr-repo.json:
	aws ecr create-repository --profile $(AWS_PROFILE) \
	  --repository-name $(TAG) >$@

########################################################################
# ECR repository update
########################################################################
PHONY_TARGETS += update-repo
update-repo: state/update-repo.json
########################################################################

UPDATE_REPO_DEPS = \
    state/image.json \
    state/ecr-repo.json

state/update-repo.json: $(UPDATE_REPO_DEPS)
	docker tag $(TAG) $(ECS_REPO)/$(TAG):v$(VERSION)
	aws ecr get-login-password --profile $(AWS_PROFILE) --region $(AWS_REGION) | \
	  docker login --username AWS --password-stdin $(ECS_REPO)/$(TAG)
	AWS_PROFILE=$(AWS_PROFILE) docker push $(ECS_REPO)/$(TAG):v$(VERSION)
	cp state/image.json $@
