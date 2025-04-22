#-*- mode: makefile; -*-

PERL_MODULES = \
    lib/Bedrock/Apache/API.pm.in \
    lib/Bedrock/Apache/API/Args.pm.in \
    lib/Bedrock/Apache/API/Routes.pm.in \
    lib/Bedrock/Apache/API/Handler.pm.in

GPERL_MODULES = $(PERL_MODULES:.pm.in=.pm)

@do_subst_command@

lib/Bedrock/Apache/API.pm: lib/Bedrock/Apache/API/Args.pm

lib/Bedrock/Apache/API/Routes.pm: lib/Bedrock/Apache/API.pm

%.pm: %.pm.in
	test -d "$$(dirname "$@")" || $(INSTALL) -d "$$(dirname "$@")"
	$(do_subst) $< > $@
	perl -I lib -wc $@ || (rm -f $@ && false)

VERSION = $(shell sh -c 'cat VERSION')

TARBALL = Bedrock-Apache-API-Routes-$(VERSION).tar.gz

TARBALL_DEPS = \
    buildspec.yml \
    $(GPERL_MODULES) \
    $(CONFIG_FILES) \
    requires \
    test-requires \
    README.md \
    VERSION

$(TARBALL): $(TARBALL_DEPS)
	make-cpan-dist.pl -b $<

README.md: lib/Bedrock/Apache/API.pm.in
	pod2markdown $< > $@

CLEANFILES += $(GPERL_MODULES)
