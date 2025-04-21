# README 

This is the README for building a containerized HTTP API based on
`mod_perl` and Bedrock. The `Makefile` will:

1. ...create a CPAN distribution for a `mod_perl` based HTTP API (`Bedrock::Apache::API`)
2. ...build the Docker image based on the debian version of Bedrock
   containing the API codd
3. ...push the image to AWS ECR
4. ...update an ECS task definition for the service running on a Fargate cluster
5. ...add or update DNS records for API

## Recipes

| Description | Command |
| ----------- | ------- |
| Create the CPAN tarball only | `make` |
| Create the Docker image only and the CPAN tarball | `make image`|
| Push the latest image to ECR (creates the tarball and image) |`make update-repo` |
| Update the service (build tarball, image, push to repo, relaunch) | `make update-service` |
| Update Route53 | Adds a CNAME that points to the ALB (if one is enabled) |
| Force new deployment (will build run all recipes if required) |`make restart-service` |
| Create a MySQL service | `make mysql-service` |


