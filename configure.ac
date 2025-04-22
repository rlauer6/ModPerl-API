AC_INIT([ModPerl-API],
        m4_esyscmd_s([
if test -z "$VERSION"; then \
  VERSION=$(test -e VERSION && cat VERSION); \
  VERSION=${VERSION:-0.0.0}; \
fi; \
echo $VERSION
]),
        m4_esyscmd_s([if test -z "$EMAIL"; then EMAIL=me@example.com; fi; echo $EMAIL]),[ModPerl-API])

__poor_mans_expand_file_name () {
    _tmp_name=$1
    _max_iter=10
    _iter=0

    eval "_tmp_val=\"\${${_tmp_name}}\""

    while test "$_iter" -lt "$_max_iter"; do
        # Stop if it's clearly not a var name or if it's <none>
        case "$_tmp_val" in
            "" | "<none>" | *[!a-zA-Z0-9_]* )
                break
                ;;
        esac

        # Safe eval only on valid var names
        if ! printf "%s" "$_tmp_val" | grep -qE '^[a-zA-Z_][a-zA-Z0-9_]*$'; then
            break
        fi

        eval "_next_val=\"\${$_tmp_val}\""

        if [ "$_next_val" = "$_tmp_val" ] || [ -z "$_next_val" ]; then
            break
        fi

        _tmp_val="$_next_val"
        _iter=$(expr $_iter + 1)
    done

    printf "%s\n" "$_tmp_val"
    unset _tmp_val _next_val _tmp_name _max_iter _iter
}

[
do_subst_command='do_subst = sed \
  -e '"'"'s,[@]PACKAGE[@],$(PACKAGE),g'"'"' \
  -e '"'"'s,[@]VERSION[@],$(VERSION),g'"'"' \
  -e '"'"'s,[@]PACKAGE_VERSION[@],$(PACKAGE_VERSION),g'"'"' \
  -e '"'"'s,[@]PATH_SEPARATOR[@],$(PATH_SEPARATOR),g'"'"' \
  -e '"'"'s~[@]configure_input[@]~Generated from $@.in; do not edit by hand.~g'"'"' \
  -e "s,[@]BUILD_DATE[@],$$(date '"'"'+%Y-%m-%d %H:%M:%S'"'"'),g" \
  \
  -e '"'"'s,[@]bindir[@],$(bindir),g'"'"' \
  -e '"'"'s,[@]datadir[@],$(datadir),g'"'"' \
  -e '"'"'s,[@]exec_prefix[@],$(exec_prefix),g'"'"' \
  -e '"'"'s,[@]includedir[@],$(includedir),g'"'"' \
  -e '"'"'s,[@]infodir[@],$(infodir),g'"'"' \
  -e '"'"'s,[@]libdir[@],$(libdir),g'"'"' \
  -e '"'"'s,[@]libexecdir[@],$(libexecdir),g'"'"' \
  -e '"'"'s,[@]localstatedir[@],$(localstatedir),g'"'"' \
  -e '"'"'s,[@]mandir[@],$(mandir),g'"'"' \
  -e '"'"'s,[@]oldincludedir[@],$(oldincludedir),g'"'"' \
  -e '"'"'s,[@]prefix[@],$(prefix),g'"'"' \
  -e '"'"'s,[@]sbindir[@],$(sbindir),g'"'"' \
  -e '"'"'s,[@]sharedstatedir[@],$(sharedstatedir),g'"'"' \
  -e '"'"'s,[@]sysconfdir[@],$(sysconfdir),g'"'"' \
  \
  -e '"'"'s,[@]abs_builddir[@],$(abs_builddir),g'"'"' \
  -e '"'"'s,[@]abs_srcdir[@],$(abs_srcdir),g'"'"' \
  -e '"'"'s,[@]abs_top_builddir[@],$(abs_top_builddir),g'"'"' \
  -e '"'"'s,[@]abs_top_srcdir[@],$(abs_top_srcdir),g'"'"' \
  -e '"'"'s,[@]srcdir[@],$(srcdir),g'"'"' \
  -e '"'"'s,[@]top_builddir[@],$(top_builddir),g'"'"' \
  -e '"'"'s,[@]top_srcdir[@],$(top_srcdir),g'"'"' \
\
  -e '"'"'s,[@]perl5libdir[@],$(perl5libdir),g'"'"' \
  -e '"'"'s,[@]perlrun[@],$(perlrun),g'"'"' \
  -e '"'"'s,[@]ksh[@],$(ksh),g'"'"' \
  -e '"'"'s,[@]bashrun[@],$(bashrun),g'"'"' \
  -e '"'"'s,[@]scandeps[@],$(scandeps),g'"'"' \
  -e '"'"'s,[@]make_cpan_dist[@],$(make_cpan_dist),g'"'"' \
  -e '"'"'s,[@]logdir[@],$(logdir),g'"'"' \
\
  -e '"'"'s,[@]SOME_THINGY[@],$(SOME_THINGY),g'"'"' \
'
]

