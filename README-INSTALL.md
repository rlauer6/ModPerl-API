# README

This is the README that will help you create a new API project.

To initialize a new project, download the init script and run it in
the directory where you want to start your project.

# Requirements

* `Bedrock`
* `docker`
* `autoconf` and friends

# Quick Start

## Step 1. Download and run the initialization script.

```
curl -O https://raw.githubusercontent.com/rlauer6/ModPerl-API/master/init-bedrock-api
chmod +x init-bedrock-api
./init-bedrock-api
```

This will download the API framework and install it in your project
directory. It will create files and sub-directories that are then
linked back to the ModPerl-API project directory, essentially allow
you to customize your API. The files you can override are listed in
the `INSTALL-FILES` file in the root of the ModPerl-API project.

## Step 2. Add .gitignore file

```
echo "ModPerl-API/" >.gitignore
```

## Step 3. Initialize your git repository

```
git init
```

## Step 4. Add your routes to the config file (See
[README.md](README.md) to learn about configuring routes)

## Step 5. Build your API by modifing `/lib/Bedrock/Apache/API/Routes.pm`