AC_SUBST([do_subst_command])

AC_CONFIG_AUX_DIR([autotools])
AC_CONFIG_MACRO_DIR([autotools])

AM_INIT_AUTOMAKE([-Wall -Wno-portability gnu foreign tar-ustar])

AC_PATH_PROG([BASHRUN],
	[bash])

if test -z "$BASHRUN" ; then 
  AC_MSG_ERROR([bash not found?])
fi

AC_PATH_PROG([POD2MAN],
	[pod2man])

if test -z "$POD2MAN" ; then 
  AC_MSG_ERROR([pod2man not found?])
fi

AC_PATH_PROG([POD2MARKDOWN],
	[pod2markdown])

if test -z "$POD2MARKDOWN" ; then 
  AC_MSG_ERROR([pod2markdown  not found?])
fi

AC_PATH_PROG([SCANDEPS],
	[scandeps-static.pl])

if test -z "$SCANDEPS" ; then 
  AC_MSG_WARN([scandeps-static.pl not found...you won't be able generate project requirements])
fi

AC_PATH_PROG([MAKE_CPAN_DIST],
	[make-cpan-dist.pl])

if test -z "$MAKE_CPAN_DIST" ; then 
  AC_MSG_WARN([make-cpan-dist not found...you won't be able to create a CPAN distribution])
fi

AC_PATH_PROG([PERL],
	[perl])

if test -z "$PERL" ; then 
  AC_MSG_ERROR([perl not found?])
fi

perl5libdir=\${datadir}/perl5
AC_ARG_WITH(
	[perl5libdir],[  --with-perl5libdir (defaults to [DATAROOTDIR]/perl5)],
	[
    if test "${withval}" = 'no'; then
        true
    elif test "$withval" = 'yes'; then
        perl5libdir=$($PERL -MConfig -wle 'print $Config{sitelib};')
    else
        perl5libdir=$withval    
    fi
        ]
)

AC_SUBST([perl5libdir])

dnl ================================
dnl Optional AWS deployment
dnl ================================

AC_MSG_CHECKING([[whether to enable AWS deployment]])

AC_ARG_ENABLE([aws],
  [AS_HELP_STRING([--enable-aws], [Enable AWS support (default is disabled)])],
  [enable_aws="$enableval"],
  [enable_aws=no])

AM_CONDITIONAL([USE_AWS], [test "x$enable_aws" = "xyes"])

if test "x$enable_aws" = "xyes"; then
  AC_DEFINE([ENABLE_AWS], [1], [Define to 1 to enable aws support])
  AC_MSG_RESULT([yes])
else
  AC_DEFINE([ENABLE_AWS], [0], [Define to 0 if aws support is disabled])
  AC_MSG_RESULT([no])
fi


dnl ########################################################################
AC_ARG_WITH(
	[aws_profile],[  --with-aws-profile ],
	[ aws_profile="$withval" ],
        [ aws_profile="no-profile" ]
)

AC_SUBST([aws_profile])

dnl ########################################################################
AC_ARG_WITH(
	[aws_account], [  --with-aws-account ],
	[ aws_account="$withval" ],
        [ aws_account="" ]
)

if test "x$enable_aws" = "xyes"; then
  aws_account="$(aws sts get-caller-identity --profile "$aws_profile" 2>&1 | jq -r .Account)"
  aws_account=${aws_account:-000000000000}
fi                 

AC_SUBST([aws_account])

dnl ########################################################################
AC_ARG_WITH(
	[aws_region],[  --with-aws-region ],
	[ aws_region="$withval" ],
        [ aws_region="us-east-1" ]
)

AC_SUBST([aws_region])

dnl ########################################################################
AC_ARG_WITH(
	[aws_cluster_name],[  --with-aws-cluster-name ],
	[ aws_cluster_name="$withval" ],
        [ aws_cluster_name="no-cluster-name" ]
)

AC_SUBST([aws_cluster_name])

dnl ########################################################################
AC_ARG_WITH(
	[aws_security_group],[  --with-aws-security-group ],
	[ aws_security_group="$withval" ],
        [ aws_security_group="no-security-group" ]
)

AC_SUBST([aws_security_group])

dnl ########################################################################
AC_ARG_WITH(
	[aws_subnets],[  --with-aws-subnets ],
	[ aws_subnets="$withval" ],
        [ aws_subnets="" ]
)

AC_SUBST([aws_subnets])

dnl ########################################################################
AC_ARG_WITH(
	[aws_alb_name],[  --with-aws-alb-name ],
	[ aws_alb_name="$withval" ],
        [ aws_alb_name="none" ]
)

AC_SUBST([aws_alb_name])

dnl ########################################################################
AC_ARG_WITH(
	[aws_mysql_security_group],[  --with-aws-mysql-security-group ],
	[ aws_mysql_security_group="$withval" ],
        [ aws_mysql_security_group="" ]
)

AC_SUBST([aws_mysql_security_group])

dnl ########################################################################
AC_ARG_WITH(
	[aws_route53_profile],[  --with-aws-route53-profile ],
	[ aws_route53_profile="$withval" ],
        [ aws_route53_profile="$aws_profile" ]
)

AC_SUBST([aws_route53_profile])

dnl ########################################################################
AC_ARG_WITH(
	[aws_vpc_id],[  --with-aws-vpc-id ],
	[ aws_vpc_id="$withval" ],
        [ aws_vpc_id="" ]
)

if test "x$enable_aws" = "xyes"; then
  if test -n "$aws_subnets" && test -z "$aws_vpc_id"; then
    aws_vpc_id=$(aws ec2 describe-subnets --profile $aws_profile \
      --subnet-ids $aws_subnets \
      --query 'Subnets[[0]].VpcId' \
      --output text);
   fi       
fi

AC_SUBST([aws_vpc_id])

dnl ########################################################################
AC_ARG_WITH(
	[api_host],[  --with-api-host ],
	[ api_host="$withval" ],
        [ api_host="" ]
)

AC_SUBST([api_host])

dnl ########################################################################
AC_ARG_WITH(
	[aws_hosted_zone_id],[  --with-aws-hosted_zone-id ],
	[ aws_hosted_zone_id="$withval" ],
        [ aws_hosted_zone_id="" ]
)

AC_SUBST([aws_hosted_zone_id])

dnl ########################################################################
AC_ARG_WITH(
	[mysql_host],[  --with-mysql-host ],
	[ mysql_host="$withval" ],
        [ mysql_host="none" ]
)

AC_SUBST([mysql_host])

dnl ########################################################################
AC_ARG_WITH(
	[mysql_database],[  --with-mysql-database ],
	[ mysql_database="$withval" ],
        [ mysql_database="none" ]
)

AC_SUBST([mysql_database])

dnl ########################################################################
dnl AC_SUBST is done below after warnings about missing arguments
AC_ARG_WITH(
	[mysql_user],[  --with-mysql-user ],
	[ mysql_user="$withval" ],
        [ mysql_user="none" ]
)

dnl ########################################################################
AC_ARG_WITH(
	[mysql_password],[  --with-mysql-password ],
	[ mysql_password="$withval" ],
        [ mysql_password="none"]
)

AC_SUBST([mysql_password])

dnl ########################################################################
AC_ARG_WITH(
	[service_name],[  --with-service-name ],
	[ service_name="$withval" ],
        [ service_name=bedrock-api ]
)

AC_SUBST([service_name])

dnl ########################################################################
AC_ARG_WITH(
	[api_name],[  --with-api-name ],
	[ api_name="$withval" ],
        [ api_name="bedrock-api" ]
)

AC_SUBST([api_name])

dnl ########################################################################
AC_ARG_WITH(
	[api_key],[  --with-api-key ],
	[ api_key="$withval" ],
        [ api_key=$(aws secretsmanager get-secret-value --secret-id api-key --profile $aws_profile 2>/dev/null | jq -r .SecretString); \
          test -z "$api_key" && api_key="$(uuid | tr a-z A-Z)" 
        ]
)

AC_SUBST([api_key])

dnl ########################################################################
AC_ARG_WITH(
	[domain],[  --with-domain ],
	[ domain="$withval" ],
        [ domain="" ]
)

AC_SUBST([domain])

dnl ########################################################################
AC_ARG_WITH(
	[base_url],[  --with-base_url ],
	[ base_url="$withval" ],
        [ base_url="/bedrock-api" ]
)

AC_SUBST([base_url])

dnl ########################################################################
AC_ARG_WITH(
	[max_capacity],[  --with-max_capacity ],
	[ max_capacity="$withval" ],
        [ max_capacity="2" ]
)

AC_SUBST([max_capacity])

dnl ================================
dnl Optional database feature
dnl ================================

AC_MSG_CHECKING([[whether to enable database support in your API]])

AC_ARG_ENABLE([database],
  [AS_HELP_STRING([--disable-database], [Disable database support (default is enabled)])],
  [enable_database="$enableval"],
  [enable_database=yes])

AM_CONDITIONAL([USE_DATABASE], [test "x$enable_database" = "xyes"])

if test "x$enable_database" = "xyes"; then
  AC_DEFINE([ENABLE_DATABASE], [1], [Define to 1 if database support is enabled])
  AC_MSG_RESULT([yes])
else
  AC_DEFINE([ENABLE_DATABASE], [0], [Define to 0 to disable database support])
  AC_MSG_RESULT([no])
fi

if test "x$enable_database" = "xyes"; then
   if test "$mysql_database" = "<none>"; then
     AC_MSG_WARN([You enabled the use of a database but provided no --with-mysql-database?])
   fi

   if test "$mysql_password" = "<none>"; then
     AC_MSG_WARN([You enabled the use of a database but provided no --with-mysql-password?])
   fi

   if test "$mysql_host" = "<none>"; then
     AC_MSG_WARN([You enabled the use of a database but provided no --with-mysql-host?])
     AC_MSG_WARN([Set MySQL host to 'mysql.local' for AWS deployment or 'modperl-api_db_1' for local deployment])
   fi

   if test "$mysql_host" = "<none>"; then
     AC_MSG_WARN([You enabled the use of a database but provided no --with-mysql-user?])
     AC_MSG_WARN([Setting MySQL user to 'root'])
     mysql_user="root";
   fi

   AC_SUBST([mysql_user])
fi

dnl ================================
dnl Optional ALB configuration
dnl ================================

AC_MSG_CHECKING([[whether to use a load balancer with your service]])

AC_ARG_ENABLE([alb],
  [AS_HELP_STRING([--enable-alb], [Enable use of ALB (default is disabled)])],
  [enable_alb="$enableval"],
  [enable_alb=no])

AM_CONDITIONAL([USE_LOADBALANCER], [test "x$enable_alb" = "xyes"])

if test "x$enable_alb" = "xyes"; then
  AC_DEFINE([ENABLE_ALB], [1], [Define to 1 if ALB support is enabled])
  AC_MSG_RESULT([yes])
else
  AC_DEFINE([ENABLE_ALB], [0], [Define to 0 to disable ALB support])
  AC_MSG_RESULT([no])
fi
dnl ===================================
dnl Optional Autoscaling configuration
dnl ===================================

AC_MSG_CHECKING([[whether to use autoscaling with your service]])

AC_ARG_ENABLE([autoscaling],
  [AS_HELP_STRING([--enable-autoscaling], [Enable use of autoscaling (default is disabled)])],
  [enable_autoscaling="$enableval"],
  [enable_autoscaling=no])

AM_CONDITIONAL([USE_AUTOSCALING], [test "x$enable_alb" = "xyes"])

if test "x$enable_autoscaling" = "xyes"; then
  AC_DEFINE([ENABLE_AUTOSCALING], [1], [Define to 1 if AUTOSCALING support is enabled])
  AC_MSG_RESULT([yes])
else
  AC_DEFINE([ENABLE_AUTOSCALING], [0], [Define to 0 to disable AUTOSCALING support])
  AC_MSG_RESULT([no])
fi

dnl ================================
dnl Optional CNAME for ALB 
dnl ================================

AC_MSG_CHECKING([[whether to use add a DNS entry for your load balancer (default is disabled)]])

AC_ARG_ENABLE([route53],
  [AS_HELP_STRING([--enable-route53], [Enable DNS update (default is disabled)])],
  [enable_route53="$enableval"],
  [enable_route53=no])

AM_CONDITIONAL([USE_ROUTE53], [test "x$enable_route53" = "xyes"])

if test "x$enable_route53" = "xyes"; then
  AC_DEFINE([ENABLE_ROUTE53], [1], [Define to 1 if route53 support is enabled])
  AC_MSG_RESULT([yes])
else
  AC_DEFINE([ENABLE_ROUTE53], [0], [Define to 0 to disable route53 support])
  AC_MSG_RESULT([no])
fi

AX_DISTCHECK_HACK
AX_DEPS_CHECK
AX_PERLDEPS_CHECK
AX_REQUIREMENTS_CHECK

AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT

printf "                  .----------------------------.\n"
printf "                  | Default Installation Paths |\n"
printf "                  .----------------------------.\n"
printf "                        Prefix: %s\n" "`__poor_mans_expand_file_name prefix`"
printf "                 User binaries: %s\n" "`__poor_mans_expand_file_name bindir`"
printf "               System binaries: %s\n" "`__poor_mans_expand_file_name sbindir`"
printf "           Configuration files: %s\n" "`__poor_mans_expand_file_name sysconfdir`"
printf "                  Manual pages: %s\n" "`__poor_mans_expand_file_name mandir`"
printf "\n"
printf "                  .--------------------.\n"
printf "                  | Perl Configuration |\n"
printf "                  .--------------------.\n"
printf "                   perl5libdir: %s\n" "`__poor_mans_expand_file_name perl5libdir`"
printf "                          perl: %s\n" "`__poor_mans_expand_file_name PERL`"
printf "\n"
printf "                  .-------------------.\n"
printf "                  | API Configuration |\n"
printf "                  .-------------------.\n"
printf "                   base_url: %s\n" "`__poor_mans_expand_file_name base_url`"
printf "                   handler: %s\n" "`__poor_mans_expand_file_name handler`"
printf "                    domain: %s\n" "`__poor_mans_expand_file_name domain`"
printf "                   api_name: %s\n" "`__poor_mans_expand_file_name api_name`"
printf "                   api_key: %s\n" "`__poor_mans_expand_file_name api_key`"
printf "                   api_host: %s\n" "`__poor_mans_expand_file_name api_host`"
printf "\n"

if test "x$enable_database" = "xyes"; then
printf "                  .------------------------.\n"
printf "                  | Database Configuration |\n"
printf "                  .------------------------.\n"
printf "                mysql_host: %s\n" "`__poor_mans_expand_file_name mysql_host`"
printf "                mysql_user: %s\n" "`__poor_mans_expand_file_name mysql_user`"
printf "            mysql_password: %s\n" "`__poor_mans_expand_file_name mysql_password`"
printf "\n"
fi

if test "x$enable_aws" = "xyes"; then
printf "                  .---------------------------.\n"
printf "                  | AWS Service Configuration |\n"
printf "                  .---------------------------.\n"
printf "                  service_name: %s\n" "`__poor_mans_expand_file_name service_name`"
printf "                   aws_account: %s\n" "`__poor_mans_expand_file_name aws_account`"
printf "                   aws_profile: %s\n" "`__poor_mans_expand_file_name aws_profile`"
printf "                    aws_region: %s\n" "`__poor_mans_expand_file_name aws_region`"
printf "              aws_cluster_name: %s\n" "`__poor_mans_expand_file_name aws_cluster_name`"
printf "                   aws_subnets: %s\n" "`__poor_mans_expand_file_name aws_subnets`"
printf "            aws_security_group: %s\n" "`__poor_mans_expand_file_name aws_security_group`"
printf "                    aws_vpc_id: %s\n" "`__poor_mans_expand_file_name aws_vpc_id`"
printf "            aws_hosted_zone_id: %s\n" "`__poor_mans_expand_file_name aws_hosted_zone_id`"
printf "            enable_autoscaling: %s\n" "`__poor_mans_expand_file_name enable_autoscaling`"
printf "                  max_capacity: %s\n" "`__poor_mans_expand_file_name max_capacity`"
printf "\n"
fi

printf "                  .-----------------------.\n"
printf "                  | Default Program Paths |\n"
printf "                  .-----------------------.\n"
printf "                          bash: %s\n" "`__poor_mans_expand_file_name BASHRUN`"
printf "            scandeps-static.pl: %s\n" "`__poor_mans_expand_file_name SCANDEPS`"
printf "                  pod2markdown: %s\n" "`__poor_mans_expand_file_name POD2MARKDOWN`"
printf "                       pod2man: %s\n" "`__poor_mans_expand_file_name POD2MAN`"
printf "\n"
printf "               Package Version: %s\n"   "${PACKAGE_VERSION}"

if test -n "${PERL_MODULES_MISSING}"; then
 echo -e "\nNOTE: you have missing or possibly out of date Perl modules:\n"
 echo ${PERL_MODULES_MISSING} | perl -ne 'print sprintf "* %s\n", join "\n* ", grep { $_ } split /\s*,\s*/;'
fi
